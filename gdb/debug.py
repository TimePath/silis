#!/usr/bin/env python3
import json
import re
import typing
from abc import abstractmethod
from functools import partial

# noinspection PyUnresolvedReferences
import gdb


class Printer:
    # types

    @abstractmethod
    def recognize(self, type) -> typing.Optional[str]:
        pass

    @abstractmethod
    def to_type_string(self, type, args) -> str:
        pass

    # values

    @abstractmethod
    def to_string(self, val) -> str:
        pass

    def display_hint(self, val) -> str:
        return f"={self.to_string(val)}\0"

    @abstractmethod
    def children(self, val) -> typing.Sequence[typing.Tuple[str, str]]:
        pass


def printer(arg: typing.Union[str, Printer]):
    def recognize(pattern: typing.Pattern, type) -> typing.Optional[typing.Match]:
        unqualified = type.unqualified()
        for name in [unqualified.name, unqualified.strip_typedefs().name]:
            if not name:
                continue
            m = pattern.match(name)
            if m:
                return m
        return None

    class TypePrinterWrapper:
        def __init__(self, it: Printer, pattern: typing.Pattern):
            self.it = it
            self.pattern = pattern
            if not getattr(it.recognize, "__isabstractmethod__", None):
                setattr(self, "recognize", lambda type: it.recognize(type))

        def recognize(self, type):
            m = recognize(self.pattern, type)
            if m:
                return self.it.to_type_string(type, m.groups())

    class TypePrinterFactory:
        def __init__(self, name: str, it: Printer, pattern: typing.Pattern):
            self.name = name
            self.enabled = True
            self.it = it
            self.pattern = pattern

        def instantiate(self):
            return TypePrinterWrapper(self.it, self.pattern)

    class PrettyPrinterWrapper:
        def __init__(self, it: Printer, val):
            if not getattr(it.to_string, "__isabstractmethod__", None):
                setattr(self, "to_string", lambda: it.to_string(val))
            if not getattr(it.display_hint, "__isabstractmethod__", None):
                setattr(self, "display_hint", lambda: it.display_hint(val))
            if not getattr(it.children, "__isabstractmethod__", None):
                setattr(self, "children", lambda: it.children(val))

    class PrettyPrinterFactory:
        def __init__(self, name: str, it: Printer, pattern: typing.Pattern):
            self.name = name
            self.enabled = True
            self.it = it
            self.pattern = pattern

        def __call__(self, val):
            ret = self._lookup(val)
            return ret

        def _lookup(self, val):
            if val.type.code == gdb.TYPE_CODE_PTR:
                if not val:
                    return PrettyPrinterWrapper(NULL(), val)
            if val.type == gdb.lookup_type("void *"):
                return None
            if recognize(self.pattern, val.type):
                return PrettyPrinterWrapper(self.it, val)

    def thunk(regex: str, ctor: typing.Callable[[], Printer]):
        objfile = gdb.current_objfile()
        name = f"{objfile}:{regex}"
        it = ctor()
        pattern = re.compile(f"^{regex}$")
        gdb.types.register_type_printer(objfile, TypePrinterFactory(name, it, pattern))
        gdb.printing.register_pretty_printer(objfile, PrettyPrinterFactory(name, it, pattern), True)
        return ctor

    if isinstance(arg, str):
        return partial(thunk, arg)
    return thunk(arg.__name__, arg)


class NULL(Printer):
    def to_string(self, val):
        return "NULL"


@printer("Vector__(.*)")
class Vector(Printer):
    def to_type_string(self, type, args):
        return f"Vector<{args[0]}>"

    def to_string(self, val):
        n = val["_size"]
        return f"Vector of {n}"

    def children(self, val):
        n = val["_size"]
        for i in range(n):
            yield (f"[{i}]", val["_data"][i])


@printer("Slice__(.*)")
class Slice(Printer):
    def to_type_string(self, type, args):
        return f"Slice<{args[0]}>"

    def to_string(self, val):
        begin = val["_begin"]["r"]
        end = val["_end"]
        n = end - begin
        return f"Slice of {n}"

    def children(self, val):
        begin = val["_begin"]["r"]
        end = val["_end"]
        n = end - begin
        for i in range(n):
            yield (f"[{i}]", begin[i])


@printer
class String(Printer):
    def to_type_string(self, type, args):
        return "String"

    def to_string(self, val):
        begin = val["bytes"]["_begin"]["r"]
        end = val["bytes"]["_end"]
        n = end - begin
        return json.dumps("".join(chr(begin[i]) for i in range(n)))

    def children(self, val):
        yield ("encoding", val["encoding"])
        yield ("bytes", val["bytes"])

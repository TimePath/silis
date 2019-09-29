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
    def to_type_string(self, type, args: typing.List[str]) -> str:
        pass

    # values

    @abstractmethod
    def to_string(self, val, args: typing.List[str]) -> str:
        pass

    def display_hint(self, val, args: typing.List[str]) -> str:
        return f"={self.to_string(val, args)}\0"

    @abstractmethod
    def children(self, val, args: typing.List[str]) -> typing.Sequence[typing.Tuple[str, str]]:
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
                return self.it.to_type_string(type, list(m.groups()))

    class TypePrinterFactory:
        def __init__(self, name: str, it: Printer, pattern: typing.Pattern):
            self.name = name
            self.enabled = True
            self.it = it
            self.pattern = pattern

        def instantiate(self):
            return TypePrinterWrapper(self.it, self.pattern)

    class PrettyPrinterWrapper:
        def __init__(self, it: Printer, val, args):
            if not getattr(it.to_string, "__isabstractmethod__", None):
                setattr(self, "to_string", lambda: it.to_string(val, args))
            if not getattr(it.display_hint, "__isabstractmethod__", None):
                setattr(self, "display_hint", lambda: it.display_hint(val, args))
            if not getattr(it.children, "__isabstractmethod__", None):
                setattr(self, "children", lambda: it.children(val, args))

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
                    return PrettyPrinterWrapper(NULL(), val, [])
            if val.type == gdb.lookup_type("void").pointer():
                return None
            for tf in [lambda: val.type, lambda: val.type.target()]:
                try:
                    t = tf()
                except RuntimeError:
                    continue
                m = recognize(self.pattern, t)
                if m:
                    return PrettyPrinterWrapper(self.it, val, list(m.groups()))

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
    def to_string(self, val, args):
        return "NULL"


@printer("Ref__(.*)")
class Ref(Printer):
    def to_type_string(self, type, args):
        return f"Ref<{args[0]}>"

    def to_string(self, val, args):
        n = val["priv"]["id"]
        return f"Ref<{args[0]}>: {n}"

    def children(self, val, args):
        priv = val["priv"]
        t = gdb.lookup_type(args[0])
        yield ("id", (priv["id"]))
        if priv["container"]:
            try:
                ref = priv["deref"][0](priv["container"], priv["id"])
                yield ("to", ref.cast(t.pointer())[0])
            except gdb.error as e:
                yield ("to", str(e))
            except gdb.MemoryError:
                yield ("to", "INVALID")
            except:
                pass


class ADTLike(Printer):
    def variant(self, val):
        kind = val["kind"]["_val"]
        u = val["u"]
        try:
            return u.type.keys()[kind - 1]
        except IndexError:
            return "?"

    def children(self, val, args):
        kind = val["kind"]["_val"]
        u = val["u"]
        try:
            k = u.type.keys()[kind - 1]
        except IndexError:
            return
        yield (f"kind:{k}", True)
        variant = u[k]
        if variant.type.name:
            yield ("value", variant)
            return
        for k in variant.type.keys():
            yield (k, variant[k])


@printer("ADT__(.*)")
class ADT(ADTLike):
    def to_type_string(self, type, args):
        return f"{args[0]}<*>"

    def to_string(self, val, args):
        return f"{args[0]}<{self.variant(val)}>"


@printer("Result__(.*)__(.*)")
class Result(ADTLike):
    def to_type_string(self, type, args):
        return f"Result<{args[0]} | {args[1]}>"

    def to_string(self, val, args):
        return f"Result<{self.variant(val)}>"


@printer("Vector__(.*)")
class Vector(Printer):
    def to_type_string(self, type, args):
        return f"Vector<{args[0]}>"

    def to_string(self, val, args):
        n = val["_size"]
        return f"Vector of {n}"

    def children(self, val, args):
        n = val["_size"]
        for i in range(n):
            yield (f"[{i}]", val["_data"][i])


@printer("Slice__(.*)")
class Slice(Printer):
    def to_type_string(self, type, args):
        return f"Slice<{args[0]}>"

    def to_string(self, val, args):
        begin = val["_begin"]["r"]
        end = val["_end"]
        n = end - begin
        return f"Slice of {n}"

    def children(self, val, args):
        begin = val["_begin"]["r"]
        end = val["_end"]
        n = end - begin
        for i in range(n):
            yield (f"[{i}]", begin[i])


@printer
class String(Printer):
    def to_type_string(self, type, args):
        return "String"

    def to_string(self, val, args):
        begin = val["bytes"]["_begin"]["r"]
        end = val["bytes"]["_end"]
        n = end - begin
        return json.dumps(bytes(begin[i] for i in range(n)).decode("utf8"))

    def children(self, val, args):
        yield ("encoding", val["encoding"])
        yield ("bytes", val["bytes"])


@printer("Trie__(.*)")
class Trie(Printer):
    def to_type_string(self, type, args):
        return f"Trie<{args[0]}>"

    def to_string(self, val, args):
        n = val["entries"]["_size"]
        return f"Trie of {n}"

    def children(self, val, args):
        n = val["entries"]["_size"]
        for i in range(n):
            it = val["entries"]["_data"][i]
            v = it["value"]
            begin = it["key"]["_begin"]["r"]
            end = it["key"]["_end"]
            k = json.dumps(bytes(begin[i] for i in range(end - begin)).decode("utf8"))
            yield (k, v)


@printer("Value")
class Value(ADTLike):
    def to_type_string(self, type, args):
        return f"Value<*>"

    def to_string(self, val, args):
        return f"Value<{self.variant(val)}>"

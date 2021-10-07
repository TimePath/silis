import os
import sys

sys.path.append(os.path.dirname(os.path.realpath(__file__)))
import vis
import inspect

gdb = __import__("gdb")
printers = {}


def register_printers(objfile):
    global printers
    printers = vis.register()
    gdb.printing.register_pretty_printer(objfile, GDBValuePrinter.factory)


def lookup(valobj):
    from vis_lib import parse_template_args

    val_type = valobj.type
    lookup_tag = val_type.tag
    if not lookup_tag:
        return None
    template_args = list(parse_template_args(lookup_tag))
    if len(template_args):
        lookup_tag = lookup_tag[:lookup_tag.find("<")]
    printer = printers.get(lookup_tag)

    def genargs():
        i = 0
        n = len(template_args)
        while i < n:
            try:
                arg = val_type.template_argument(i)
            except RuntimeError as e:
                arg = gdb.lookup_type(template_args[i])
            isintegral = not isinstance(arg, gdb.Type)
            i += 1
            yield {
                "key": arg.tag if not isintegral else False,
                "val": Type(arg) if not isintegral else arg,
            }

    args = list(genargs())
    i = 0
    while isinstance(printer, dict):
        arg = args[i]
        i += 1
        dyn = printer.get(None)
        varargs = map(lambda it: it["val"], args[i:]) if dyn and inspect.getfullargspec(dyn).varargs else []
        printer = printer.get(arg["key"]) or dyn(arg["val"], *varargs)
    return printer


class Type:
    def __init__(self, gdbtype):
        self.gdbtype = gdbtype

    def __str__(self):
        return self.gdbtype.name


class Value:
    def __init__(self, gdbvalue):
        self.gdbvalue = gdbvalue

    def __getitem__(self, item):
        return Value(self.gdbvalue.__getitem__(item))

    def __index__(self):
        return self.gdbvalue.__index__()

    def __str__(self):
        return self.gdbvalue.__str__()

    def cast(self, t):
        return Value(self.gdbvalue.cast(t.gdbtype))


class GDBValuePrinter:
    """https://sourceware.org/gdb/onlinedocs/gdb/Pretty-Printing-API.html#Pretty-Printing-API"""

    @classmethod
    def factory(cls, val):
        printer = lookup(val)
        if not printer:
            return None
        return GDBValuePrinter(printer, val)

    def __init__(self, printer, val):
        self.printer = printer
        self.val = val

        def bind(k, f, transform=lambda it: it):
            if getattr(f, "__isabstractmethod__", False):
                return

            def method():
                printer.update(Value(self.val))
                return transform(f())

            setattr(self, k, method)

        def transform_summary(ret):
            if isinstance(ret, Value):
                ret = ret.gdbvalue
            return ret

        def transform_child(child):
            key = child[0]
            val = child[1]
            if isinstance(val, Value):
                val = val.gdbvalue
            return key, val

        def transform_children(children):
            for it in children:
                yield transform_child(it)

        bind("to_string", printer.repr, transform_summary)
        bind("children", printer.children, transform_children)


register_printers(gdb.current_objfile())

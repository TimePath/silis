import os
import sys

sys.path.append(os.path.dirname(os.path.realpath(__file__)))
import vis

gdb = __import__("gdb")
printers = {}


def register_printers(objfile):
    global printers
    printers = vis.register()
    gdb.printing.register_pretty_printer(objfile, GDBValuePrinter.factory)


def lookup(val):
    val_type = val.type
    lookup_tag = val_type.tag
    try:
        val_type.template_argument(0)
        lookup_tag = lookup_tag[:lookup_tag.find("<")]
    except RuntimeError as e:
        pass
    printer = printers.get(lookup_tag)
    i = 0
    while isinstance(printer, dict):
        arg = val_type.template_argument(i)
        integral = not isinstance(arg, gdb.Type)
        i += 1
        k = arg.tag if not integral else False
        printer = printer.get(k) or printer.get(None)(arg)
    return printer


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

        def bind(k, f):
            if getattr(f, "__isabstractmethod__", False):
                return

            def method():
                printer.update(self.val)
                return f()

            setattr(self, k, method)

        bind("to_string", printer.repr)
        bind("children", printer.children)


register_printers(gdb.current_objfile())

from typing import Any, List

import os
import sys

sys.path.append(os.path.dirname(os.path.realpath(__file__)))
import vis

lldb = __import__("lldb")
printers = {}


def __lldb_init_module(debugger, internal_dict):
    global printers
    printers = vis.register()

    category = debugger.GetDefaultCategory()
    for k in printers:
        is_regex = True
        type_name_specifier = lldb.SBTypeNameSpecifier(f"^{k}(<.+>)?$", is_regex)

        cls = LLDBValuePrinter
        type_synthetic = lldb.SBTypeSynthetic.CreateWithClassName(f"{__name__}.{cls.__name__}")
        type_synthetic.SetOptions(lldb.eTypeOptionCascade)
        category.AddTypeSynthetic(type_name_specifier, type_synthetic)

        type_summary = lldb.SBTypeSummary.CreateWithFunctionName(f"{__name__}.{cls.__name__}.{cls.summarise.__name__}")
        type_summary.SetOptions(lldb.eTypeOptionCascade)
        category.AddTypeSummary(type_name_specifier, type_summary)


def lookup(valobj):
    val_type = canonical(valobj.GetType())
    lookup_tag = val_type.GetName()
    if val_type.GetNumberOfTemplateArguments():
        lookup_tag = lookup_tag[:lookup_tag.find("<")]
    printer = printers.get(lookup_tag)

    def parse_template_args(typename):
        csv = typename[typename.find("<") + 1: -1]
        depth = 0
        buf = ""
        for c in csv + ",":
            if c == ",":
                if depth:
                    buf += c
                else:
                    yield buf
                    buf = ""
            elif c == " ":
                pass
            elif c == "<":
                buf += c
                depth += 1
            elif c == ">":
                buf += c
                depth -= 1
            else:
                buf += c

    template_args = list(parse_template_args(val_type.GetName()))

    def evaluate(exp):
        thread = valobj.process.GetSelectedThread()
        frame = thread.GetFrameAtIndex(0)
        val = frame.EvaluateExpression(exp)
        return val if not val.GetError().Fail() else None

    i = 0
    while isinstance(printer, dict):
        argtype = canonical(val_type.GetTemplateArgumentType(i))
        isintegral = val_type.GetTemplateArgumentKind(i) == lldb.eTemplateArgumentKindIntegral
        if isintegral:
            val = evaluate(f"({argtype.GetName()}) {template_args[i]}")
            argtype = val.GetValueAsUnsigned()
        i += 1
        k = argtype.GetName() if not isintegral else False
        printer = printer.get(k) or printer.get(None)(Type(argtype) if not isintegral else argtype)
    return printer


def canonical(t):
    if t.IsReferenceType():
        t = t.GetDereferencedType()
    while t.IsPointerType():
        t = t.GetPointeeType()
    t = t.GetUnqualifiedType()
    t = t.GetCanonicalType()
    return t


class Type:
    def __init__(self, sbtype):
        self.sbtype = sbtype

    def __str__(self):
        return self.sbtype.GetName()


class Value:
    def __init__(self, sbvalue):
        self.sbvalue = sbvalue

    def __getitem__(self, item):
        if isinstance(item, str):
            return Value(self.sbvalue.GetChildMemberWithName(item))
        return Value(self.sbvalue.GetChildAtIndex(item))


class LLDBValuePrinter:
    """
    https://lldb.llvm.org/cpp_reference/namespacelldb.html
    https://opensource.apple.com/source/lldb/lldb-310.2.36/www/python_reference/frames.html
    """

    def __init__(self, valobj, internal_dict):
        printer = lookup(valobj)
        self.printer = printer
        self.valobj = valobj

        def bind(k, f, transform):
            if getattr(f, "__isabstractmethod__", False):
                return

            def method():
                printer.update(Value(self.valobj))
                return transform(f())

            setattr(self, k, method)

        def transform_summary(ret):
            if isinstance(ret, Value):
                ret = ret.sbvalue
            if isinstance(ret, lldb.SBValue):
                ret = ret.GetValue()
            ret = str(ret)
            return ret

        bind("get_summary", printer.repr, transform_summary)
        bind("get_value", printer.repr, lambda it: it.sbvalue if isinstance(it, Value) else None)

    @classmethod
    def summarise(cls, valobj, internal_dict):
        valobj_synthetic = valobj
        valobj = valobj_synthetic.GetNonSyntheticValue()
        inst = LLDBValuePrinter(valobj, internal_dict)
        inst.update()
        return inst.get_summary()

    children: List[Any]

    def update(self):
        self.printer.update(Value(self.valobj))
        if self.has_children():
            self.children = list(self.printer.children())
        return False

    def get_summary(self):
        return ""

    def has_children(self):
        return not getattr(self.printer.children, "__isabstractmethod__", False)

    def num_children(self):
        if not self.has_children():
            return 0
        return len(self.children)

    def get_child_at_index(self, index):
        if not self.has_children():
            return None
        ret = self.children[index][1]
        return ret.sbvalue

    def get_child_index(self, name):
        if not self.has_children():
            return -1
        return -1

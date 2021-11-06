from typing import Any, List

import os
import sys
import inspect

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
    from vis_lib import parse_template_args

    val_type = canonical(valobj.GetType())
    lookup_tag = val_type.GetName()
    template_args = list(parse_template_args(lookup_tag))
    if len(template_args):
        lookup_tag = lookup_tag[:lookup_tag.find("<")]
    printer = printers.get(lookup_tag)

    def evaluate(exp):
        thread = valobj.process.GetSelectedThread()
        frame = thread.GetFrameAtIndex(0)
        val = frame.EvaluateExpression(exp)
        return val if not val.GetError().Fail() else None

    def genargs():
        i = 0
        n = len(template_args)
        while i < n:
            kind = val_type.GetTemplateArgumentKind(i)
            kind = lldb.eTemplateArgumentKindNull
            if kind != lldb.eTemplateArgumentKindNull:
                arg = val_type.GetTemplateArgumentType(i)
            else:
                arg = valobj.target.FindFirstType(template_args[i])
            arg = canonical(arg)
            isintegral = kind == lldb.eTemplateArgumentKindIntegral
            if isintegral:
                val = evaluate(f"({arg.GetName()}) {template_args[i]}")
                arg = val.GetValueAsUnsigned()
            i += 1
            yield {
                "key": arg.GetName() if not isintegral else False,
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
    if not printer:
        raise Exception(f"unable to lookup {lookup_tag}")
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
        val = self.sbvalue
        if isinstance(item, str):
            return Value(val.GetChildMemberWithName(item))
        t = val.Dereference().GetType()
        return Value(val.CreateChildAtOffset("", t.size * item, t))

    def __index__(self):
        return self.sbvalue.GetValueAsUnsigned()

    def __str__(self):
        return self.sbvalue.GetValue()

    def cast(self, t):
        if t.sbtype.GetName() == "":
            return self
        return Value(self.sbvalue.Cast(t.sbtype))


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
        child = self.children[index]
        key = child[0]
        val = child[1].sbvalue
        return val.CreateChildAtOffset(key, 0, val.GetType())

    def get_child_index(self, name):
        if not self.has_children():
            return -1
        return -1

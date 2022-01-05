# SPDX-License-Identifier: AFL-3.0

from abc import abstractmethod


class Printer:
    @abstractmethod
    def update(self, val):
        pass

    @abstractmethod
    def repr(self):
        pass

    @abstractmethod
    def children(self):
        pass


class WordPrinter(Printer):
    _data: object

    def update(self, val):
        self._data = val["data_"]

    def repr(self):
        return self._data


class PointerPrinter(Printer):
    _data: object

    def update(self, val):
        self._data = val["data_"]

    def repr(self):
        return self._data


class TuplePrinter(Printer):
    Ts: object

    def __init__(self, Ts):
        super().__init__()
        self.Ts = Ts

    _data: object

    def update(self, val):
        self._data = val["data_"]

    def repr(self):
        return f"Tuple<{', '.join(str(t) for t in self.Ts)}>"

    def children(self):
        for i, t in enumerate(self.Ts):
            yield f"[{i}]", self._data.child(i).child(0)


class OptionalPrinter(Printer):
    T: object

    def __init__(self, T):
        super().__init__()
        self.T = T

    _data: object
    _active: object

    def update(self, val):
        self._data = val["data_"]
        self._active = val["valueBit_"]["data_"]

    def repr(self):
        return f"Optional{self.T}"

    def children(self):
        i = int(self._active)
        if not i:
            yield f"active", self._active
            return
        yield f"value", self._data.cast(self.T)


class VariantPrinter(Printer):
    E: object
    Ts: object

    def __init__(self, E, Ts):
        super().__init__()
        self.E = E
        self.Ts = Ts

    _data: object
    _active: object

    def update(self, val):
        self._data = val["data_"]
        self._active = val["active_"]["data_"]

    def repr(self):
        return f"Variant"

    def children(self):
        i = int(self._active)
        if i not in range(1, 1 + len(self.Ts)):
            yield f"active", self._active
            return
        t = self.Ts[i - 1]
        yield f"[{i}:{str(t) or '<unnamed>'}]", self._data.cast(t)


class SpanPrinter(Printer):
    T: object
    N: object

    def __init__(self, T, N):
        super().__init__()
        self.T = T
        self.N = N
        # self._size = N

    _data: object
    _size: object

    def update(self, val):
        self._data = val["data_"]
        self._size = val["size_"]["data_"]

    def repr(self):
        return f"{self.T}[{self._size}]"

    def children(self):
        for i in range(self._size):
            yield f"[{i}]", self._data[i].cast(self.T)


class StringSpanPrinter(Printer):
    _data: object
    _size: object

    def update(self, val):
        span = val["data_"]
        self._data = span["data_"]
        self._size = span["size_"]["data_"]

    def repr(self):
        s = "".join(chr(int(self._data[i]["data_"])) for i in range(self._size))
        return f"String ({self._size}) {repr(s)}"


class ArrayPrinter(SpanPrinter):
    def update(self, val):
        self._data = val["data_"]
        self._size = self.N


class DynArrayPrinter(SpanPrinter):
    def __init__(self, T):
        super().__init__(T, "*")


class StringPrinter(StringSpanPrinter):
    pass


class ListPrinter(DynArrayPrinter):
    def __init__(self, T):
        super().__init__(T)

    def update(self, val):
        super(ListPrinter, self).update(val)
        self._data = self._data["data_"]


def register():
    return {
        "tier0::Byte": WordPrinter(),
        "tier0::UShort": WordPrinter(),
        "tier0::Short": WordPrinter(),
        "tier0::UInt": WordPrinter(),
        "tier0::Int": WordPrinter(),
        "tier0::ULong": WordPrinter(),
        "tier0::Long": WordPrinter(),
        # "tier0::ptr": PointerPrinter(),
        "tier0::Tuple": {
            None: lambda *Ts: TuplePrinter(Ts)
        },
        "tier0::Variant": {
            None: lambda E: {
                None: lambda *Ts: VariantPrinter(E, Ts)
            }
        },
        "tier0::Optional": {
            None: lambda T: OptionalPrinter(T)
        },
        "tier0::Span": {
            None: lambda T: {
                None: lambda N: SpanPrinter(T, N),
            }
        },
        "tier0::StringSpan": StringSpanPrinter(),
        "tier0::Array": {
            None: lambda T: {
                None: lambda N: ArrayPrinter(T, N),
            }
        },
        "tier1::DynArray": {
            None: lambda T: DynArrayPrinter(T),
        },
        "tier1::String": StringPrinter(),
        "tier2::List": {
            None: lambda T: ListPrinter(T),
        },
    }

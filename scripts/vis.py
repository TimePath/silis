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
    wordValue: object

    def update(self, val):
        self.wordValue = val["wordValue"]

    def repr(self):
        return self.wordValue


class PointerPrinter(Printer):
    _value: object

    def update(self, val):
        self._value = val["_value"]

    def repr(self):
        return self._value


class VariantPrinter(Printer):
    E: object
    Ts: object

    def __init__(self, E, Ts):
        super().__init__()
        self.E = E
        self.Ts = Ts

    _active: object
    _u: object

    def update(self, val):
        self._active = val["active"]["wordValue"]
        self._u = val["u"]
        pass

    def repr(self):
        return f"Variant"

    def children(self):
        i = int(self._active)
        if not i:
            yield f"active", self._active
            return
        t = self.Ts[i - 1]
        yield f"[{t}]", self._u.cast(t)


class ArrayPrinter(Printer):
    T: object
    N: object

    def __init__(self, T, N):
        super().__init__()
        self.T = T
        self.N = N

    _data: object

    def update(self, val):
        self._data = val["_data"]

    def repr(self):
        return f"{self.T}[{self.N}]"

    def children(self):
        for i in range(self.N):
            yield f"[{i}]", self._data[i]


class DynArrayPrinter(Printer):
    T: object

    def __init__(self, T):
        super().__init__()
        self.T = T

    _size: object
    _data: object

    def update(self, val):
        self._size = val["_size"]["wordValue"]
        self._data = val["memory"]

    def repr(self):
        return f"{self.T}[{int(self._size)}]"

    def children(self):
        for i in range(self._size):
            yield f"[{i}]", self._data[i].cast(self.T)


class ListPrinter(DynArrayPrinter):
    def __init__(self, T):
        super().__init__(T)
        self.T = T

    def update(self, val):
        super(ListPrinter, self).update(val)
        self._data = self._data["memory"]


def register():
    return {
        "tier0::Byte": WordPrinter(),
        "tier0::UShort": WordPrinter(),
        "tier0::Short": WordPrinter(),
        "tier0::UInt": WordPrinter(),
        "tier0::Int": WordPrinter(),
        # "tier0::ptr": PointerPrinter(),
        "tier0::Variant": {
            None: lambda E: {
                None: lambda *Ts: VariantPrinter(E, Ts)
            }
        },
        "tier0::Span": {
            None: lambda T: {
                None: lambda N: ArrayPrinter(T, N),
            }
        },
        "tier0::Array": {
            None: lambda T: {
                None: lambda N: ArrayPrinter(T, N),
            }
        },
        "tier1::DynArray": {
            None: lambda T: DynArrayPrinter(T),
        },
        "tier2::List": {
            None: lambda T: ListPrinter(T),
        },
    }

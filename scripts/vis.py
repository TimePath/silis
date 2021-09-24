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


class ArrayPrinter(Printer):
    def __init__(self, T, N):
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


def register():
    return {
        "tier0::Byte": WordPrinter(),
        "tier0::Int": WordPrinter(),
        "tier0::SizedArray": {
            None: lambda T: {
                None: lambda N: ArrayPrinter(T, N),
            }
        },
    }

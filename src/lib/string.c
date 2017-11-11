#include "string.h"

bool str_equals(string_view_t self, string_view_t other) {
    const size_t selfSize = str_size(self);
    const size_t otherSize = str_size(other);
    return selfSize == otherSize && memcmp(self.begin, other.begin, selfSize) == 0;
}

#include "number.h"

#include <math.h>

size_t num_size_base10(size_t n) {
    size_t zeros = (size_t) log10(n);
    return zeros + 1;
}

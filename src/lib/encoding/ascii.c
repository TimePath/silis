#include <system.h>

#include <lib/slice.h>
#include <lib/string.h>

static size_t ascii_point_units(size_t c);

static size_t ascii_count_points(Slice(uint8_t) slice);

static size_t ascii_count_units(Slice(uint8_t) slice);

static Slice(uint8_t) ascii_next(Slice(uint8_t) slice);

static Slice(uint8_t) ascii_skip_units(Slice(uint8_t) slice, size_t n);

static size_t ascii_get(Slice(uint8_t) slice);

StringEncoding _ENCODING_ASCII = {
        .name = "ascii",
        .size_point = 1,
        .size_unit = 1,
        .point_units = ascii_point_units,
        .count_points = ascii_count_points,
        .count_units = ascii_count_units,
        .next = ascii_next,
        .skip_units = ascii_skip_units,
        .get = ascii_get,
};

static size_t ascii_point_units(size_t c)
{
    (void) c;
    return 1;
}

static size_t ascii_count_points(Slice(uint8_t) slice)
{
    return Slice_size(&slice);
}

static size_t ascii_count_units(Slice(uint8_t) slice)
{
    return Slice_size(&slice);
}

static Slice(uint8_t) ascii_next(Slice(uint8_t) slice)
{
    return (Slice(uint8_t)) {._begin.r = Slice_begin(&slice) + 1, ._end = Slice_end(&slice)};
}

static Slice(uint8_t) ascii_skip_units(Slice(uint8_t) slice, size_t n)
{
    return (Slice(uint8_t)) {._begin.r = Slice_begin(&slice) + n, ._end = Slice_end(&slice)};
}

static size_t ascii_get(Slice(uint8_t) slice)
{
    return *Slice_begin(&slice);
}

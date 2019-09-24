#pragma once

#include <lib/slice.h>
#include <lib/vector.h>

Ref_instantiate(Type, size_t);

Slice_instantiate(Ref(Type));
Vector_instantiate(Ref(Type));

#define Type(id, _) \
    /** The leaf types of the type system */ \
    _(id, Opaque, { \
        size_t size; \
    }) \
    /** A function, represented as arg0 -> argN -> ret */ \
    _(id, Function, { \
        Ref(Type) in; \
        Ref(Type) out; \
    }) \
    /** Structs. TODO */ \
    _(id, Aggregate, { \
        /** index to self */ \
        Ref(Type) self; \
        /** index to next child, or 0 */ \
        Ref(Type) next; \
    }) \
    /** Pointers. TODO */ \
    _(id, Pointer, { \
        Ref(Type) pointee; \
    }) \
/**/

ENUM(Type)

Slice_instantiate(Type);
Vector_instantiate(Type);

#define Type_delete(self) ((void) (self))

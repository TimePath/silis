#pragma once

#include <lib/slice.h>
#include <lib/vector.h>

Ref_instantiate(Type, size_t);

Slice_instantiate(Ref(Type));
Vector_instantiate(Ref(Type));

#define Type(_, case) \
    /** The leaf types of the type system */ \
    case(_, Opaque, struct { \
        size_t size; \
    }) \
    /** A function, represented as arg0 -> argN -> ret */ \
    case(_, Function, struct { \
        Ref(Type) in; \
        Ref(Type) out; \
    }) \
    /** Structs. TODO */ \
    case(_, Aggregate, struct { \
        /** index to self */ \
        Ref(Type) self; \
        /** index to next child, or 0 */ \
        Ref(Type) next; \
    }) \
    /** Pointers. TODO */ \
    case(_, Pointer, struct { \
        Ref(Type) pointee; \
    }) \
/**/

ADT_instantiate(Type);
#undef Type

Slice_instantiate(Type);
Vector_instantiate(Type);

#define Type_delete(self) ((void) (self))

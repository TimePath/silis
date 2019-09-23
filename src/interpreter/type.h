#pragma once

#include <lib/slice.h>
#include <lib/vector.h>

Ref_instantiate(Type, size_t);

typedef Ref(Type) TypeRef;

Slice_instantiate(TypeRef);
Vector_instantiate(TypeRef);

#define Type(id, _) \
    /** The leaf types of the type system */ \
    _(id, Opaque, { \
        size_t size; \
    }) \
    /** A function, represented as arg0 -> argN -> ret */ \
    _(id, Function, { \
        TypeRef in; \
        TypeRef out; \
    }) \
    /** Structs. TODO */ \
    _(id, Aggregate, { \
        /** index to self */ \
        TypeRef self; \
        /** index to next child, or 0 */ \
        TypeRef next; \
    }) \
    /** Pointers. TODO */ \
    _(id, Pointer, { \
        TypeRef pointee; \
    }) \
/**/

ENUM(Type)

Slice_instantiate(Type);
Vector_instantiate(Type);

#define Type_delete(self) ((void) (self))

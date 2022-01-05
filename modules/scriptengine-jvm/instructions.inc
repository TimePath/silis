#pragma once
// SPDX-License-Identifier: AFL-3.0

// https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-6.html

#define INSTRUCTIONS(X) \
    X(, athrow, 0xbf, {}, ()) \
    X(, monitorenter, 0xc2, {}, ()) \
    X(, monitorexit, 0xc3, {}, ()) \
    X(, nop, 0x0, {}, ()) \
    X(_, return, 0xb1, {}, ()) \
    X(, wide, 0xc4, { /* TODO */ }, ()) \
    INSTRUCTIONS_CLASS(X) \
    INSTRUCTIONS_INVOKE(X) \
    INSTRUCTIONS_STACK(X) \
    INSTRUCTIONS_BRANCH(X) \
    INSTRUCTIONS_REFERENCE(X) \
    INSTRUCTIONS_BYTE(X) \
    INSTRUCTIONS_CHAR(X) \
    INSTRUCTIONS_SHORT(X) \
    INSTRUCTIONS_INT(X) \
    INSTRUCTIONS_LONG(X) \
    INSTRUCTIONS_FLOAT(X) \
    INSTRUCTIONS_DOUBLE(X) \
    /**/

#define INSTRUCTIONS_CLASS(X) \
    X(, anewarray, 0xbd, { UShort index; }, (self.index)) \
    X(, arraylength, 0xbe, {}, ()) \
    X(, checkcast, 0xc0, { UShort index; }, (self.index)) \
    X(, getfield, 0xb4, { UShort index; }, (self.index)) \
    X(, getstatic, 0xb2, { UShort index; }, (self.index)) \
    X(, instanceof, 0xc1, { UShort index; }, (self.index)) \
    X(, multianewarray, 0xc5, { UShort index; Byte dimensions; }, (self.index, self.dimensions)) \
    X(_, new, 0xbb, { UShort index; }, (self.index)) \
    X(, newarray, 0xbc, { Byte atype; }, (self.atype)) \
    X(, putfield, 0xb5, { UShort index; }, (self.index)) \
    X(, putstatic, 0xb3, { UShort index; }, (self.index)) \
    /**/

#define INSTRUCTIONS_INVOKE(X) \
    X(invoke, dynamic, 0xba, { UShort index; UShort _pad; }, (self.index, self._pad)) \
    X(invoke, interface, 0xb9, { UShort index; Byte _count; Byte _pad; }, (self.index, self._count, self._pad)) \
    X(invoke, special, 0xb7, { UShort index; }, (self.index)) \
    X(invoke, static, 0xb8, { UShort index; }, (self.index)) \
    X(invoke, virtual, 0xb6, { UShort index; }, (self.index)) \
    /**/

#define INSTRUCTIONS_STACK(X) \
    X(, dup, 0x59, {}, ()) \
    X(, dup_x1, 0x5a, {}, ()) \
    X(, dup_x2, 0x5b, {}, ()) \
    X(, dup2, 0x5c, {}, ()) \
    X(, dup2_x1, 0x5d, {}, ()) \
    X(, dup2_x2, 0x5e, {}, ()) \
    X(, ldc, 0x12, { Byte index; }, (self.index)) \
    X(, ldc_w, 0x13, { UShort index; }, (self.index)) \
    X(, ldc2_w, 0x14, { UShort index; }, (self.index)) \
    X(, pop, 0x57, {}, ()) \
    X(, swap, 0x5f, {}, ()) \
    /**/

#define INSTRUCTIONS_BRANCH(X) \
    INSTRUCTIONS_BRANCH_BASIC(X) \
    INSTRUCTIONS_BRANCH_ADVANCED(X) \
    /**/

#define INSTRUCTIONS_BRANCH_BASIC(X) \
    X(_, goto, 0xa7, { Short branch; }, (self.branch)) \
    X(, goto_w, 0xc8, { Int branch; }, (self.branch)) \
    X(, if_acmpeq, 0xa5, { Short branch; }, (self.branch)) \
    X(, if_acmpne, 0xa6, { Short branch; }, (self.branch)) \
    X(, if_icmpeq, 0x9f, { Short branch; }, (self.branch)) \
    X(, if_icmpge, 0xa2, { Short branch; }, (self.branch)) \
    X(, if_icmpgt, 0xa3, { Short branch; }, (self.branch)) \
    X(, if_icmple, 0xa4, { Short branch; }, (self.branch)) \
    X(, if_icmplt, 0xa1, { Short branch; }, (self.branch)) \
    X(, if_icmpne, 0xa0, { Short branch; }, (self.branch)) \
    X(, ifeq, 0x99, { Short branch; }, (self.branch)) \
    X(, ifge, 0x9c, { Short branch; }, (self.branch)) \
    X(, ifgt, 0x9d, { Short branch; }, (self.branch)) \
    X(, ifle, 0x9e, { Short branch; }, (self.branch)) \
    X(, iflt, 0x9b, { Short branch; }, (self.branch)) \
    X(, ifne, 0x9a, { Short branch; }, (self.branch)) \
    X(, ifnonnull, 0xc7, { Short branch; }, (self.branch)) \
    X(, ifnull, 0xc6, { Short branch; }, (self.branch)) \
    X(, jsr, 0xa8, { Short branch; }, (self.branch)) \
    X(, jsr_w, 0xc9, { Int branch; }, (self.branch)) \
    /**/

#define INSTRUCTIONS_BRANCH_ADVANCED(X) \
    X(, lookupswitch, 0xab, { UInt _default; UInt npairs; /* TODO */ }, ()) \
    X(, ret, 0xa9, { Byte index; }, (self.index)) \
    X(, tableswitch, 0xaa, { UInt _default; UInt low; UInt high; /* TODO */ }, ()) \
    /**/

#define INSTRUCTIONS_REFERENCE(X) \
    X(a, aload, 0x32, {}, ()) \
    X(a, astore, 0x53, {}, ()) \
    X(a, const_null, 0x1, {}, ()) \
    X(a, load, 0x19, { Byte index; }, (self.index)) \
    X(a, load_0, 0x2a, {}, ()) \
    X(a, load_1, 0x2b, {}, ()) \
    X(a, load_2, 0x2c, {}, ()) \
    X(a, load_3, 0x2d, {}, ()) \
    X(a, return, 0xb0, {}, ()) \
    X(a, store, 0x3a, { Byte index; }, (self.index)) \
    X(a, store_0, 0x4b, {}, ()) \
    X(a, store_1, 0x4c, {}, ()) \
    X(a, store_2, 0x4d, {}, ()) \
    X(a, store_3, 0x4e, {}, ()) \
    /**/

#define INSTRUCTIONS_BYTE(X) \
    X(b, aload, 0x33, {}, ()) \
    X(b, astore, 0x54, {}, ()) \
    X(b, ipush, 0x10, { Int8 value; }, (self.value)) \
    /**/

#define INSTRUCTIONS_CHAR(X) \
    X(c, aload, 0x34, {}, ()) \
    X(c, astore, 0x55, {}, ())\
    /**/

#define INSTRUCTIONS_SHORT(X) \
    X(s, aload, 0x35, {}, ()) \
    X(s, astore, 0x56, {}, ()) \
    X(s, ipush, 0x11, { Short value; }, (self.value)) \
    /**/

#define INSTRUCTIONS_INT(X) \
    X(i, 2b, 0x91, {}, ()) \
    X(i, 2c, 0x92, {}, ()) \
    X(i, 2d, 0x87, {}, ()) \
    X(i, 2f, 0x86, {}, ()) \
    X(i, 2l, 0x85, {}, ()) \
    X(i, 2s, 0x93, {}, ()) \
    X(i, add, 0x60, {}, ()) \
    X(i, aload, 0x2e, {}, ()) \
    X(i, and, 0x7e, {}, ()) \
    X(i, astore, 0x4f, {}, ()) \
    X(i, const_0, 0x3, {}, ()) \
    X(i, const_1, 0x4, {}, ()) \
    X(i, const_2, 0x5, {}, ()) \
    X(i, const_3, 0x6, {}, ()) \
    X(i, const_4, 0x7, {}, ()) \
    X(i, const_5, 0x8, {}, ()) \
    X(i, const_m1, 0x2, {}, ()) \
    X(i, div, 0x6c, {}, ()) \
    X(i, inc, 0x84, { Byte index; Int8 value; }, (self.index, self.value)) \
    X(i, load, 0x15, { Byte index; }, (self.index)) \
    X(i, load_0, 0x1a, {}, ()) \
    X(i, load_1, 0x1b, {}, ()) \
    X(i, load_2, 0x1c, {}, ()) \
    X(i, load_3, 0x1d, {}, ()) \
    X(i, mul, 0x68, {}, ()) \
    X(i, neg, 0x74, {}, ()) \
    X(i, or, 0x80, {}, ()) \
    X(i, rem, 0x70, {}, ()) \
    X(i, return, 0xac, {}, ()) \
    X(i, shl, 0x78, {}, ()) \
    X(i, shr, 0x7a, {}, ()) \
    X(i, store, 0x36, { Byte index; }, (self.index)) \
    X(i, store_0, 0x3b, {}, ()) \
    X(i, store_1, 0x3c, {}, ()) \
    X(i, store_2, 0x3d, {}, ()) \
    X(i, store_3, 0x3e, {}, ()) \
    X(i, sub, 0x64, {}, ()) \
    X(i, ushr, 0x7c, {}, ()) \
    X(i, xor, 0x82, {}, ()) \
    /**/

#define INSTRUCTIONS_LONG(X) \
    X(l, 2d, 0x8a, {}, ()) \
    X(l, 2f, 0x89, {}, ()) \
    X(l, 2i, 0x88, {}, ()) \
    X(l, add, 0x61, {}, ()) \
    X(l, aload, 0x2f, {}, ()) \
    X(l, and, 0x7f, {}, ()) \
    X(l, astore, 0x50, {}, ()) \
    X(l, cmp, 0x94, {}, ()) \
    X(l, const_0, 0x9, {}, ()) \
    X(l, const_1, 0xa, {}, ()) \
    X(l, div, 0x6d, {}, ()) \
    X(l, load, 0x16, { Byte index; }, (self.index)) \
    X(l, load_0, 0x1e, {}, ()) \
    X(l, load_1, 0x1f, {}, ()) \
    X(l, load_2, 0x20, {}, ()) \
    X(l, load_3, 0x21, {}, ()) \
    X(l, mul, 0x69, {}, ()) \
    X(l, neg, 0x75, {}, ()) \
    X(l, or, 0x81, {}, ()) \
    X(l, rem, 0x71, {}, ()) \
    X(l, return, 0xad, {}, ()) \
    X(l, shl, 0x79, {}, ()) \
    X(l, shr, 0x7b, {}, ()) \
    X(l, store, 0x37, { Byte index; }, (self.index)) \
    X(l, store_0, 0x3f, {}, ()) \
    X(l, store_1, 0x40, {}, ()) \
    X(l, store_2, 0x41, {}, ()) \
    X(l, store_3, 0x42, {}, ()) \
    X(l, sub, 0x65, {}, ()) \
    X(l, ushr, 0x7d, {}, ()) \
    X(l, xor, 0x83, {}, ()) \
    /**/

#define INSTRUCTIONS_FLOAT(X) \
    X(f, 2d, 0x8d, {}, ()) \
    X(f, 2i, 0x8b, {}, ()) \
    X(f, 2l, 0x8c, {}, ()) \
    X(f, add, 0x62, {}, ()) \
    X(f, aload, 0x30, {}, ()) \
    X(f, astore, 0x51, {}, ()) \
    X(f, cmpg, 0x96, {}, ()) \
    X(f, cmpl, 0x95, {}, ()) \
    X(f, const_0, 0xb, {}, ()) \
    X(f, const_1, 0xc, {}, ()) \
    X(f, const_2, 0xd, {}, ()) \
    X(f, div, 0x6e, {}, ()) \
    X(f, load, 0x17, { Byte index; }, (self.index)) \
    X(f, load_0, 0x22, {}, ()) \
    X(f, load_1, 0x23, {}, ()) \
    X(f, load_2, 0x24, {}, ()) \
    X(f, load_3, 0x25, {}, ()) \
    X(f, mul, 0x6a, {}, ()) \
    X(f, neg, 0x76, {}, ()) \
    X(f, rem, 0x72, {}, ()) \
    X(f, return, 0xae, {}, ()) \
    X(f, store, 0x38, { Byte index; }, (self.index)) \
    X(f, store_0, 0x43, {}, ()) \
    X(f, store_1, 0x44, {}, ()) \
    X(f, store_2, 0x45, {}, ()) \
    X(f, store_3, 0x46, {}, ()) \
    X(f, sub, 0x66, {}, ()) \
    /**/

#define INSTRUCTIONS_DOUBLE(X) \
    X(d, 2f, 0x90, {}, ()) \
    X(d, 2i, 0x8e, {}, ()) \
    X(d, 2l, 0x8f, {}, ()) \
    X(d, add, 0x63, {}, ()) \
    X(d, aload, 0x31, {}, ()) \
    X(d, astore, 0x52, {}, ()) \
    X(d, cmpg, 0x98, {}, ()) \
    X(d, cmpl, 0x97, {}, ()) \
    X(d, const_0, 0xe, {}, ()) \
    X(d, const_1, 0xf, {}, ()) \
    X(d, div, 0x6f, {}, ()) \
    X(d, load, 0x18, { Byte index; }, (self.index)) \
    X(d, load_0, 0x26, {}, ()) \
    X(d, load_1, 0x27, {}, ()) \
    X(d, load_2, 0x28, {}, ()) \
    X(d, load_3, 0x29, {}, ()) \
    X(d, mul, 0x6b, {}, ()) \
    X(d, neg, 0x77, {}, ()) \
    X(d, rem, 0x73, {}, ()) \
    X(d, return, 0xaf, {}, ()) \
    X(d, store, 0x39, { Byte index; }, (self.index)) \
    X(d, store_0, 0x47, {}, ()) \
    X(d, store_1, 0x48, {}, ()) \
    X(d, store_2, 0x49, {}, ()) \
    X(d, store_3, 0x4a, {}, ()) \
    X(d, sub, 0x67, {}, ()) \
    /**/

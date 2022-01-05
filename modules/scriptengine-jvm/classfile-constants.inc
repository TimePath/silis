#pragma once
// SPDX-License-Identifier: AFL-3.0

// https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-4.html

#define CONSTANTS(X) \
    X(Utf8, 1, { \
        Utf8String string; \
    }, (self.string)) \
    X(Integer, 3, { \
        UInt bytes; \
    }, (self.bytes)) \
    X(Float, 4, { \
        UInt bytes; \
    }, (self.bytes)) \
    X(Long, 5, { \
        UInt highBytes; \
        UInt lowBytes; \
    }, (self.highBytes, self.lowBytes)) \
    X(Double, 6, { \
        UInt highBytes; \
        UInt lowBytes; \
    }, (self.highBytes, self.lowBytes)) \
    X(Class, 7, { \
        UShort nameIndex; \
    }, (self.nameIndex)) \
    X(String, 8, { \
        UShort stringIndex; \
    }, (self.stringIndex)) \
    X(Fieldref, 9, { \
        UShort classIndex; \
        UShort nameAndTypeIndex; \
    }, (self.classIndex, self.nameAndTypeIndex)) \
    X(Methodref, 10, { \
        UShort classIndex; \
        UShort nameAndTypeIndex; \
    }, (self.classIndex, self.nameAndTypeIndex)) \
    X(InterfaceMethodref, 11, { \
        UShort classIndex; \
        UShort nameAndTypeIndex; \
    }, (self.classIndex, self.nameAndTypeIndex)) \
    X(NameAndType, 12, { \
        UShort nameIndex; \
        UShort descriptorIndex; \
    }, (self.nameIndex, self.descriptorIndex)) \
    X(MethodHandle, 15, { \
        Byte referenceKind; \
        UShort referenceIndex; \
    }, (self.referenceKind, self.referenceIndex)) \
    X(MethodType, 16, { \
        UShort descriptorIndex; \
    }, (self.descriptorIndex)) \
    X(Dynamic, 17, { \
        UShort bootstrapMethodAttrIndex; \
        UShort nameAndTypeIndex; \
    }, (self.bootstrapMethodAttrIndex, self.nameAndTypeIndex)) \
    X(InvokeDynamic, 18, { \
        UShort bootstrapMethodAttrIndex; \
        UShort nameAndTypeIndex; \
    }, (self.bootstrapMethodAttrIndex, self.nameAndTypeIndex)) \
    X(Module, 19, { \
        UShort nameIndex; \
    }, (self.nameIndex)) \
    X(Package, 20, { \
        UShort nameIndex; \
    }, (self.nameIndex)) \
    /**/

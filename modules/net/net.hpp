#pragma once

#include "../tier0/tier0.hpp"

namespace net::ethernet {
    using namespace tier0;
}

namespace net::ethernet {
    struct Address {
        Span<Byte, Size(6)> span_;

        explicit Address(Span<Byte, Size(6)> span) : span_(span) {}

        void print() {
            let span = span_;
            printf("%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX",
                   Native<Byte>(span.get(0)), Native<Byte>(span.get(1)), Native<Byte>(span.get(2)),
                   Native<Byte>(span.get(3)), Native<Byte>(span.get(4)), Native<Byte>(span.get(5))
            );
        }
    };

    struct Ethernet2 {
        Span<Byte, Size(6 + 6 + 2)> span_;

        explicit Ethernet2(Span<Byte, Size(6 + 6 + 2)> span) : span_(span) {}

        [[nodiscard]]
        Address dst() const { return Address(span_); }

        [[nodiscard]]
        Address src() const { return Address(span_.offset<Size(6)>()); }

        [[nodiscard]]
        UShort type() const {
            let span = span_.offset<Size(6 + 6)>();
            return UShort(span.get(0) << 8 | span.get(1));
        }
    };
}

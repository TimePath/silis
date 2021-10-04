#pragma once

namespace net::ethernet {
    struct Address {
        Span<Byte, Size(6)> _span;

        explicit Address(Span<Byte, Size(6)> span) : _span(span) {}

        void print() {
            let span = _span;
            printf("%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX",
                   span.get(0).wordValue, span.get(1).wordValue, span.get(2).wordValue,
                   span.get(3).wordValue, span.get(4).wordValue, span.get(5).wordValue
            );
        }
    };

    struct Ethernet2 {
        Span<Byte, Size(6 + 6 + 2)> _span;

        explicit Ethernet2(Span<Byte, Size(6 + 6 + 2)> span) : _span(span) {}

        [[nodiscard]]
        Address dst() const { return Address(_span); }

        [[nodiscard]]
        Address src() const { return Address(_span.offset<Size(6)>()); }

        [[nodiscard]]
        UShort type() const {
            let span = _span.offset<Size(6 + 6)>();
            return UShort(span.get(0) << 8 | span.get(1));
        }
    };
}

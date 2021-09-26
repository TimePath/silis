#pragma once

namespace net::ethernet {
    struct Address {
        Span<Byte, 6> _span;

        explicit Address(Span<Byte, 6> span) : _span(span) {}

        void print() {
            let d = _span._data;
            printf("%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX" ":" "%02hhX",
                   d[0].wordValue, d[1].wordValue, d[2].wordValue,
                   d[3].wordValue, d[4].wordValue, d[5].wordValue
            );
        }
    };

    struct Ethernet2 {
        Span<Byte, 6 + 6 + 2> _span;

        explicit Ethernet2(Span<Byte, 6 + 6 + 2> span) : _span(span) {}

        [[nodiscard]]
        Address dst() const { return Address(_span); }

        [[nodiscard]]
        Address src() const { return Address(_span.offset<6>()); }

        [[nodiscard]]
        UShort type() const {
            let span = _span.offset<6 + 6>()._data;
            return UShort(span[0] << 8 | span[1]);
        }
    };
}

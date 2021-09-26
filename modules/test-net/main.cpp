#include <cstdio>
#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"
#include "../tier2/tier2.hpp"
#include "../kernel/kernel.hpp"
#include "../net/net.hpp"

using namespace tier2;

Native<Int> main(Native<Int> argc, Array<cstring, Size(1)>::array_type argv) {
    (void) argc;
    let handle = interface_open(argv[1]);
    var buffer = Array<Byte, Size(0xffff + 1)>();
    let span = buffer.asSpan();
    while (true) {
        let ret = interface_read(handle, span);
        if (ret < 0) {
            break;
        }
        printf("\n");

        let frame = net::ethernet::Ethernet2(span);
        printf("frame.dst: ");
        frame.dst().print();
        printf("\n");
        printf("frame.src: ");
        frame.src().print();
        printf("\n");
        let type = frame.type();
        printf("frame.type: 0x%04hX\n", type.wordValue);
        fflush(stdout);
    }
    return 0;
}

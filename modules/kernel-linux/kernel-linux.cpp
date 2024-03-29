// SPDX-License-Identifier: AFL-3.0
#define ENABLE_EXTRAS 0

#if ENABLE_EXTRAS
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#endif

#include <cerrno>
#include <cstdio>
#include <cstring>

#define LIBKERNEL_EXPORTS

#include "../kernel/kernel.hpp"

namespace kernel {
    DynArray<Byte> file_read(cstring path) {
        var fd = fopen(path, "rb");
        if (!fd) {
            var err = strerror(errno);
            assert(!err);
        }
        fseek(fd, 0, SEEK_END);
        var size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        var ret = DynArray<Byte>::uninitialized(Int(size));
        var readRet = fread(&ret.get(0), 1, Native<Size>(size), fd);
        assert(Size(readRet) == Size(size));
        fclose(fd);
        return ret;
    }

    Int interface_open(cstring name) {
        (void) name;
        return -1;
#if ENABLE_EXTRAS
        var interfaces = if_nameindex();
        if (!interfaces) {
            perror("if_nameindex");
            return -1;
        }
        for (var i = 0;; ++i) {
            var it = interfaces[i];
            if (!it.if_index) break;
            printf("iface: %s\n", it.if_name);
        }
        if_freenameindex(interfaces);

        var idx = if_nametoindex(name);
        if (idx == 0) {
            perror("if_nametoindex");
            return -2;
        }

        let handle = socket(PF_PACKET, SOCK_RAW, htobe16(ETH_P_ALL));
        if (handle == -1) {
            perror("socket");
            return -3;
        }

        var sll = sockaddr_ll();
        sll.sll_family = AF_PACKET;
        sll.sll_protocol = htobe16(ETH_P_ALL);
        sll.sll_ifindex = Native<Int>(idx);
        if (bind(handle, reinterpret_cast<Native<ptr<sockaddr>>>(&sll), sizeof(sll)) == -1) {
            perror("socket");
            return -4;
        }

        var req = packet_mreq();
        req.mr_ifindex = Native<Int>(idx);
        req.mr_type = PACKET_MR_PROMISC;
        if (setsockopt(handle, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &req, sizeof(req)) == -1) {
            perror("setsockopt");
            return -5;
        }

        printf("handle: %d\n", handle);
        return handle;
#endif
    }

    Int interface_read(Int handle, Span<Byte, Size(0xffff + 1)> span) {
        (void) handle;
        (void) span;
        return -1;
#if ENABLE_EXTRAS
        var addr = sockaddr_ll();
        static_assert(sizeof(addr) <= sizeof(sockaddr_storage));
        var addrSize = socklen_t(sizeof(addr));
        let size = recvfrom(handle,
                            &span.data_, span.size(),
                            MSG_TRUNC,
                            reinterpret_cast<Native<ptr<sockaddr>>>(&addr), &addrSize);
        // response from interface: addr.sll_ifindex
        if (errno) {
            let errStr = strerror(errno);
            printf("recvfrom: %s\n", errStr);
            return -2;
        }
        if (size == -1) {
            return -1;
        }
        return Int(size);
#endif
    }
}

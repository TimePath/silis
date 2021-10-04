#include "../../test/test.hpp"

#include "../../tier0/tier0.hpp"
#include "../../tier1/tier1.hpp"

#include "../../kernel/kernel.hpp"
#include "../scriptengine.hpp"

using namespace test;

TEST("LoadClass") {
    var data = file_read("modules/scriptengine-jvm/tests/Main.class");
    var ret = scriptengine::jvm::LoadClass(move(data));
    scriptengine::jvm::LoadCode({ret, 1});
    ret.release();
    printf("LoadClass\n");
}

// Toy test for toy.h. Not named *_test.cpp so run_tests.py discovery never picks it up; tests drive it explicitly.
#include "toy.h"
#include "check.h"
#ifndef TOY_FIXTURE
#error "test.toml cxxflags were not applied"
#endif
int main() {
    KT_CHECK(isAdult(18));
    KT_CHECK(!isAdult(17));
    KT_CHECK_EQ(scale(-5), -1);
    KT_CHECK_EQ(scale(0), 0);
    KT_CHECK_EQ(scale(3), 6);
    KT_CHECK_EQ(needTrue(), 1);
    KT_CHECK_EQ(label()[0], 'a');
    return kt::Report("toy_check");
}

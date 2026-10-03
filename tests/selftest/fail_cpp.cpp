// Deliberately failing test for `scripts/run_tests.py --selftest`.
// Proves the runner reports a failing check as FAIL, including under -DNDEBUG
// (KT_CHECK never uses assert). Not matched by discovery (not *_test.cpp).
#include "check.h"

int main() {
    KT_CHECK(2 + 2 == 4);   // passing check
    KT_CHECK(1 == 2);       // deliberate failure
    return kt::Report("fail_cpp");
}

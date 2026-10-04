// Regression tests for Action Hotbar bugs found in the doc 20 audit.
#include "HotbarConfig.h"
#include "check.h"
#include <climits>

int main() {
    KT_CHECK_EQ(hb::slotDigit(0), '1');
    KT_CHECK_EQ(hb::slotDigit(8), '9');
    KT_CHECK_EQ(hb::slotDigit(9), '0');
    KT_CHECK_EQ(hb::slotDigit(-1), '?');
    KT_CHECK_EQ(hb::slotDigit(10), '?');
    KT_CHECK_EQ(hb::slotDigit(INT_MIN), '?');
    KT_CHECK_EQ(hb::slotDigit(INT_MAX), '?');
    return kt::Report("hotbar_bugs_test");
}

// A.cpp: fixture
static const unsigned kKnown = 0x00500000;      // allowlisted
static const unsigned kOutLow = 0x003FFFFF;     // below range, ignored
static const unsigned kOutHigh = 0x00C00000;    // above range, ignored
static const unsigned kLong = 0x100500000;      // not 8 hex digits, ignored
static const unsigned kPlain = 0x1234;

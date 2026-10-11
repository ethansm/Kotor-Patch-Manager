## Specialty lens: cpp-logic (reviewer)
- Boundaries: compare every changed comparison, loop bound, index and size against _pre/. Look for off-by-one.
- Lifetime: new early returns that skip a release/destroy; flags set before the work they guard succeeds.
- Types: signed/unsigned and 32-bit overflow in new arithmetic; float vs int truncation.

// List every defined function within [START, END), for positional-
// clustering searches once one class member's Steam address is known (GOG's
// AddressDatabases show tight source-order address clustering per class --
// see ghidra_knowledge lessons). Current contents: Buff Duration HUD
// UX-redesign session (U1 step 0) -- window around the extrapolated
// Steam candidate (0x0074ab10) for CSWGuiMainInterfaceChar::Constructor
// (GOG addr 5357840), extrapolated by simple delta from the confirmed
// Initialize address (0x007450e0 = GOG 5334752) across 5 intervening,
// not-yet-Steam-confirmed functions -- looking for the real function
// boundary nearest the candidate.
//
// @category KOTOR
// @menupath Tools.KOTOR.List Functions In Range

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

public class ListFunctionsInRange extends GhidraScript {
    private static final String START = "0074a700";
    private static final String END   = "0074b400";

    @Override
    public void run() throws Exception {
        Address start = currentProgram.getAddressFactory().getAddress("0x" + START);
        Address end   = currentProgram.getAddressFactory().getAddress("0x" + END);
        FunctionIterator it = currentProgram.getListing().getFunctions(start, true);
        while (it.hasNext()) {
            Function fn = it.next();
            if (fn.getEntryPoint().compareTo(end) >= 0) break;
            println(fn.getEntryPoint() + "  size=" + fn.getBody().getNumAddresses() + "  " + fn.getName()
                + "  params=" + fn.getParameterCount());
        }
    }
}

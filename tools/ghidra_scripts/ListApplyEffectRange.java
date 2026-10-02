// @category KOTOR
// @menupath Tools.KOTOR.List Apply Effect Range
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

public class ListApplyEffectRange extends GhidraScript {
    @Override
    public void run() throws Exception {
        Address start = currentProgram.getAddressFactory().getAddress("0x00543000");
        Address end = currentProgram.getAddressFactory().getAddress("0x0054c000");
        println("=== Functions in [00543000,0054c000) ===");
        FunctionIterator it = currentProgram.getFunctionManager().getFunctionsNoStubs(true);
        while (it.hasNext()) {
            Function f = it.next();
            Address a = f.getEntryPoint();
            if (a.compareTo(start) >= 0 && a.compareTo(end) < 0) {
                println("  " + f.getName() + " @ " + a + " size=" + f.getBody().getNumAddresses()
                    + " params=" + f.getParameterCount());
            }
        }
    }
}

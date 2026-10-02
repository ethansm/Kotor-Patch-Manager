// @category KOTOR
// @menupath Tools.KOTOR.Find Call Site Context 0051c350
//
// Item 1 (R3 residual) hop 3: FUN_0051c350 is thiscall, dereferences its
// OWN `this`+0x4 to get FUN_00538e00's `this`. Need to see what ECX is
// set to right before `CALL FUN_0051c350` inside FUN_00569320 (the
// CSWSObject heartbeat) -- is it the SAME object as the heartbeat's own
// `this` (param_1), or something else (e.g. a fixed global address moved
// into ECX)? Dumps the ~20 instructions immediately preceding every CALL
// to 0x0051c350 found anywhere in the binary, plus which function each
// call site is inside.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

public class FindCallSiteContext_0051c350 extends GhidraScript {
    @Override
    public void run() throws Exception {
        Address target = currentProgram.getAddressFactory().getAddress("0x0051c350");
        ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(target);
        while (refs.hasNext()) {
            Reference ref = refs.next();
            if (!ref.getReferenceType().isCall()) continue;
            Address callSite = ref.getFromAddress();
            Function containing = getFunctionContaining(callSite);
            println("");
            println("==== CALL SITE " + callSite + " in " +
                (containing != null ? containing.getName() + "@" + containing.getEntryPoint() : "?") + " ====");
            // walk backwards ~20 instructions from callSite
            Instruction insn = getInstructionAt(callSite);
            java.util.List<Instruction> preceding = new java.util.ArrayList<>();
            Instruction cur = insn;
            for (int i = 0; i < 20 && cur != null; i++) {
                cur = cur.getPrevious();
                if (cur == null) break;
                preceding.add(0, cur);
            }
            for (Instruction p : preceding) {
                println("  " + p.getAddress() + "  " + p.toString());
            }
            println("  " + callSite + "  " + insn.toString() + "   <-- CALL");
        }
    }
}

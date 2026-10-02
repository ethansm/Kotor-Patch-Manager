// @category KOTOR
// args: hexOffset[,hexOffset] -- lists every instruction (with containing function) that has that scalar operand (memory displacement / immediate)
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class FindOffsetUsers extends GhidraScript {
    public void run() throws Exception {
        for (String h : getScriptArgs()[0].split(",")) {
            long k = Long.parseLong(h, 16);
            println("### scalar 0x" + h);
            for (Instruction ins : currentProgram.getListing().getInstructions(true)) {
                for (int i = 0; i < ins.getNumOperands(); i++) {
                    ghidra.program.model.scalar.Scalar s = ins.getScalar(i);
                    if (s != null && s.getUnsignedValue() == k) {
                        Function f = getFunctionContaining(ins.getAddress());
                        println("  " + ins.getAddress() + "  " + ins + "   fn=" + (f == null ? "-" : f.getEntryPoint().toString()));
                    }
                }
            }
        }
    }
}

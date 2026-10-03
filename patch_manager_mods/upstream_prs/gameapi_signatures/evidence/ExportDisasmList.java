// Read-only: disassemble + decompile the functions listed in <outdir>/addrs.txt (one hex address per line).
// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class ExportDisasmList extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args[0]);
        List<String> lines = Files.readAllLines(new File(dir, "addrs.txt").toPath());
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        Listing l = currentProgram.getListing();
        try (PrintWriter w = new PrintWriter(new FileWriter(new File(dir, "disasm.txt")))) {
            for (String s : lines) {
                s = s.trim();
                if (s.isEmpty()) continue;
                Address a = toAddr(Long.parseLong(s.replace("0x", ""), 16));
                Function fn = getFunctionAt(a);
                if (fn == null) fn = getFunctionContaining(a);
                w.println("#### " + s + " fn=" + (fn == null ? "NONE" : fn.getName() + " entry=" + fn.getEntryPoint() + " cc=" + fn.getCallingConventionName() + " proto=" + fn.getPrototypeString(true, true)));
                if (fn == null) continue;
                InstructionIterator it = l.getInstructions(fn.getBody(), true);
                int n = 0;
                while (it.hasNext() && n < 700) {
                    Instruction ins = it.next();
                    StringBuilder b = new StringBuilder();
                    for (byte x : ins.getBytes()) b.append(String.format("%02x", x & 0xff));
                    w.println(ins.getAddress() + "  " + String.format("%-18s", b) + " " + ins.toString());
                    n++;
                }
                try {
                    DecompileResults r = di.decompileFunction(fn, 60, monitor);
                    if (r != null && r.decompileCompleted()) { w.println("--- decompile"); w.println(r.getDecompiledFunction().getC()); }
                } catch (Exception e) { w.println("--- decompile failed " + e); }
            }
        }
    }
}

// Read-only. Reads <outDir>/requests.txt, writes <outDir>/<program>_out.txt. Request lines:
//   FN <hexaddr>            full disassembly of the function containing addr (bytes + text + call-target annotation)
//   DI <hexaddr> <nbytes>   linear disassembly from addr for nbytes
//   RAW <hexaddr> <nbytes>  hex bytes
//   REFS <hexaddr>          references TO addr (from, type, containing function)
//   FIND <hex bytes, ?? wildcard>  all hits in initialized memory (max 60)
//   PTRS <hexaddr> <ndwords> dword table dump with function names for pointer values
// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class ExportRangeDisasm extends GhidraScript {
    PrintWriter w;
    Address A(long a) { return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(a); }
    String fnName(Address a) {
        Function f = currentProgram.getFunctionManager().getFunctionContaining(a);
        return f == null ? "-" : f.getName() + "@" + f.getEntryPoint();
    }
    void insn(Instruction i) {
        StringBuilder b = new StringBuilder();
        try { for (byte x : i.getBytes()) b.append(String.format("%02x ", x)); } catch (Exception e) {}
        String extra = "";
        for (Address t : i.getFlows()) extra += "  ->" + t + "(" + fnName(t) + ")";
        w.println(i.getAddress() + "  " + String.format("%-26s", b) + i + extra);
    }
    public void run() throws Exception {
        String dir = getScriptArgs()[0];
        List<String> reqs = Files.readAllLines(Paths.get(dir, "requests.txt"));
        w = new PrintWriter(new FileWriter(new File(dir, currentProgram.getName() + "_out.txt")));
        Listing L = currentProgram.getListing();
        Memory mem = currentProgram.getMemory();
        for (String line : reqs) {
            line = line.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            String[] p = line.split("\\s+");
            w.println("=== " + line);
            try {
                if (p[0].equals("FN")) {
                    Function f = currentProgram.getFunctionManager().getFunctionContaining(A(Long.parseLong(p[1], 16)));
                    if (f == null) { w.println("NOFUNC"); continue; }
                    w.println("FUNC " + f.getName() + " entry=" + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses() + " conv=" + f.getCallingConventionName() + " thunk=" + f.isThunk());
                    InstructionIterator it = L.getInstructions(f.getBody(), true);
                    while (it.hasNext()) insn(it.next());
                } else if (p[0].equals("DI")) {
                    long a = Long.parseLong(p[1], 16); int n = Integer.parseInt(p[2]);
                    Instruction i = L.getInstructionContaining(A(a));
                    if (i == null) i = L.getInstructionAfter(A(a));
                    while (i != null && i.getAddress().getOffset() < a + n) { insn(i); i = i.getNext(); }
                } else if (p[0].equals("RAW")) {
                    long a = Long.parseLong(p[1], 16); int n = Integer.parseInt(p[2]);
                    byte[] b = new byte[n]; mem.getBytes(A(a), b);
                    StringBuilder sb = new StringBuilder();
                    for (int k = 0; k < n; k++) { if (k % 16 == 0) { if (k > 0) sb.append("\n"); sb.append(String.format("%08x:", a + k)); } sb.append(String.format(" %02x", b[k])); }
                    w.println(sb);
                } else if (p[0].equals("REFS")) {
                    Address t = A(Long.parseLong(p[1], 16));
                    ReferenceIterator ri = currentProgram.getReferenceManager().getReferencesTo(t);
                    int c = 0;
                    while (ri.hasNext() && c < 200) { Reference r = ri.next(); w.println("  from " + r.getFromAddress() + " " + r.getReferenceType() + " in " + fnName(r.getFromAddress())); c++; }
                } else if (p[0].equals("FIND")) {
                    StringBuilder rest = new StringBuilder(); for (int k = 1; k < p.length; k++) rest.append(p[k]);
                    String hx = rest.toString(); int n = hx.length() / 2;
                    byte[] pat = new byte[n], msk = new byte[n];
                    for (int k = 0; k < n; k++) { String s = hx.substring(2 * k, 2 * k + 2); if (s.equals("??")) { pat[k] = 0; msk[k] = 0; } else { pat[k] = (byte) Integer.parseInt(s, 16); msk[k] = (byte) 0xff; } }
                    Address s = mem.getMinAddress(); int c = 0;
                    while (s != null && c < 60) {
                        Address h = mem.findBytes(s, pat, msk, true, monitor);
                        if (h == null) break;
                        w.println("  hit " + h + " in " + fnName(h)); c++;
                        s = h.add(1);
                    }
                    w.println("  total<=" + c);
                } else if (p[0].equals("PTRS")) {
                    long a = Long.parseLong(p[1], 16); int n = Integer.parseInt(p[2]);
                    for (int k = 0; k < n; k++) {
                        int v = mem.getInt(A(a + 4L * k));
                        long u = v & 0xffffffffL;
                        String nm = "";
                        try { Function f = currentProgram.getFunctionManager().getFunctionAt(A(u)); if (f != null) nm = f.getName(); } catch (Exception e) {}
                        w.println(String.format("  %08x [%d] = %08x %s", a + 4L * k, k, u, nm));
                    }
                }
            } catch (Exception e) { w.println("ERR " + e); }
        }
        w.close();
        println("done");
    }
}

// Exports the static call graph: <outdir>/callgraph.tsv (caller_fn, call_site, callee_fn, type, insn) and
// <outdir>/callers_count.tsv (callee_fn, name, fan_in_sites, fan_in_fns) sorted by fan-in.
// type: CALL (direct), TAILJMP (JMP to a function entry), INDIRECT (computed call; callee '-', insn has the operand,
// e.g. vtable slot), IMPORT (call through an IAT slot; callee = import name).
// Headless arg: outdir. Doc 10 Phase 2.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class ExportCallGraph extends GhidraScript {
    static String hx(Address a) { return String.format("%08x", a.getOffset()); }

    @Override
    public void run() throws Exception {
        File root = new File(getScriptArgs()[0]);
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing l = currentProgram.getListing();
        Map<String, Integer> sites = new HashMap<>();
        Map<String, Set<String>> fns = new HashMap<>();
        int n = 0;
        try (PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(new File(root, "callgraph.tsv"))))) {
            w.println("caller_fn\tcall_site\tcallee_fn\ttype\tinsn");
            for (Function f : fm.getFunctions(true)) {
                if (f.isThunk()) continue;
                InstructionIterator it = l.getInstructions(f.getBody(), true);
                while (it.hasNext()) {
                    Instruction ins = it.next();
                    FlowType ft = ins.getFlowType();
                    boolean call = ft.isCall(), jmp = ft.isJump();
                    if (!call && !jmp) continue;
                    List<String[]> outs = new ArrayList<>();
                    for (Address t : ins.getFlows()) {
                        Function g = fm.getFunctionAt(t);
                        if (g == null && call) g = fm.getFunctionContaining(t);
                        if (g == null) continue;
                        if (jmp && (g == f || !g.getEntryPoint().equals(t))) continue;
                        if (g.isThunk()) {
                            Function tg = g.getThunkedFunction(true);
                            if (tg != null && tg.isExternal()) { outs.add(new String[] { tg.getName(), "IMPORT" }); continue; }
                        }
                        outs.add(new String[] { hx(g.getEntryPoint()), call ? "CALL" : "TAILJMP" });
                    }
                    if (outs.isEmpty() && call) {
                        String imp = null;
                        for (Reference r : ins.getReferencesFrom()) {
                            if (r.isExternalReference()) { imp = ((ExternalReference) r).getLabel(); break; }
                            Data dd = l.getDataAt(r.getToAddress());
                            if (dd != null) for (Reference r2 : dd.getReferencesFrom())
                                if (r2.isExternalReference()) { imp = ((ExternalReference) r2).getLabel(); break; }
                            if (imp != null) break;
                        }
                        outs.add(imp != null ? new String[] { imp, "IMPORT" } : new String[] { "-", "INDIRECT" });
                    }
                    for (String[] o : outs) {
                        w.println(hx(f.getEntryPoint()) + "\t" + hx(ins.getAddress()) + "\t" + o[0] + "\t" + o[1] + "\t" + ins);
                        n++;
                        if (!o[0].equals("-")) {
                            sites.merge(o[0], 1, Integer::sum);
                            fns.computeIfAbsent(o[0], k -> new HashSet<>()).add(hx(f.getEntryPoint()));
                        }
                    }
                }
            }
        }
        List<String> keys = new ArrayList<>(sites.keySet());
        keys.sort((x, y) -> sites.get(y) - sites.get(x));
        try (PrintWriter w = new PrintWriter(new FileWriter(new File(root, "callers_count.tsv")))) {
            w.println("callee_fn\tname\tfan_in_sites\tfan_in_fns");
            for (String k : keys) {
                String name = k;
                try { Function g = fm.getFunctionAt(toAddr(Long.parseLong(k, 16))); if (g != null) name = g.getName(true); } catch (Exception e) { }
                w.println(k + "\t" + name + "\t" + sites.get(k) + "\t" + fns.get(k).size());
            }
        }
        println("ExportCallGraph: " + n + " edges, " + keys.size() + " callees");
    }
}

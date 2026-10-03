// Read-only: exports per-function fingerprints (fp.tsv) and defined strings (strings.tsv).
// Headless arg 0: output directory. Used by the Steam address-DB batch pipeline (DESIGN.md section 2).
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.StringDataInstance;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.Reference;
import java.io.*;
import java.util.*;

public class ExportFingerprints extends GhidraScript {
    private static final Set<String> STACK_REGS = new HashSet<>(Arrays.asList("ESP", "EBP", "SP", "BP", "RSP", "RBP"));

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args.length > 0 ? args[0] : ".");
        dir.mkdirs();
        Listing listing = currentProgram.getListing();
        Memory mem = currentProgram.getMemory();
        FunctionManager fm = currentProgram.getFunctionManager();

        int fnCount = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(new File(dir, "fp.tsv")))) {
            w.println("addr\tsize\tretn\tn_insn\tcallconv\tthunk_target\tcallees\tstr_refs\tdisp\timm\tname\tnamespace\tsource");
            for (Function fn : fm.getFunctions(true)) {
                monitor.checkCancelled();
                int retn = -1, nInsn = 0;
                TreeSet<Long> callees = new TreeSet<>(), strRefs = new TreeSet<>();
                List<Long> disp = new ArrayList<>(), imm = new ArrayList<>();
                for (Function c : fn.getCalledFunctions(monitor)) addEntry(callees, c.getEntryPoint());

                InstructionIterator it = listing.getInstructions(fn.getBody(), true);
                while (it.hasNext()) {
                    monitor.checkCancelled();
                    Instruction ins = it.next();
                    nInsn++;
                    if (ins.getMnemonicString().equals("RET")) {
                        int r = 0;
                        if (ins.getNumOperands() > 0) {
                            try { r = (int) ins.getScalar(0).getUnsignedValue(); } catch (Exception e) { r = 0; }
                        }
                        retn = Math.max(retn, r);
                    }
                    if (ins.getFlowType().isJump()) { // tail call: JMP to another function's entry
                        for (Address t : ins.getFlows()) {
                            Function tf = fm.getFunctionAt(t);
                            if (tf != null && !tf.getEntryPoint().equals(fn.getEntryPoint())) addEntry(callees, t);
                        }
                    }
                    for (Reference ref : ins.getReferencesFrom()) {
                        Address to = ref.getToAddress();
                        if (!to.isMemoryAddress()) continue;
                        Data d = listing.getDataContaining(to);
                        if (d == null) continue;
                        if (isString(d)) { strRefs.add(d.getMinAddress().getOffset()); continue; }
                        if (d.isPointer() && d.getValue() instanceof Address) { // one level of indirection
                            Data t = listing.getDataContaining((Address) d.getValue());
                            if (t != null && isString(t)) strRefs.add(t.getMinAddress().getOffset());
                        }
                    }
                    for (int i = 0; i < ins.getNumOperands(); i++) {
                        int type = ins.getOperandType(i);
                        Object[] objs = ins.getOpObjects(i);
                        if ((type & ghidra.program.model.lang.OperandType.DYNAMIC) != 0) collectDisp(objs, disp);
                        if ((type & ghidra.program.model.lang.OperandType.SCALAR) != 0) {
                            for (Object o : objs) {
                                if (!(o instanceof Scalar)) continue;
                                Scalar s = (Scalar) o;
                                long v = s.getUnsignedValue();
                                if (v == 0 || v == 1 || s.getSignedValue() == -1 || v == 0xffffffffL) continue;
                                if (resolvesToMemory(mem, v)) continue;
                                imm.add(v);
                            }
                        }
                    }
                }
                Collections.sort(disp);
                Collections.sort(imm);
                String thunk = "";
                if (fn.isThunk()) {
                    Function t = fn.getThunkedFunction(true);
                    if (t != null) thunk = hex(t.getEntryPoint().getOffset());
                }
                String ns = fn.getParentNamespace().isGlobal() ? "" : fn.getParentNamespace().getName();
                String cc = fn.getCallingConventionName() != null ? fn.getCallingConventionName() : "";
                w.println(hex(fn.getEntryPoint().getOffset()) + "\t" + fn.getBody().getNumAddresses() + "\t" + retn + "\t"
                    + nInsn + "\t" + cc + "\t" + thunk + "\t" + join(callees) + "\t" + join(strRefs) + "\t"
                    + join(disp) + "\t" + join(imm) + "\t" + esc(fn.getName()) + "\t" + esc(ns) + "\t" + fn.getSymbol().getSource());
                fnCount++;
            }
        }

        int strCount = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(new File(dir, "strings.tsv")))) {
            w.println("addr\ttext");
            DataIterator di = listing.getDefinedData(true);
            while (di.hasNext()) {
                monitor.checkCancelled();
                Data d = di.next();
                if (!(d.getValue() instanceof String)) continue;
                w.println(hex(d.getMinAddress().getOffset()) + "\t" + esc((String) d.getValue()));
                strCount++;
            }
        }
        println("ExportFingerprints: " + fnCount + " functions, " + strCount + " strings -> " + dir);
    }

    // [reg+disp] operand: needs a non-stack base register and a displacement 0 < disp <= 0x10000.
    private void collectDisp(Object[] objs, List<Long> out) {
        List<Register> regs = new ArrayList<>();
        List<Scalar> scalars = new ArrayList<>();
        for (Object o : objs) {
            if (o instanceof Register) regs.add((Register) o);
            else if (o instanceof Scalar) scalars.add((Scalar) o);
        }
        if (regs.isEmpty() || scalars.isEmpty()) return;
        if (regs.size() >= 2) scalars.remove(0);            // [base+index*scale+disp]: first scalar is the scale
        else if (scalars.size() >= 2) return;               // [index*scale+disp]: no base register
        if (scalars.isEmpty()) return;
        if (STACK_REGS.contains(regs.get(0).getName().toUpperCase())) return;
        long v = scalars.get(scalars.size() - 1).getUnsignedValue();
        if (v > 0 && v <= 0x10000) out.add(v);
    }

    private boolean resolvesToMemory(Memory mem, long v) {
        try { return mem.contains(toAddr(v)); } catch (Exception e) { return false; }
    }

    private boolean isString(Data d) {
        if (d.getValue() instanceof String) return true;
        String n = d.getDataType().getName().toLowerCase();
        return n.contains("string") || n.contains("unicode")
            || StringDataInstance.getStringDataInstance(d) != StringDataInstance.NULL_INSTANCE;
    }

    private void addEntry(Set<Long> set, Address a) {
        if (a.isMemoryAddress()) set.add(a.getOffset());
    }

    private String hex(long v) { return String.format("0x%08x", v); }

    private String join(Collection<Long> c) {
        StringBuilder sb = new StringBuilder();
        for (Long v : c) { if (sb.length() > 0) sb.append(','); sb.append(String.format("0x%x", v)); }
        return sb.toString();
    }

    private String esc(String s) {
        return s.replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n").replace("\r", "\\r");
    }
}

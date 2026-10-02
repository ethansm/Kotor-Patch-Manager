// Decompiles candidate functions found while porting K2AspyrMapAspectFix
// from GOG to Steam Aspyr (via string-keyword fallback search after the
// exact GOG byte patterns missed). Also dumps full disassembly for each,
// and searches the symbol table for any Map-related class/vtable symbols
// (in case RTTI analysis already produced named classes).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Map Candidates

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;

public class DecompileMapCandidates extends GhidraScript {

    private static final String[] TARGET_ENTRIES = {
        "00747210",  // LBL_MAP / LBL_MAPVIEW / LBL_MAPBORDER / BTN_MINIMAP refs
        "00523870",  // MapResX / MapZoom / MapPt1X/Y / MapPt2X/Y refs (projection params)
        "007b88b0",  // "Mini Map" string ref
        "007b5f50",  // "Mini Map" string ref
        "0052b270",  // AreaMapData / AreaMapDataSize / AreaMapResX/Y refs
        "005285b0",  // AreaMapData / AreaMapDataSize / AreaMapResX/Y refs
        "00893950"   // LBL_Map (different casing) ref
    };

    @Override
    public void run() throws Exception {
        println("=== Symbol table search for Map-related class/vtable symbols ===");
        SymbolTable st = currentProgram.getSymbolTable();
        SymbolIterator it = st.getAllSymbols(true);
        int count = 0;
        while (it.hasNext()) {
            Symbol s = it.next();
            String n = s.getName();
            if (n.contains("Map") && (n.contains("vtable") || n.contains("vftable")
                    || n.contains("RTTI") || n.contains("::") || n.startsWith("PTR_"))) {
                println("  " + s.getSymbolType() + " " + n + " @ " + s.getAddress());
                count++;
                if (count > 200) { println("  ... truncated"); break; }
            }
        }
        println("Total: " + count);

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGET_ENTRIES) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) {
                    println("  (no function defined at this exact address)");
                    fn = getFunctionContaining(addr);
                    if (fn != null) {
                        println("  containing function: " + fn.getName() + " @ " + fn.getEntryPoint());
                    } else {
                        continue;
                    }
                }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println("  -- Decompiled C --");
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed: " + (res != null ? res.getErrorMessage() : "null") + ")");
                }

                println("  -- Full disassembly --");
                Listing listing = currentProgram.getListing();
                InstructionIterator insnIt = listing.getInstructions(fn.getBody(), true);
                while (insnIt.hasNext()) {
                    Instruction insn = insnIt.next();
                    byte[] bytes = insn.getBytes();
                    StringBuilder hexStr = new StringBuilder();
                    for (byte b : bytes) hexStr.append(String.format("%02x ", b & 0xff));
                    println("    " + insn.getAddress() + "  " + String.format("%-30s", hexStr.toString()) + insn.toString());
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}

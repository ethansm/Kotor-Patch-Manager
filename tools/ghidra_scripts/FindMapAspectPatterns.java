// Searches the Steam Aspyr swkotor2.exe for the GOG K2AspyrMapAspectFix
// hook byte patterns (3 detour hooks), to find their Steam-build equivalent
// addresses for porting. For every hit, dumps the containing function,
// disassembly context, and decompiled C so the match can be judged by hand.
// Falls back to a Map/MiniMap-keyword string + RTTI-class-name search when a
// byte pattern is ambiguous or misses.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Map Aspect Patterns

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.DataType;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.symbol.Reference;

import java.util.regex.Pattern;

public class FindMapAspectPatterns extends GhidraScript {

    // GOG 0x00564DDD, 12 bytes: mov [ebp-0x260], eax ; mov eax, [ebp-0x260]
    private static final String PATTERN_PROJECTION =
        "89 85 a0 fd ff ff 8b 85 a0 fd ff ff";

    // GOG 0x005633A5, 10 bytes: mov dword ptr [ebp-0x84], 0
    private static final String PATTERN_AREA_MAP =
        "c7 85 7c ff ff ff 00 00 00 00";

    // GOG 0x0052202A, 6 bytes: mov [ebp-0x74], eax ; mov eax, [ebp-0x74]
    private static final String PATTERN_MINIMAP =
        "89 45 8c 8b 45 8c";

    private static final String[] STRING_KEYWORDS = {
        "Map", "MAP", "AreaMap", "MiniMap", "MINIMAP", "MiniMap"
    };

    private static final Pattern RTTI_MAP_CLASS = Pattern.compile(".?AV.*Map.*@@");

    private DecompInterface decomp;

    @Override
    public void run() throws Exception {
        decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            Address start = currentProgram.getMinAddress();

            println("################################################");
            println("=== [1] preserveAreaMapProjection pattern (12 bytes, most distinctive) ===");
            report(findBytes(start, PATTERN_PROJECTION, 50), 12);

            println("################################################");
            println("=== [2] preserveAreaMapAspect pattern (10 bytes) ===");
            report(findBytes(start, PATTERN_AREA_MAP, 50), 10);

            println("################################################");
            println("=== [3] preserveMiniMapAspect pattern (6 bytes, least distinctive) ===");
            report(findBytes(start, PATTERN_MINIMAP, 50), 6);

            println("################################################");
            println("=== [4] String keyword fallback search (Map/MiniMap) ===");
            stringSearch();
        } finally {
            decomp.dispose();
        }
    }

    private void report(Address[] hits, int patternLen) throws Exception {
        println("Found " + hits.length + " hit(s).");
        for (Address hit : hits) {
            println("---- HIT at " + hit + " ----");
            Function fn = getFunctionContaining(hit);
            if (fn == null) {
                println("  (not inside any defined function)");
            } else {
                println("  Containing function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " (size " + fn.getBody().getNumAddresses() + " bytes)");
            }

            println("  -- Disassembly context --");
            Listing listing = currentProgram.getListing();
            Address ctxStart = hit.subtract(16);
            if (ctxStart == null || ctxStart.getOffset() < 0) {
                ctxStart = hit;
            }
            InstructionIterator it = listing.getInstructions(ctxStart, true);
            Address end = hit.add(patternLen + 16);
            while (it.hasNext()) {
                Instruction insn = it.next();
                if (insn.getAddress().compareTo(end) > 0) break;
                String marker = (insn.getAddress().compareTo(hit) >= 0
                    && insn.getAddress().compareTo(hit.add(patternLen - 1)) <= 0) ? " <== IN PATTERN" : "";
                println("    " + insn.getAddress() + ": " + insn.toString() + marker);
            }

            if (fn != null) {
                println("  -- Decompiled C --");
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompilation failed: " + (res != null ? res.getErrorMessage() : "null result") + ")");
                }
            }
            println("");
        }
    }

    private void stringSearch() throws Exception {
        DataIterator it = currentProgram.getListing().getDefinedData(currentProgram.getMinAddress(), true);
        int matched = 0;
        while (it.hasNext()) {
            Data d = it.next();
            DataType dt = d.getDataType();
            String tn = dt.getName().toLowerCase();
            if (!tn.contains("string") && !tn.contains("char")) continue;
            Object val = d.getValue();
            if (val == null) continue;
            String s = val.toString();
            if (s.length() < 3 || s.length() > 100) continue;

            boolean hit = false;
            for (String kw : STRING_KEYWORDS) {
                if (s.contains(kw)) { hit = true; break; }
            }
            boolean rttiHit = RTTI_MAP_CLASS.matcher(s).find();
            if (!hit && !rttiHit) continue;

            matched++;
            println("STRING @ " + d.getAddress() + " : \"" + s + "\"" + (rttiHit ? " [RTTI-CLASS-CANDIDATE]" : ""));
            Reference[] refs = getReferencesTo(d.getAddress());
            int refCount = 0;
            for (Reference r : refs) {
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                println("    ref from " + from + (fn != null ? " in " + fn.getName() + " @ " + fn.getEntryPoint() : " (no function)"));
                refCount++;
                if (refCount > 15) { println("    ... (more refs truncated)"); break; }
            }
            if (matched > 300) { println("... (too many string matches, truncating)"); break; }
        }
        println("Total matching strings: " + matched);
    }
}

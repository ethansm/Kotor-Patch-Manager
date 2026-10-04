// @category KOTOR
// Read-only: writes every initialized memory block's raw bytes to <out>/<block>.bin and a
// map (blocks.tsv: name, start VA, size, rwx) so the program can be disassembled offline
// with objdump/capstone. Arg 0 = output directory.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.mem.*;
import java.io.*;
public class ExportMemoryBlocks extends GhidraScript {
    public void run() throws Exception {
        File out = new File(getScriptArgs()[0]);
        out.mkdirs();
        try (PrintWriter map = new PrintWriter(new File(out, "blocks.tsv"))) {
            map.println("name\tstart\tsize\tperms\tfile");
            for (MemoryBlock b : currentProgram.getMemory().getBlocks()) {
                if (!b.isInitialized()) continue;
                byte[] buf = new byte[(int) b.getSize()];
                b.getBytes(b.getStart(), buf);
                String f = b.getName().replaceAll("[^A-Za-z0-9_.-]", "_") + ".bin";
                try (FileOutputStream fo = new FileOutputStream(new File(out, f))) { fo.write(buf); }
                String perms = (b.isRead() ? "r" : "-") + (b.isWrite() ? "w" : "-") + (b.isExecute() ? "x" : "-");
                map.println(b.getName() + "\t0x" + b.getStart().toString() + "\t" + b.getSize() + "\t" + perms + "\t" + f);
            }
        }
        println("ExportMemoryBlocks: done -> " + out);
    }
}

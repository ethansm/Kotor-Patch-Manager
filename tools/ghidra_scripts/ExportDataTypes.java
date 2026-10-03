// Read-only: exports struct/union/enum/typedef definitions from the program's data type manager.
// Headless arg 0: output directory (writes types.tsv). See steam_db_batches/DESIGN.md section 2.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import java.io.*;
import java.util.Iterator;

public class ExportDataTypes extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args.length > 0 ? args[0] : ".");
        dir.mkdirs();
        int types = 0, rows = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(new File(dir, "types.tsv")))) {
            w.println("category\ttype_name\tkind\tsize\tordinal\tfield_offset\tfield_name\tfield_type");
            Iterator<DataType> it = currentProgram.getDataTypeManager().getAllDataTypes();
            while (it.hasNext()) {
                monitor.checkCancelled();
                DataType dt = it.next();
                String kind;
                if (dt instanceof Structure) kind = "struct";
                else if (dt instanceof Union) kind = "union";
                else if (dt instanceof ghidra.program.model.data.Enum) kind = "enum";
                else if (dt instanceof TypeDef) kind = "typedef";
                else continue; // pointers, arrays, built-ins
                String head = esc(dt.getCategoryPath().getPath()) + "\t" + esc(dt.getName()) + "\t" + kind + "\t" + dt.getLength() + "\t";
                types++;
                if (dt instanceof Composite) {
                    DataTypeComponent[] comps = ((Composite) dt).getComponents();
                    if (comps.length == 0) { w.println(head + "\t\t\t"); rows++; }
                    for (DataTypeComponent c : comps) {
                        monitor.checkCancelled();
                        String name = c.getFieldName() == null ? "" : c.getFieldName();
                        w.println(head + c.getOrdinal() + "\t" + c.getOffset() + "\t" + esc(name) + "\t" + esc(c.getDataType().getDisplayName()));
                        rows++;
                    }
                } else if (dt instanceof ghidra.program.model.data.Enum) {
                    ghidra.program.model.data.Enum e = (ghidra.program.model.data.Enum) dt;
                    String[] names = e.getNames();
                    if (names.length == 0) { w.println(head + "\t\t\t"); rows++; }
                    for (int i = 0; i < names.length; i++) {
                        monitor.checkCancelled();
                        w.println(head + i + "\t" + e.getValue(names[i]) + "\t" + esc(names[i]) + "\t");
                        rows++;
                    }
                } else {
                    w.println(head + "\t\t\t" + esc(((TypeDef) dt).getBaseDataType().getDisplayName()));
                    rows++;
                }
            }
        }
        println("ExportDataTypes: " + types + " types, " + rows + " rows -> " + new File(dir, "types.tsv"));
    }

    private String esc(String s) {
        return s.replace("\t", "\\t").replace("\n", "\\n").replace("\r", "\\r");
    }
}

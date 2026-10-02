// @category KOTOR
// args: addrHex:lenHex ...  -- hex dump bytes (and 4-byte LE dwords) at each address
import ghidra.app.script.GhidraScript;
public class HexDump extends GhidraScript {
    public void run() throws Exception {
        for (String s : getScriptArgs()) {
            String[] p = s.split(":"); long a = Long.parseLong(p[0], 16); int n = Integer.parseInt(p[1], 16);
            for (int i = 0; i < n; i += 16) {
                StringBuilder sb = new StringBuilder(String.format("%08x:", a + i));
                for (int j = 0; j < 16 && i + j < n; j++) { try { sb.append(String.format(" %02x", getByte(toAddr(a + i + j)) & 0xff)); } catch (Exception e) { sb.append(" ??"); } }
                println(sb.toString());
            }
        }
    }
}

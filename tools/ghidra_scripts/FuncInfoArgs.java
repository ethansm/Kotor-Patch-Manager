// FuncInfoArgs.java — @category KOTOR. For each hex addr: containing fn, entry?, first 16 bytes.
import ghidra.app.script.GhidraScript; import ghidra.program.model.listing.*; import ghidra.program.model.address.*;
public class FuncInfoArgs extends GhidraScript { public void run() throws Exception {
  for (String s : getScriptArgs()) { Address a = toAddr(Long.parseLong(s,16)); Function f = getFunctionContaining(a);
    byte[] b = new byte[16]; currentProgram.getMemory().getBytes(a,b); StringBuilder h=new StringBuilder(); for(byte x:b) h.append(String.format("%02x ",x&0xff));
    println("ADDR "+s+" fn="+(f==null?"NONE":f.getName()+"@"+f.getEntryPoint()+" sz="+f.getBody().getNumAddresses()+(f.getEntryPoint().equals(a)?" [ENTRY]":" [inside]"))+" bytes="+h); } } }

// FindBytesCtx.java — @category KOTOR. args: hexbytes(no spaces) [maxHits]. Exact find(Address,byte[]) loop; prints fn + offset from entry.
import ghidra.app.script.GhidraScript; import ghidra.program.model.listing.*; import ghidra.program.model.address.*;
public class FindBytesCtx extends GhidraScript { public void run() throws Exception {
  String hx=getScriptArgs()[0]; int max=getScriptArgs().length>1?Integer.parseInt(getScriptArgs()[1]):200;
  byte[] pat=new byte[hx.length()/2]; for(int i=0;i<pat.length;i++) pat[i]=(byte)Integer.parseInt(hx.substring(2*i,2*i+2),16);
  Address start=currentProgram.getMinAddress(); int n=0;
  while(n<max){ Address h=find(start,pat); if(h==null)break; n++; Function f=getFunctionContaining(h);
    println("HIT "+h+" fn="+(f==null?"NONE":f.getName()+"@"+f.getEntryPoint()+" +0x"+Long.toHexString(h.subtract(f.getEntryPoint()))+" sz="+f.getBody().getNumAddresses())); start=h.add(1);}
  println("TOTAL "+n); } }

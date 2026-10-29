# Name check cards (blind)

Each card gives static facts about one address in swkotor2.exe and ONE candidate name for it.
Judge only from the facts on the card. Do not read any other file.

- consistent: the facts fit the name (what it calls, reads, compares, strings, how it is used).
- inconsistent: the facts contradict the name (e.g. "Vtable" but the address is a float divisor,
  "Iat" but it is never CALLed through, a GUI name on an audio routine).
- unsure: the facts say too little either way.

Naming noise to ignore: prefixes Addr/k/G_/g_, suffixes Global/Val/Flag. Iat = import address table
slot (CALL dword ptr [addr]); Vtable/Vtbl = vtable pointer stored into an object (MOV [reg],addr).

Write one TSV line per card: `card<TAB>verdict<TAB>quote` where quote is the one fact line
(copied verbatim, may be shortened) that decided it. Header line: `card	verdict	quote`.

## C01
kind: function
address: 0x00919671
candidate name: `Global::ExecuteCommandGetGalaxyMapPlanetAvailable`
facts:
- size 125 B, RET 0
- callers 90: 0x004010a0, 0x004073b0, 0x00407540, 0x004077a0, 0x004077d0, ...
- callees: 0x00923904 (CALL), 0x0091a0d2 (CALL), 0x00922d2a (CALL), 0x00922b20 (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    _sprintf
    Library: Visual Studio 2008 Release */
    int __cdecl _sprintf(char *_Dest,char *_Format,...)
    if ((_Format == (char *)0x0) || (_Dest == (char *)0x0)) {
    piVar1 = __errno();
    __invalid_parameter(0,0,0,0,0);
    iVar2 = __output_l(&local_24,_Format,(_locale_t)0x0,&stack0x0000000c);
    if (local_24._cnt < 0) {
    __flsbuf(0,&local_24);

## C02
kind: function
address: 0x0091c6b5
candidate name: `Global::_free`
facts:
- size 133 B, RET 0
- callers 44: 0x00406350, 0x0073a3a0, 0x0073a8a0, 0x0073aa30, 0x009196fd operator delete, ...
- callees: 0x00924dcc (CALL), 0x00926d37 (CALL), 0x00927110 (CALL), 0x00927140 (CALL), 0x0091c70b (CALL), 0x000000f8 (CALL), ...
- globals: 0x00a806c8 READ, 0x00a80dd8 READ
- imports: GetLastError (KERNEL32.DLL), HeapFree (KERNEL32.DLL)
- twins: none
- decomp: /* WARNING: Function: __SEH_prolog4 replaced with injection: SEH_prolog4 */
    void __cdecl _free(void *_Memory)
    if (_Memory != (void *)0x0) {
    if (DAT_00a80dd8 == 3) {
    __lock(4);
    iVar1 = ___sbh_find_block(_Memory);
    if (iVar1 != 0) {
    ___sbh_free_block(iVar1,_Memory);
    FUN_0091c70b();
    if (iVar1 != 0) {

## C03
kind: function
address: 0x0091c630
candidate name: `Global::_memset`
facts:
- size 122 B, RET 0
- callers 136: 0x00405080, 0x004073b0, 0x00407920, 0x004087a0, 0x00425db0 CAurTextureBasic::GetMemoryUsage, ...
- callees: 0x00926f92 (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    void * __cdecl _memset(void *_Dst,int _Val,size_t _Size)
    if (_Size == 0) {
    if ((((char)_Val == '\0') && (0xff < _Size)) && (DAT_00a80ddc != 0)) {
    pvVar2 = (void *)__VEC_memzero();
    if (3 < _Size) {
    uVar3 = -(int)_Dst & 3;
    if (uVar3 != 0) {
    *(char *)puVar5 = (char)_Val;
    puVar5 = (uint *)((int)puVar5 + 1);

## C04
kind: function
address: 0x0091c630
candidate name: `Global::ExecuteCommandSetLockOrientationInDialog`
facts:
- size 122 B, RET 0
- callers 136: 0x00405080, 0x004073b0, 0x00407920, 0x004087a0, 0x00425db0 CAurTextureBasic::GetMemoryUsage, ...
- callees: 0x00926f92 (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    void * __cdecl _memset(void *_Dst,int _Val,size_t _Size)
    if (_Size == 0) {
    if ((((char)_Val == '\0') && (0xff < _Size)) && (DAT_00a80ddc != 0)) {
    pvVar2 = (void *)__VEC_memzero();
    if (3 < _Size) {
    uVar3 = -(int)_Dst & 3;
    if (uVar3 != 0) {
    *(char *)puVar5 = (char)_Val;
    puVar5 = (uint *)((int)puVar5 + 1);

## C05
kind: function
address: 0x00919671
candidate name: `Global::_sprintf`
facts:
- size 125 B, RET 0
- callers 90: 0x004010a0, 0x004073b0, 0x00407540, 0x004077a0, 0x004077d0, ...
- callees: 0x00923904 (CALL), 0x0091a0d2 (CALL), 0x00922d2a (CALL), 0x00922b20 (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    _sprintf
    Library: Visual Studio 2008 Release */
    int __cdecl _sprintf(char *_Dest,char *_Format,...)
    if ((_Format == (char *)0x0) || (_Dest == (char *)0x0)) {
    piVar1 = __errno();
    __invalid_parameter(0,0,0,0,0);
    iVar2 = __output_l(&local_24,_Format,(_locale_t)0x0,&stack0x0000000c);
    if (local_24._cnt < 0) {
    __flsbuf(0,&local_24);

## C06
kind: function
address: 0x0091c6b5
candidate name: `Global::ExecuteCommandMoveAwayFromObject`
facts:
- size 133 B, RET 0
- callers 44: 0x00406350, 0x0073a3a0, 0x0073a8a0, 0x0073aa30, 0x009196fd operator delete, ...
- callees: 0x00924dcc (CALL), 0x00926d37 (CALL), 0x00927110 (CALL), 0x00927140 (CALL), 0x0091c70b (CALL), 0x000000f8 (CALL), ...
- globals: 0x00a806c8 READ, 0x00a80dd8 READ
- imports: GetLastError (KERNEL32.DLL), HeapFree (KERNEL32.DLL)
- twins: none
- decomp: /* WARNING: Function: __SEH_prolog4 replaced with injection: SEH_prolog4 */
    void __cdecl _free(void *_Memory)
    if (_Memory != (void *)0x0) {
    if (DAT_00a80dd8 == 3) {
    __lock(4);
    iVar1 = ___sbh_find_block(_Memory);
    if (iVar1 != 0) {
    ___sbh_free_block(iVar1,_Memory);
    FUN_0091c70b();
    if (iVar1 != 0) {

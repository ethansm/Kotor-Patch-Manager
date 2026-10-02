#!/bin/bash
# Phase 1 gate while g++-multilib is absent: MinGW DLL must build; every x86_64
# test row must report "all pass"; i386 rows may only fail on the missing
# multilib headers (bits/c++config.h / bits/wordsize.h). With multilib present,
# requires full `make all test` green.
W=~/Games/KOTOR2_Mod_Workspace/tools/kpm-registry/src/KotorPatcher
S=${TMPDIR:-/tmp}/kpm_registry_gate; mkdir -p $S
out=$S/gate_$(date +%H%M%S)
make -C $W dll > $out.dll.log 2>&1 || { echo "GATE FAIL: dll build"; tail -20 $out.dll.log; exit 1; }
make -C $W test > $out.test.log 2>&1; trc=$?
if [ -d /usr/lib/gcc/x86_64-linux-gnu/13/32 ]; then
  make -C $W so > $out.so.log 2>&1 || { echo "GATE FAIL: so build"; exit 1; }
  [ $trc = 0 ] || { echo "GATE FAIL: make test rc=$trc"; grep -nE 'FAIL|error' $out.test.log | head; exit 1; }
  echo "GATE PASS (full, multilib)"; exit 0
fi
# Per-row verdicts for x86_64
awk '/^  \[x86_64\]/{if(row)print row, st; row=$2; st="NORESULT"} /^  all pass/{st="PASS"} /FAILURES:/{st="FAIL"} END{if(row)print row, st}' $out.test.log > $out.rows
bad64=$(grep -v ' PASS$' $out.rows)
errs=$(grep -E 'error' $out.test.log | grep -vE 'fatal error: (bits|gnu)/[a-z0-9_+-]+\.h: No such file|^make|Error 1' )
fails=$(grep -c 'FAILURES' $out.test.log)
n64=$(wc -l < $out.rows)
if [ -n "$bad64" ] || [ -n "$errs" ] || [ "$fails" != 0 ]; then
  echo "GATE FAIL"; echo "$bad64"; echo "$errs" | head -20; grep -B3 -A0 'FAIL' $out.test.log | head -40; exit 1
fi
echo "GATE PASS (dll + $n64 x86_64 rows; i386 skipped: no multilib) log=$out.test.log"
cat $out.rows

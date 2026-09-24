#!/usr/bin/env bash

# Test suite runner for tinyDLX, described in docs/internals/Testing.md.
# Usage: tools/run_tests.sh [--update]

set -u

cd "$(dirname "${BASH_SOURCE[0]}")/.." || exit 3

# All test cases are quadruple:
# - a name
# - the program source file
# - the exit code it must produce
# - a cycle cap
# For each case run will be generated the final state dump, which will be diffed
# against the frozen 'golden' snapshot found as tests/expected/<name>.json.
# # name                source                               exit  cap
CASES=(
  # Programs that run to a halt
  "full_isa           tests/full_isa.asm                      0  1000"
  "startup_circuit    tests/startup_circuit.asm               0  1000"
  "mov_special        tests/mov_special.asm                   0  1000"
  "interrupt          tests/interrupt.asm                     0  1000"
  # Programs that must fault
  "unaligned          tests/faults/unaligned.asm              1   100"
  "rom_write          tests/faults/rom_write.asm              1   100"
  "unmapped           tests/faults/unmapped.asm               1   100"
  "unknown_int        tests/faults/unknown_int.asm            1   100"
  "int_while_disabled tests/faults/int_while_disabled.asm     1   100"
  "unknown_debug_int  tests/faults/unknown_debug_int.asm      1   100"
  # Reserved encodings the assembler cannot emit, so the source is the 32 bit
  # word itself and the runner writes it out big endian
  "unknown_opcode     0x7C000000                              1   100"
  "unknown_func       0x0000003F                              1   100"
  "reserved_spr       0x00001830                              1   100"
)

case "${1-}" in
  "")          update=0 ;;
  -u|--update) update=1 ;;
  *)           echo "Usage: tools/run_tests.sh [--update]" >&2; exit 3 ;;
esac

[ -x bin/cpu_seq ] || { echo "bin/cpu_seq not found: run make first" >&2; exit 3; }

# Creare tmp dir and delete it on exit
tmp=$(mktemp -d) || exit 3
trap 'rm -rf "$tmp"' EXIT

failed=0

for row in "${CASES[@]}"; do
  read -r name source expected cap <<< "$row"
  golden=tests/expected/$name.json
  got=$tmp/$name.json
  err=$tmp/$name.err
  input=/dev/null
  [ -f "${source%.asm}.in" ] && input="${source%.asm}.in"
  # The test program binary will be in "$tmp/$name.bin"

  if [ "${source#0x}" = "$source" ]; then
    # If source is a .asm file assemble
    if ! python3 tools/asm.py "$source" "$tmp/$name.bin" > "$err" 2>&1; then
      echo "FAIL  $name: assembly failed"
      sed 's/^/      /' "$err"
      failed=1
      continue
    fi
  else
    # If source starts with 0x it is a binary file that neewds to be written directly
    # Four big endian bytes
    printf '%b' "$(printf '\\x%02X\\x%02X\\x%02X\\x%02X' \
      $(( (source >> 24) & 0xFF )) $(( (source >> 16) & 0xFF )) \
      $(( (source >>  8) & 0xFF )) $((  source        & 0xFF )))" > "$tmp/$name.bin"
  fi

  # Run the emulator with:
  #  - initial GPRs values set to 0
  #  - strict mode (warnings are faults)
  #  - print final state dump to $got (which will be diffed against the golden json file)
  #  - passing input (/dev/null if not defined as $name.in) and redirecting emulator info
  #    output to /dev/null
  bin/cpu_seq -b "$tmp/$name.bin" -G 0 -s -C "$cap" -d "$got" \
              < "$input" > /dev/null 2> "$err"
  code=$?

  if [ "$code" -ne "$expected" ]; then
    # If status code differs the test fails
    echo "FAIL  $name: exit $code, expected $expected"
    grep -E '\[(ERROR|WARN)\]' "$err" | sed 's/^/      /'
    failed=1
  elif [ "$update" -eq 1 ]; then
    # If the emulation did not fail and the flag --update is setted, save the got dump as
    # the dolgen one for future tests
    cp "$got" "$golden"
    echo "ok    $name: golden updated"
  elif [ ! -f "$golden" ]; then
    # If the emulation succeded but the goelden output does not exists
    echo "FAIL  $name: no $golden yet, run with --update"
    failed=1
  elif ! diff -u "$golden" "$got" > "$tmp/$name.diff"; then
    # If diff fails the test has failed
    echo "FAIL  $name: snapshot differs from $golden"
    # The diff infos get printed
    sed -n '3,$p' "$tmp/$name.diff" | grep -E '^[-+]' | sed 's/^/      /'
    failed=1
  else
    echo "ok    $name"
  fi
done

[ "$failed" -eq 0 ] || exit 1

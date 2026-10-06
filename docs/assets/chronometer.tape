# Run from root with: vhs docs/assets/chronometer.tape
Output docs/assets/chronometer.gif

Set FontSize 18
Set Padding 20
Set Width 1000
Set Height 450

# Build outside the recording
Hide
Type "make && clear"
Enter
Sleep 3s
Show

Type "python tools/asm.py examples/chronometer.asm chronometer.bin"
Enter
Sleep 1s
# Use a x50 factor on the run frequency to speed up the chronometer
# which expets to run at 1000 Hz
Type "./bin/cpu_seq -b chronometer.bin -f 500000 -s"
Enter
Sleep 7s
Type "q"
Sleep 1s

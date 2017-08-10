# AxisFix

This project tries to fix the axis problem found when using some PlayStation to USB converters to play games that use DDR (Dance Dance Revolution) mats. There is already a solution for Windows using AutoHotKey, but there is apparently none for Linux, and so I developed this code.

## The Problem

Some PlayStation Controller ports to USB port converters have a problem where analog axis are reported when pressing d-pad buttons, and so when playing DDR-like games (StepMania, for instance) you come to the problem where you cant press two opposing directions at the same time, since an analog stick cant be facing to opposite directions at the same time.

## Available fixes

On Windows, there is an AutoHotKey based solution which reads the axis position and based on small differences on the axis value, simulates keyboard input. This solution does not work correctly on Linux because AHK does not exist for Linux.

There is a solution somewhere online for Linux which involves patchin the joystick kernel module (iirc) but doing that isn't really that intuitive.

## This tool

I wrote this tool to solve the problem presented above. It uses the same idea the AHK one uses, watching the joystick's input and simulating keyboard key presses as the 'correct' input to StepMania.

There are 2 modes, `config` and `fix`. The config mode serves as a Wizard of sorts to create the .ini configuration file the tool accepts.
`fix` mode takes a configuration file and starts running the algorithm that takes pad input and converts it to key strokes.

## Compiling

To compile this program, you can just run `make` on the root folder. If you want it to produce debug messages, then run `make debug` inside the `src/` directory.

### Requirements

I think all you need is to have a recent kernel, since the joystick and uinput modules aren't all that new (to my knowledge).

This tool depends on no dynamic libraries.

## License

Unless stated otherwise in the file (such as in ini.*), the code in this repository is released under an MIT-like license. Check LICENSE for more information.

This tool uses the INIH library, available at https://github.com/benhoyt/inih all credit for ini.h and ini.c should go to their creator and not me.
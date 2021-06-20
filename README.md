# GameBoy Emulator
Second Year Processing Project @ EPFL ([CS-212 course](https://edu.epfl.ch/coursebook/en/system-programming-project-CS-212)): GameBoy Emulator

by Szabina Horváth-Mikulas and Raoul Gerber

There are some issues with the screen display, making this version of the emulator not functional in its current state. Over 95% of the code works as intended though and a tiny bit of polishing could render it functional.

The main file (called gbsimulator) can be compiled by simply typing make in the terminal. It must be noted however, that before it can be done so, one must launch the command "export LD_LIBRARY_PATH=." once per (re-)opened terminal. To then execute the file, one must indicate the ROM file to launch in a first argument, as an example: "./gbsimulator game.gb"

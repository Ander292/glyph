# A command line text editor

A simple command line text editor made for windows. I decided to refrain from using most standard c libraries for file I/O. I used only windows, conio and stdint libraries. This program was written to be used for the new win11 windows terminal host and will not work (at least not properly) on the older conhost.exe that is the default for win10 and earlier.

### How to use:

It is ran from the command line and requires a path to a file as argument. If the file already exists it will open it and if it doesnt it will create it. It doesnt support being called without a file path at startup. Ctrl+q is used to quit and Ctrl+s is used to save. All other shortcuts are intuitive and similar to most other text editors.

### How to build:

I did not use any sort of cmake or some other build program. I made a powershell script for it. Passing the "build" argument will build the file. The settings.json file is there for the [vscode actions buttons](https://marketplace.visualstudio.com/items?itemName=seunlanlege.action-buttons) plugin

### Whats broken:

1) Ctrl + arrowKeys don't work properly.
2) Sometimes it glitches on the end of the line for some reason.
3) The header thing is broken as well. The exit message is not displayed properly sometimes (for too small width)

### But why???

No idea. This started as an experiment about strings and string functions and ended up becoming a text editor.

### I am not responsible for any mental or emotional damage caused by looking at this code or by using the program itself. You have been warned!
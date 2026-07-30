# A command line text editor (v2)

A simple command line text editor made for windows/linux. libraries. Initially I made this for windows only but it is now cross platform. The editor fully supports UTF-8.

### How to use:

It is ran from the command line and requires a path to a file as argument. If the file already exists it will open it and if it doesnt it will create it. It doesnt support being called without a file path at startup. Ctrl+q is used to quit and Ctrl+s is used to save. All other shortcuts are intuitive and similar to most other text editors.

### How to build:

I did not use any sort of cmake or some other build program. I made a powershell script for it. Passing the "build" argument will build the file. The settings.json file is there for the [vscode actions buttons](https://marketplace.visualstudio.com/items?itemName=seunlanlege.action-buttons) plugin

For linux use the build.sh script with build argument

I compiled this with gcc for linux and mingw gcc for windows. If you use another compiler then you will need to compile them without the script.

### But why???

No idea. This started as an experiment about strings and string functions and ended up becoming a text editor. Its essentially just a glorified string and file I/O practice.

### I am not responsible for any mental or emotional damage caused by looking at this code or by using the program itself. You have been warned!

@echo off
cls
g++ -Wall fans.cpp main.cpp -lole32 -static-libgcc -static-libstdc++ -o AlienwareControls.exe
g++ -Wall fans.cpp main.cpp -lole32 -static-libgcc -static-libstdc++ -o AlienwareControlsW.exe -Wl,--subsystem,windows

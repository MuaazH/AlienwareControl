@echo off
md obj
cls
g++ -c -g -Wall src/TestFans.cpp -o obj/TestFans.obj
g++ -Wall obj/TestFans.obj obj/fans.obj -o TestFans.exe -lole32
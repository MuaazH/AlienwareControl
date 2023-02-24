@echo off
md obj
cls
g++ -c -Wall Test.cpp -o obj/test.obj
g++ -Wall obj/test.obj obj/lights.obj -o SetLights.exe -Llib -lsetupapi -lhid
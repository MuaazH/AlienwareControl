@echo off
md obj
cls
g++ -c -Wall src/Test.cpp -o obj/test.obj
g++ -Wall obj/test.obj obj/lights.obj -o SetLights.exe -Ldll -lsetupapi -lhid
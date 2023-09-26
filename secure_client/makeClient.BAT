@echo off
rem Compile secure_client.cpp to create secure_client.exe

rem Compile secure_client.cpp
g++ -Wall -O2 secure_client.cpp -lws2_32 -o secure_client.exe

rem Check if compilation was successful
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b %errorlevel%
)

rem Compilation successful
echo Compilation successful. secure_client.exe is ready.
pause
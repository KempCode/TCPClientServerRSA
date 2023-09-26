@echo off
rem Compile secure_server.cpp to create secure_server.exe

rem Compile secure_server.cpp
g++ secure_server.cpp -lws2_32 -o secure_server.exe

rem Check if compilation was successful
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b %errorlevel%
)

rem Compilation successful
echo Compilation successful. secure_server.exe is ready.
pause
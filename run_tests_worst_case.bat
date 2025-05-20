@echo off
rem ************************************************************
rem *  run_tests.bat
rem *
rem *  Usage: run_tests.bat -n 1000 -src-size 0.3 -dst-size 0.3 -ntests 10
rem *
rem *  This script compiles two C++ files and then executes:
rem *    .\bin\matrix.exe <identifier> -n <n> --src-size <src-size> --dst-size <dst-size> -u
rem *  followed by
rem *    .\bin\main.exe <identifier> --test-number=<i>
rem *  for each i from 0 to ntests.
rem ************************************************************

rem --- Parse command-line arguments ---
set "n="
set "ntests="
set "algorithms="

:parse
if "%~1"=="" goto after_parse

if "%~1"=="-n" (
    set "n=%2"
    shift
    shift
    goto parse
)
if "%~1"=="-ntests" (
    set "ntests=%2"
    shift
    shift
    goto parse
)

if "%~1"=="-algorithms" (
    set "algorithms=%2"
    shift
    shift
    goto parse
)

rem If unknown parameter, skip it.
shift
goto parse

:after_parse

rem --- Validate parameters ---
if "%n%"=="" (
    echo Error: Missing parameter -n.
    exit /b 1
)
if "%ntests%"=="" (
    echo Error: Missing parameter -ntests.
    exit /b 1
)

if "%algorithms%"=="" (
    echo Error: Missing parameter -algorithms.
    exit /b 1
)

echo Using parameters:
echo    n       = %n%
echo    ntests  = %ntests%
echo    algorithms = %algorithms%

rem --- Compile the C++ files ---
echo Compiling main...
g++ -fopenmp -O2 -std=c++17 -I include/ .\src\main.cc -o bin/main.exe
if errorlevel 1 (
    echo Compilation of main failed.
    exit /b 1
)

@REM echo Compiling getMatrixEntropy...
@REM g++ -std=c++17 -I include/ .\src\getMatrixEntropy.cc -o bin/entropy.exe
@REM if errorlevel 1 (
@REM     echo Compilation of getMatrixEntropy failed.
@REM     exit /b 1
@REM )

rem --- Build identifier string ---
set "identifier=%n%-worst-case"

if exist "output\%identifier%" (
    echo Deleting folder output\%identifier%...
    rmdir /s /q "output\%identifier%"
)

rem --- Run the main executable for each test ---
echo Running main tests...
for /L %%i in (0,1,%ntests%) do (
    echo Running test number %%i...
    echo Generating demand matrix...
    python .\src\worst_case.py %n%

    echo Computing Demand Matrix Entropy...
    python .\src\bounds.py %identifier%

    echo Running main...
    .\bin\main.exe %identifier% --test-number=%%i --algorithms=%algorithms%
    if errorlevel 1 (
        echo Test %%i failed.
    )

    echo ----------------------------------------------------
)

echo All tests completed.
pause

@echo off
rem ************************************************************
rem *  run_tests.bat
rem *
rem *  Usage: run_tests.bat -n 1000 -src-size 0.3 -dst-size 0.3 -ntests 10 -algorithms "onehop-mloggap-basic"
rem *
rem *  This script compiles two C++ files and then executes:
rem *    .\bin\matrix.exe <identifier> -n <n> --src-size <src-size> --dst-size <dst-size> -u
rem *  followed by
rem *    .\bin\main.exe <identifier> --test-number=<i>
rem *  for each i from 0 to ntests.
rem ************************************************************

rem --- Parse command-line arguments ---
set "n="
set "src_size="
set "dst_size="
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
if "%~1"=="-src-size" (
    set "src_size=%2"
    shift
    shift
    goto parse
)
if "%~1"=="-dst-size" (
    set "dst_size=%2"
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
if "%src_size%"=="" (
    echo Error: Missing parameter -src-size.
    exit /b 1
)
if "%dst_size%"=="" (
    echo Error: Missing parameter -dst-size.
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
echo    src_size = %src_size%
echo    dst_size = %dst_size%
echo    ntests  = %ntests%
echo    algorithms = %algorithms%

rem --- Compile the C++ files ---
echo Compiling main...
g++ -fopenmp -O2 -std=c++17 -I include/ .\src\main.cc -o bin/main.exe
if errorlevel 1 (
    echo Compilation of main failed.
    exit /b 1
)

echo Compiling getMatrixEntropy...
g++ -std=c++17 -I include/ .\src\getMatrixEntropy.cc -o bin/entropy.exe
if errorlevel 1 (
    echo Compilation of getMatrixEntropy failed.
    exit /b 1
)

echo Compiling createDemandMatrix...
g++ -std=c++17 -I include/ .\src\createDemandMatrix.cc -o bin/matrix.exe
if errorlevel 1 (
    echo Compilation of createDemandMatrix failed.
    exit /b 1
)

rem --- Build identifier string ---
set "identifier=%n%-%src_size%-%dst_size%"

if exist "output\%identifier%" (
    echo Deleting folder output\%identifier%...
    rmdir /s /q "output\%identifier%"
)

rem --- Run the main executable for each test ---
echo Running main tests...
for /L %%i in (0,1,%ntests%) do (
    echo Running test number %%i...
    echo Generating demand matrix...
    .\bin\matrix.exe %identifier% -n %n% --src-size %src_size% --dst-size %dst_size% -u

    echo Computing Demand Matrix Entropy...
    .\bin\entropy.exe %identifier%

    echo Running main...
    .\bin\main.exe %identifier% --test-number=%%i --algorithms=%algorithms%
    if errorlevel 1 (
        echo Test %%i failed.
    )

    echo ----------------------------------------------------
)

echo All tests completed.
pause

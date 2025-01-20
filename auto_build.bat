@echo off

REM Change to the build directory
cd build
IF %ERRORLEVEL% NEQ 0 (
    echo Build directory not found!
    exit /b 1
)

REM Run the build command
cmake --build . 
IF %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

REM Run the executable
send_blob.exe
IF %ERRORLEVEL% NEQ 0 (
    echo Execution failed!
    exit /b 1
)

REM Return to original directory
cd ..
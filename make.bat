@ECHO OFF

SETLOCAL ENABLEDELAYEDEXPANSION

FOR %%X IN (ninja.exe) DO (
    SET NinjaFound=%%~$PATH:X
)

IF DEFINED NinjaFound (
    SET Generator=Ninja
) ELSE (
    SET Generator=JOM
)

TITLE Making OpenCOR (using !Generator!)...

IF EXIST "C:\Program Files (x86)\" (
    SET ProgFilesDir=C:\Program Files ^(x86^)
) ELSE (
    SET ProgFilesDir=C:\Program Files
)

IF EXIST "C:\Qt\5.4\msvc2013_64\bin\" (
    CALL "!ProgFilesDir!\Microsoft Visual Studio 12.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
) ELSE (
    CALL "!ProgFilesDir!\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"
)

CD build

IF DEFINED NinjaFound (
    SET CMakeGenerator=Ninja
) ELSE (
    SET CMakeGenerator=NMake Makefiles JOM
)

cmake -G "!CMakeGenerator!" -DSHOW_INFORMATION_MESSAGE=ON -DCMAKE_BUILD_TYPE=Release ..

SET ExitCode=!ERRORLEVEL!

IF !ExitCode! EQU 0 (
    IF DEFINED NinjaFound (
        ninja

        SET ExitCode=!ERRORLEVEL!
    ) ELSE (
        jom

        SET ExitCode=!ERRORLEVEL!
    )
)

CD ..

EXIT /B !ExitCode!

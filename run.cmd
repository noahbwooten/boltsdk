@echo off
setlocal

rem
rem run.cmd
rem (c)Noah Wooten 2023 - 2026, All Rights Reserved
rem
rem Put applications on the SDK's disk and start the machine, in a window.
rem
rem   run.cmd                           the machine, as its disk was left
rem   run.cmd examples\calc             a project's newest build on it first
rem   run.cmd examples\calc\out\debug\sdkcalc.bxf     an application by itself
rem   run.cmd examples\calc\out\debug\sdkcalc.bxi     a package, to install
rem   run.cmd --clean examples\calc     a fresh disk before anything goes on
rem   run.cmd --memory 1024             anything else goes to run_qemu.py
rem
rem An application goes into /programs/<module> with a link at the top of the
rem Actions menu, so it is a click away once the desktop is up. A package goes
rem on the desktop, and opening it installs it the way it would be installed
rem anywhere else. Whatever was there already under the same name is replaced.
rem
rem The disk is run\bolt-disk.img, made from image\fs.bin the first time. It
rem keeps what the machine writes to it -- settings, files, what was installed
rem -- until --clean makes it again. QEMU has to be installed; see
rem docs\setup.md.
rem
rem An argument that names a file or a folder is something to put on the disk,
rem and anything else is passed to run_qemu.py, which is how --memory 1024 and
rem --disk-bus nvme get there.
rem

rem Captured before the arguments are walked, because shift renumbers %0 too
rem and %~dp0 would then name the working directory instead of this one.
set "HERE=%~dp0"

set "CLEAN="
set "WHAT="
set "EXTRA="

rem --- arguments ---------------------------------------------------------------

rem Labels rather than bracketed blocks, so an argument holding a bracket
rem cannot end a block early.
:arguments
if "%~1"=="" goto parsed
if /i "%~1"=="--clean" goto take_clean
if /i "%~1"=="--help" goto usage
if /i "%~1"=="-h" goto usage
if "%~1"=="/?" goto usage
if exist "%~1" goto take_what
set "EXTRA=%EXTRA% %1"
shift
goto arguments

:take_clean
set "CLEAN=--clean"
shift
goto arguments

:take_what
set "WHAT=%WHAT% %1"
shift
goto arguments

:usage
echo usage: run.cmd [--clean] [project ^| app.bxf ^| package.bxi ...] [run_qemu.py arguments]
echo.
echo   Puts each project, application or package on the SDK's disk, then starts
echo   the machine in a window. With nothing to put on, it starts the disk as it
echo   was left.
echo.
echo   --clean    make the disk again from image\fs.bin first
echo.
echo   Anything else goes to run_qemu.py: --memory 1024, --disk-bus nvme.
exit /b 0

:parsed
where python > nul 2>&1
if errorlevel 1 (
    echo run: python is not on PATH; see docs\setup.md
    exit /b 1
)

rem --- the disk ----------------------------------------------------------------

python "%HERE%tools\slipstream.py" --disk "%HERE%run\bolt-disk.img" --image "%HERE%image\fs.bin" %CLEAN%%WHAT%
if errorlevel 1 exit /b 1

rem --- the machine -------------------------------------------------------------

rem --keep-disk because the disk is this script's to keep, not run_qemu.py's:
rem left to itself it would make the disk again whenever the image was newer,
rem and take everything that was put on it with it.
echo.
echo starting the machine
python "%HERE%tools\run_qemu.py" --efi "%HERE%image\bootx64.efi" --fs-image "%HERE%image\fs.bin" --run-dir "%HERE%run" --keep-disk --interactive%EXTRA%

exit /b %errorlevel%

@echo off
setlocal

rem
rem build.cmd
rem (c)Noah Wooten 2023 - 2026, All Rights Reserved
rem
rem Build an application with the Bolt SDK.
rem
rem   build.cmd examples\calc               a Debug build
rem   build.cmd examples\calc --release     optimised
rem   build.cmd examples\calc --clean       the output thrown away first
rem
rem A project is a folder with an app.ini in it; docs\guide\project-file.md
rem describes it. What comes out is in the project's out\debug or out\release:
rem the application as <module>.bxf, and its package as <module>.bxi when
rem app.ini has a [package]. run.cmd puts either on the disk.
rem
rem One line over tools\bxbuild.py, which is where the work is. clang and lld
rem are looked for in toolchain\bin first, then on PATH; see docs\setup.md.
rem

if "%~1"=="" goto usage
if /i "%~1"=="--help" goto usage
if /i "%~1"=="-h" goto usage
if "%~1"=="/?" goto usage

where python > nul 2>&1
if errorlevel 1 (
    echo build: python is not on PATH; see docs\setup.md
    exit /b 1
)

python "%~dp0tools\bxbuild.py" %*
exit /b %errorlevel%

:usage
echo usage: build.cmd ^<project^> [--release] [--clean]
echo.
echo   Builds the project whose app.ini is in that folder: build.cmd examples\calc
exit /b 0

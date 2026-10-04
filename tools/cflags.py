"""What a BoltOS module is compiled and linked with.

The flags themselves, apart from the build that uses them, so that the two
builds that make modules -- scripts/build.py, which makes the system, and the
SDK's bxbuild.py, which makes an application -- cannot drift apart. The SDK
ships this file as it is. A flag changed here changes both, which is the point:
an application compiled differently from the modules it calls into is a
problem found the hard way.

What each module adds -- its defines, its optimisation, where its headers
are -- stays with whoever builds it.
"""

# clang in its MSVC-compatible driver, so the flags below are the ones the
# project system used and the ones every comment in this tree describes.
CL_LEADING = ["/c", "/Z7", "/nologo", "/W3", "/WX-", "/diagnostics:column"]

CL_TRAILING = ["/fp:precise", "/Zc:wchar_t", "/Zc:forScope", "/Zc:inline",
               "/permissive-", "/Gd", "/TC", "/FC"]

# /X    no system include path at all. BoltOS supplies its own string.h,
#       stdio.h, math.h and stdarg.h, and the /I paths below already find them
#       ahead of anything a toolchain ships. Saying so outright is what lets a
#       Debian or macOS host compile this without a Windows SDK present.
# /Zl   emit no default library name. MSVC's /MD wrote a reference to msvcrt
#       into every object, which a cross host cannot resolve and which nothing
#       here needs: the CRT is never initialised in a hand-mapped image.
# /GS-  no stack security cookie. It is provided by the CRT, which is absent,
#       so the check would call through a null pointer at the first function
#       that overflowed.
# /Gs   push the stack probe threshold past any frame here, so no call to
#       __chkstk is generated. That symbol lives in the CRT too, and MSVC only
#       reached for it above a page; clang reaches for it more readily and the
#       kernel would not link.
FREESTANDING = ["/X", "/Zl", "/GS-", "/Gs1048576"]

# BoltOS implements its own string.h and its signatures are not the library's:
# strdup takes a char* here and clang's builtin takes a const char*. Left on,
# clang would both warn about every one of them and feel free to rewrite a call
# using semantics these functions do not promise. It also stops memset being
# compiled into a call to itself, which is what MSVC's #pragma function bought.
NO_BUILTIN = ["-fno-builtin"]

# Where clang and MSVC disagree about style rather than substance.
#
# Turned off deliberately and one at a time, not as a block. Everything clang
# reports that can actually be wrong at run time is left on, and the first
# build under it found two: KmStrToInt calling a two-argument function with one
# argument, and the input poll writing four bytes through a pointer to a
# one-byte array element. Both are fixed. Silencing the category instead would
# have shipped them.
QUIET = [
    "-Wno-macro-redefined",                 # BoltOS defines NULL twice
    "-Wno-ignored-pragma-intrinsic",        # what -fno-builtin already covers
    "-Wno-incompatible-library-redeclaration",  # BoltOS strdup is not the library's
    "-Wno-incompatible-pointer-types-discards-qualifiers",
    "-Wno-pointer-sign",
    "-Wno-unused-variable",
    "-Wno-unused-function",
    "-Wno-pragma-once-outside-header",
    "-Wno-pointer-to-int-cast",

    # Reported and not fatal, rather than silenced. Most of these are an
    # unsigned long* handed to a function taking int*, which is the same four
    # bytes on this target and harmless. The dangerous shape is a byte array
    # element passed as int*, which is not the same four bytes, so the warning
    # is worth keeping in front of whoever runs the build.
    "-Wno-error=incompatible-pointer-types",
]

# Defined for every module. WIN32 and the rest are what the project files
# defined, and the headers still read some of them.
COMMON_DEFINES = ["WIN32", "_CONSOLE", "_WINDLL", "_MBCS"]

# DWARF alongside the CodeView /Z7 already asks for, in Debug. CodeView is what
# becomes the PDB and what the map sits beside; DWARF is what QEMU's GDB stub
# can be pointed at. Four rather than five because the gdb most likely to be
# installed predates DWARF 5.
DEBUG_INFO = ["/clang:-gdwarf-4"]

# What Release adds to every compile. /Gy puts each function in its own
# section, which is what lets /OPT:REF below throw away the ones nothing calls.
RELEASE_COMPILE = ["-flto=thin", "/Gy"]

# Every link.
#
# /NOENTRY and /DLL, for every module. These images are hand-mapped by
# HalLoadModule and entered at __modmain, which the loader finds in the export
# table. They have no DllMain and the CRT never initialises them, so an entry
# point would name a function that is never called and pull in startup code
# that must not run.
LINK_COMMON = ["/INCREMENTAL:NO", "/NOLOGO", "/DYNAMICBASE", "/NXCOMPAT",
               "/MACHINE:X64", "/NOENTRY", "/DLL"]

# /OPT:REF and /OPT:ICF are what make a Release build smaller and are not
# implied by link-time optimisation. Off in Debug for the usual reason: folding
# identical functions and discarding unreferenced ones makes a stack trace
# harder to read.
LINK_RELEASE = ["/OPT:REF", "/OPT:ICF"]

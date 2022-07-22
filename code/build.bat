@echo off
    pushd ..\build
        cl -FC -Zi ..\code\ishan_handmade.cpp user32.lib gdi32.lib
    popd
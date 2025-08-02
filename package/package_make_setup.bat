@echo off

for /d %%d in (release_*) do rmdir /s /q "%%d"

python install.py GammaRay

rem binarycreator.exe -c config/config.xml -p packages GammaRaySetup.exe -v
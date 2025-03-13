rmdir /s /q packages\com.rgaa.gammaray\data

python vs_install.py rel-debug

binarycreator.exe -c config/config.xml -p packages GammaRaySetup_DebugInfo.exe -v
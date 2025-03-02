rmdir /s /q packages\com.rgaa.gammaray\data

python install.py release

binarycreator.exe -c config/config.xml -p packages GammaRaySetup.exe -v
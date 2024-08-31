//
// Created by RGAA on 31/08/2024.
//

#include <Windows.h>
#include <iostream>

int main() {
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);
    EnumDisplayDevices(NULL, 0, &dd, 0);
    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    int iModeNum = 0;
    while (EnumDisplaySettingsExW(dd.DeviceName, iModeNum, &dm, 0)) {
        std::wcout << "name: " << dd.DeviceName << " Width: " << dm.dmPelsWidth << "  Height: " << dm.dmPelsHeight << std::endl;
        iModeNum++;
    }

    return 0;
}
//
// Created by RGAA on 13/11/2025.
//

#ifndef GAMMARAYPREMIUM_SKIN_OFFICIAL_H
#define GAMMARAYPREMIUM_SKIN_OFFICIAL_H

#include "skin/interface/skin_interface.h"

namespace tc
{
    class SkinOpenSource : public SkinInterface {
    public:
        QString GetSkinName() override;

        // app name
        QString GetAppName() override;

        // version
        // eg: 1.3.5
        QString GetAppVersionName() override;

        // eg: Premium / Pro ...
        QString GetAppVersionMode() override;

        // colors
        int GetPrimaryColor() override;

        int GetSecondaryColor() override;

        int GetHeadTextColor() override;

        int GetSubHeadTextColor() override;

        int GetMainTextColor() override;

        int GetSecondaryTextColor() override;

        // icons
        QPixmap GetWindowIcon() override;

        QPixmap GetLargeIconTextLogo() override;

        QPixmap GetSquareLogo() override;

        QPixmap GetSquarePrimaryColorLogoTransBg() override;

        QPixmap GetSquareWhiteLogoTransBg() override;
    };
}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAYPREMIUM_SKIN_OFFICIAL_H

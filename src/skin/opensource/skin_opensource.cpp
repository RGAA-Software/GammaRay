//
// Created by RGAA on 13/11/2025.
//

#include "skin_opensource.h"
#include "version_config.h"

void* GetInstance() {
    static tc::SkinOpenSource impl;
    return (void*)&impl;
}

namespace tc
{

    QString SkinOpenSource::GetSkinName() {
        return "Official";
    }

    // app name
    QString SkinOpenSource::GetAppName() {
        return "GammaRay";
    }

    // version
    // eg: 1.3.5
    QString SkinOpenSource::GetAppVersionName() {
        return PROJECT_VERSION;
    }

    // eg: Premium / Pro ...
    QString SkinOpenSource::GetAppVersionMode() {
        return "Premium";
    }

    // colors
    int SkinOpenSource::GetPrimaryColor() {
        return 0;
    }

    int SkinOpenSource::GetSecondaryColor() {
        return 0;
    }

    int SkinOpenSource::GetHeadTextColor() {
        return 0;
    }

    int SkinOpenSource::GetSubHeadTextColor() {
        return 0;
    }

    int SkinOpenSource::GetMainTextColor() {
        return 0;
    }

    int SkinOpenSource::GetSecondaryTextColor() {
        return 0;
    }

    // icons
    QPixmap SkinOpenSource::GetWindowIcon() {
        QPixmap p;
        return p;
    }

    QPixmap SkinOpenSource::GetLargeIconTextLogo() {
        QPixmap p;
        p.load(":/skin/resources/tc_logo_text_trans_bg.png");
        return p;
    }

    QPixmap SkinOpenSource::GetSquareLogo() {
        QPixmap p;
        p.load(":/skin/resources/tc_icon.png");
        return p;
    }

    QPixmap SkinOpenSource::GetSquarePrimaryColorLogoTransBg() {
        QPixmap p;
        return p;
    }

    QPixmap SkinOpenSource::GetSquareWhiteLogoTransBg() {
        QPixmap p;
        return p;
    }

}
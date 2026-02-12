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
        return "OpenSource";
    }

    // app name
    QString SkinOpenSource::GetAppName() {
        return "GoDesk";
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
        static QPixmap large_icon_text_logo;
        if (large_icon_text_logo.isNull()) {
            large_icon_text_logo.load(":/skin/resources/tc_logo_text_trans_bg.png");
        }
        return large_icon_text_logo;
    }

    QPixmap SkinOpenSource::GetSquareLogo() {
        static QPixmap square_logo;
        if (square_logo.isNull()) {
            square_logo.load(":/skin/resources/tc_icon.png");
        }
        return square_logo;
    }

    QPixmap SkinOpenSource::GetSquarePrimaryColorLogoTransBg() {
        static QPixmap primary_color_logo_trans_bg;
        if (primary_color_logo_trans_bg.isNull()) {
            primary_color_logo_trans_bg.load(":/skin/resources/tc_trans_icon_blue.png");
        }
        return primary_color_logo_trans_bg;
    }

    QPixmap SkinOpenSource::GetSquareWhiteLogoTransBg() {
        static QPixmap white_logo_trans_bg;
        if (white_logo_trans_bg.isNull()) {
            white_logo_trans_bg.load(":/skin/resources/tc_trans_icon_white.png");
        }
        return white_logo_trans_bg;
    }

}
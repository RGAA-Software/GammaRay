//
// Created by RGAA on 2023/8/16.
//

#ifndef SAILFISH_SERVER_SETTINGSCONTENT_H
#define SAILFISH_SERVER_SETTINGSCONTENT_H

#include "app_content.h"

namespace tc
{

    class ClientContext;
    class Settings;
    class MultiDisplayModeWidget;

    class SettingsContent : public AppContent {
    public:

        explicit SettingsContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        ~SettingsContent() override;

        void OnContentShow() override;
        void OnContentHide() override;

    private:

        Settings* settings_ = nullptr;
        MultiDisplayModeWidget* combined_;
        MultiDisplayModeWidget* separated_;
    };

}

#endif //SAILFISH_SERVER_INFORMATIONCONTENT_H

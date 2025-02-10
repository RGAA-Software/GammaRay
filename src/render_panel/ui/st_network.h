//
// Created by RGAA on 2024-06-10.
//

#ifndef GAMMARAY_ST_NETWORK_H
#define GAMMARAY_ST_NETWORK_H

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include "tab_base.h"

namespace tc
{
    class GrApplication;

    class StNetwork : public TabBase {
    public:
        explicit StNetwork(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StNetwork() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        QLabel* license_ = nullptr;
        QLineEdit* edt_sig_server_address_ = nullptr;
        QLineEdit* edt_sig_server_port_ = nullptr;
        QLineEdit* edt_coturn_server_address_ = nullptr;
        QLineEdit* edt_coturn_server_port_ = nullptr;
        QCheckBox* cb_websocket_ = nullptr;
        QCheckBox* cb_webrtc_ = nullptr;
    };

}

#endif //GAMMARAY_ST_ABOUT_ME_H

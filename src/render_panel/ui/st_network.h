//
// Created by RGAA on 2024-06-10.
//

#ifndef GAMMARAY_ST_NETWORK_H
#define GAMMARAY_ST_NETWORK_H

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include "tab_base.h"

namespace tc
{

    class GrApplication;
    class MessageListener;
    class SpvrAccessInfo;

    class StNetwork : public TabBase {
    public:
        explicit StNetwork(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StNetwork() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        std::shared_ptr<SpvrAccessInfo> ParseSpvrAccessInfo(const std::string& info);
        void DisplaySpvrAccessInfo(const std::shared_ptr<SpvrAccessInfo>& info);
        void SaveSpvrAccessInfo();
        void SearchAccessInfo();
        void VerifyAccessInfo();

    private:
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        QTextEdit* edt_spvr_access_ = nullptr;
        QLineEdit* edt_spvr_server_host_ = nullptr;
        QLineEdit* edt_spvr_server_port_ = nullptr;
        QLineEdit* edt_relay_server_host_ = nullptr;
        QLineEdit* edt_relay_server_port_ = nullptr;
        QCheckBox* cb_websocket_ = nullptr;
        QLineEdit* edt_websocket_ = nullptr;
        QCheckBox* cb_udp_kcp_ = nullptr;
        QLineEdit* edt_udp_kcp_ = nullptr;
        QCheckBox* cb_webrtc_ = nullptr;
        QLineEdit* edt_panel_port_ = nullptr;
    };

}

#endif //GAMMARAY_ST_ABOUT_ME_H

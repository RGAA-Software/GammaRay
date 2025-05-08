//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_FLOAT_SUB_DISPLAY_PANEL_H
#define GAMMARAYPC_FLOAT_SUB_DISPLAY_PANEL_H

#include "base_widget.h"
#include <QPainter>
#include <string>
#include "client/ct_app_message.h"

namespace tc
{

    enum SubDisplayType {
        kScale,
        kResolution,
    };

    class MessageListener;
    class SwitchButton;

    class SubDisplayPanel : BaseWidget {
    public:
        explicit SubDisplayPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void Show() override;
        void Hide() override;
        void UpdateMonitorInfo(const CaptureMonitorMessage& m);
        void SetCaptureMonitorName(const std::string& name);
    private:
        BaseWidget* GetSubPanel(const SubDisplayType& type);
        void HideAllSubPanels();
        void UpdateStatus(const FloatControllerPanelUpdateMessage& msg) override;
    private:
        std::map<SubDisplayType, BaseWidget*> sub_panels_;
        CaptureMonitorMessage cap_monitors_info_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;

        std::string capture_monitor_name_;

        SwitchButton* full_color_btn_ = nullptr;
    };

}

#endif

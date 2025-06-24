//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_FLOAT_3RD_RESOLUTION_PANEL_H
#define GAMMARAYPC_FLOAT_3RD_RESOLUTION_PANEL_H

#include "base_widget.h"
#include <QPainter>
#include "client/ct_app_message.h"

namespace tc
{

    class Settings;
    class SwitchButton;
    class SingleSelectedList;

    class ThirdResolutionPanel : BaseWidget {
    public:
        explicit ThirdResolutionPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void Hide() override;
        void Show() override;
        void UpdateMonitor(const MsgClientCaptureMonitor::CaptureMonitor& m);
    private:
        void SelectCapturingMonitorSize();

    private:
        Settings* settings_ = nullptr;
        SingleSelectedList* listview_ = nullptr;
        MsgClientCaptureMonitor::CaptureMonitor monitor_;
    };

}

#endif

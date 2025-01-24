//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_FLOAT_SUB_DISPLAY_PANEL_H
#define GAMMARAYPC_FLOAT_SUB_DISPLAY_PANEL_H

#include "base_widget.h"
#include <QPainter>
#include "app_message.h"

namespace tc
{

    enum SubDisplayType {
        kScale,
        kResolution,
    };

    class SubDisplayPanel : BaseWidget {
    public:
        explicit SubDisplayPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void Show() override;
        void Hide() override;
        void UpdateMonitorInfo(const CaptureMonitorMessage& m);

    private:
        BaseWidget* GetSubPanel(const SubDisplayType& type);
        void HideAllSubPanels();

    private:
        std::map<SubDisplayType, BaseWidget*> sub_panels_;
        CaptureMonitorMessage monitors_;
    };

}

#endif

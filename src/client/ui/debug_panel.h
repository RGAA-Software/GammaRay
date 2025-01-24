//
// Created by RGAA on 12/08/2024.
//

#ifndef GAMMARAYPC_DEBUG_PANEL_H
#define GAMMARAYPC_DEBUG_PANEL_H

#include "base_widget.h"
#include "tc_message.pb.h"

namespace tc
{

    class KeyStatePanel;
    class FloatIcon;

    class DebugPanel : public BaseWidget {
    public:
        explicit DebugPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);

        void resizeEvent(QResizeEvent *event) override;
        void paintEvent(QPaintEvent *event) override;

        void UpdateOnHeartBeat(const OnHeartBeat& hb);

    private:
        KeyStatePanel* key_state_panel_ = nullptr;
        FloatIcon* close_btn_ = nullptr;
    };

}

#endif //GAMMARAYPC_DEBUG_PANEL_H

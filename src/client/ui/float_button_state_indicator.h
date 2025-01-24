//
// Created by RGAA on 2/09/2024.
//

#ifndef GAMMARAYPC_FLOAT_BUTTON_STATE_INDICATOR_H
#define GAMMARAYPC_FLOAT_BUTTON_STATE_INDICATOR_H

#include "base_widget.h"
#include "tc_message.pb.h"

namespace tc
{

    class ClientContext;
    class KeyStatePanel;
    class KeyItem;

    class FloatButtonStateIndicator : public BaseWidget {
    public:
        explicit FloatButtonStateIndicator(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void UpdateOnHeartBeat(const OnHeartBeat& hb);
        int GetPressedCount();

    private:
        KeyItem* alt_item_ = nullptr;
        KeyItem* shift_item_ = nullptr;
        KeyItem* control_item_ = nullptr;
        KeyItem* win_item_ = nullptr;
    };

}

#endif //GAMMARAYPC_FLOAT_BUTTON_STATE_INDICATOR_H

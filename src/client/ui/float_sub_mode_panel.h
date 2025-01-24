//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_FLOAT_SUB_MODE_PANEL_H
#define GAMMARAYPC_FLOAT_SUB_MODE_PANEL_H

#include "base_widget.h"
#include <QPainter>

namespace tc
{

    class Settings;
    class SwitchButton;

    class SubModePanel : BaseWidget {
    public:
        explicit SubModePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

    private:
        Settings* settings_ = nullptr;
        SwitchButton* sb_work_ = nullptr;
        SwitchButton* sb_game_ = nullptr;
    };

}

#endif //GAMMARAYPC_FLOAT_SUB_MODE_PANEL_H

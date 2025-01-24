//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_FLOAT_3RD_SCALE_PANEL_H
#define GAMMARAYPC_FLOAT_3RD_SCALE_PANEL_H

#include "base_widget.h"
#include <QPainter>
#include "settings.h"

namespace tc
{

    class Settings;
    class SingleSelectedList;

    class ThirdScalePanel : BaseWidget {
    public:
        explicit ThirdScalePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

    private:
        void UpdateScaleMode(ScaleMode mode);

    private:
        Settings* settings_ = nullptr;
        SingleSelectedList* listview_ = nullptr;
    };

}

#endif

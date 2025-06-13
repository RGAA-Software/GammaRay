//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_FLOAT_SUB_MODE_PANEL_H
#define GAMMARAYPC_FLOAT_SUB_MODE_PANEL_H

#include "base_widget.h"
#include <QPainter>
#include <map>

namespace tc
{

    class Settings;
    class SwitchButton;
    class SingleSelectedList;

    class SubFpsPanel : BaseWidget {
    public:
        enum class EFps {
            k15Fps ,
            k30Fps ,
            k60Fps ,
            k90Fps ,
            k120Fps,
            k144Fps,
        };
    public:
        explicit SubFpsPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void UpdateStatus(const FloatControllerPanelUpdateMessage& msg) override;
    private:
        Settings* settings_ = nullptr;
        SwitchButton* sb_15_fps_ = nullptr;
        SwitchButton* sb_30_fps_ = nullptr;
        SwitchButton* sb_60_fps_ = nullptr;
        SwitchButton* sb_120_fps_ = nullptr;
        SwitchButton* sb_144_fps_ = nullptr;

        std::map<EFps, SwitchButton*> fps_buttons_;

        SingleSelectedList* listview_ = nullptr;


        std::map<EFps, int> fps_info_;

    private:
        int GetFpsIndex(int fps);
    };

}

#endif //GAMMARAYPC_FLOAT_SUB_MODE_PANEL_H

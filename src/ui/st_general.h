//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_INPUT_H
#define TC_SERVER_STEAM_ST_INPUT_H

#include "tab_base.h"

#include <QLineEdit>
#include <QCheckBox>

namespace tc
{

    class StGeneral : public TabBase {
    public:
        StGeneral(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~StGeneral() override = default;

        void OnTabShow() override;
        void OnTabHide() override;
    private:
        QLineEdit* et_bitrate_;
        QLineEdit* et_res_width_;
        QLineEdit* et_res_height_;
        QCheckBox* cb_resize_res_;
        QComboBox* cb_capture_monitor_;
        QComboBox* cb_capture_audio_device_;
    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H

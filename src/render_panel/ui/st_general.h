//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_INPUT_H
#define TC_SERVER_STEAM_ST_INPUT_H

#include "tab_base.h"

#include <QLineEdit>
#include <QCheckBox>
class QPushButton;

namespace tc
{

    class StGeneral : public TabBase {
    public:
        StGeneral(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~StGeneral() override = default;

        void OnTabShow() override;
        void OnTabHide() override;
    private:
        QLineEdit* et_bitrate_ = nullptr;
        QComboBox* et_fps_ = nullptr;
        QComboBox* cb_language_ = nullptr;
        QComboBox* cb_skin_ = nullptr;
        QLineEdit* et_res_width_ = nullptr;
        QLineEdit* et_res_height_ = nullptr;
        QCheckBox* cb_resize_res_ = nullptr;
        QComboBox* cb_capture_monitor_ = nullptr;
        QComboBox* cb_capture_audio_device_name_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H

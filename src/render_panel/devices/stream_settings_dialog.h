//
// Created by RGAA on 2023-08-18.
//

#ifndef SAILFISH_CLIENT_PC_STREAM_SETTINGS_DIALOG_H
#define SAILFISH_CLIENT_PC_STREAM_SETTINGS_DIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QPainter>
#include <QCheckBox>

#include "tc_spvr_client/spvr_stream.h"
#include "tc_qt_widget/tc_custom_titlebar_dialog.h"

namespace tc
{

    class GrContext;
    class StreamDBOperator;

    class StreamSettingsDialog : public TcCustomTitleBarDialog {
    public:
        StreamSettingsDialog(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<spvr::SpvrStream>& item, QWidget* parent = nullptr);
        ~StreamSettingsDialog() override;

        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<StreamDBOperator> db_mgr_ = nullptr;
        QCheckBox* cb_audio_ = nullptr;
        QCheckBox* cb_clipboard_ = nullptr;
        QCheckBox* cb_only_viewing_ = nullptr;
        QCheckBox* cb_show_max_ = nullptr;
        QCheckBox* cb_split_windows_ = nullptr;
        QCheckBox* cb_force_relay_ = nullptr;
        QCheckBox* cb_force_software_ = nullptr;
        QLineEdit* ed_bitrate_ = nullptr;
        QLineEdit* ed_remote_device_id_ = nullptr;
        QComboBox* cb_fps_ = nullptr;
        QRadioButton* rb_ws_ = nullptr;
        QRadioButton* rb_relay_ = nullptr;
        std::shared_ptr<spvr::SpvrStream> stream_item_;

    };

}

#endif //SAILFISH_CLIENT_PC_CREATESTREAMDIALOG_H

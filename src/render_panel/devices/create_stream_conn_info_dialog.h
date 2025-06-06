//
// Created by RGAA on 2023-08-18.
//

#ifndef SAILFISH_CLIENT_PC_CREATESTREAM_CONN_INFO_DIALOG_H
#define SAILFISH_CLIENT_PC_CREATESTREAM_CONN_INFO_DIALOG_H

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
#include <QTextEdit>

#include "tc_qt_widget/tc_custom_titlebar_dialog.h"

namespace tc
{

    class GrContext;

    // Paste gammaray://xxx
    class CreateStreamConnInfoDialog : public TcCustomTitleBarDialog {
    public:
        explicit CreateStreamConnInfoDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~CreateStreamConnInfoDialog() override;
        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();
        bool GenStream();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* ed_device_name_ = nullptr;
        QTextEdit* ed_conn_info_ = nullptr;

    };

}

#endif

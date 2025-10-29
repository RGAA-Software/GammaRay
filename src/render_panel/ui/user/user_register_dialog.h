//
// Created by RGAA on 2023-08-18.
//

#pragma once

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
#include "tc_qt_widget/tc_custom_titlebar_dialog.h"

namespace tc
{

    class SpvrUser;
    class GrContext;
    class TcPasswordInput;

    class UserRegisterDialog : public TcCustomTitleBarDialog {
    public:
        UserRegisterDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~UserRegisterDialog() override;

        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* edt_stream_name_ = nullptr;
        TcPasswordInput* password_input_ = nullptr;

    };

}

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
        explicit UserRegisterDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~UserRegisterDialog() override;
        std::string GetInputUsername();
        std::string GetInputPassword();

        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();
        void Register();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* edt_username_ = nullptr;
        TcPasswordInput* password_input_ = nullptr;
        TcPasswordInput* re_password_input_ = nullptr;

    };

}

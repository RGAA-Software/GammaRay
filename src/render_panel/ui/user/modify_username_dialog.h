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

    class ModifyUsernameDialog : public TcCustomTitleBarDialog {
    public:
        ModifyUsernameDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~ModifyUsernameDialog() override;

        std::string GetUsername();
        std::string GetPassword();

        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();
        void Login();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* edt_username_ = nullptr;
        TcPasswordInput* password_input_ = nullptr;

    };

}

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

    class ModifyPasswordDialog : public TcCustomTitleBarDialog {
    public:
        explicit ModifyPasswordDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        ~ModifyPasswordDialog() override;

        std::string GetPassword();
        std::string GetPasswordAgain();

        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();
        void ModifyPassword();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        TcPasswordInput* password_input_ = nullptr;
        TcPasswordInput* password_input_again_ = nullptr;

    };

}

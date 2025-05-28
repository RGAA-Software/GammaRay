//
// Created by RGAA on 2023-08-18.
//

#ifndef SAILFISH_CLIENT_PC_INPUT_SAFETY_PWD_DIALOG_H
#define SAILFISH_CLIENT_PC_INPUT_SAFETY_PWD_DIALOG_H

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
    class GrApplication;
    class TcPasswordInput;

    class InputSafetyPwdDialog : public TcCustomTitleBarDialog {
    public:
        explicit InputSafetyPwdDialog(const std::shared_ptr<GrApplication>& ctx, QWidget* parent = nullptr);
        ~InputSafetyPwdDialog() override;
        void paintEvent(QPaintEvent *event) override;
        void closeEvent(QCloseEvent *) override;
        QString GetInputPassword();

    private:
        void CreateLayout();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        TcPasswordInput* pwd_input_ = nullptr;
        TcPasswordInput* pwd_input_again_ = nullptr;

    };

}

#endif

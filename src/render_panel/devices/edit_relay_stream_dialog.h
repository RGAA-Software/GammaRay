//
// Created by RGAA on 2023-08-18.
//

#ifndef SAILFISH_CLIENT_PC_EDIT_RELAY_STREAM_DIALOG_H
#define SAILFISH_CLIENT_PC_EDIT_RELAY_STREAM_DIALOG_H

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

#include "render_panel/database/stream_item.h"
#include "tc_qt_widget/tc_custom_titlebar_dialog.h"

namespace tc
{

    class GrContext;
    class TcPasswordInput;

    class EditRelayStreamDialog : public TcCustomTitleBarDialog {
    public:
        EditRelayStreamDialog(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<StreamItem>& item, QWidget* parent = nullptr);
        ~EditRelayStreamDialog() override;

        void paintEvent(QPaintEvent *event) override;

    private:
        void CreateLayout();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* edt_stream_name_ = nullptr;
        std::shared_ptr<StreamItem> stream_item_;
        TcPasswordInput* password_input_ = nullptr;

    };

}

#endif //SAILFISH_CLIENT_PC_CREATESTREAMDIALOG_H

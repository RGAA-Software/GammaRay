//
// Created by RGAA on 2023-08-18.
//

#ifndef ST_NETWORK_AUTO_JOIN_DIALOG_H
#define ST_NETWORK_AUTO_JOIN_DIALOG_H

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
    class GrApplication;
    class TcPasswordInput;
    class StNetworkSpvrAccessInfo;

    class StNetworkAutoJoinDialog : public TcCustomTitleBarDialog {
    public:
        StNetworkAutoJoinDialog(const std::shared_ptr<GrApplication>& app, const std::shared_ptr<StNetworkSpvrAccessInfo>& item, QWidget* parent = nullptr);
        ~StNetworkAutoJoinDialog() override;

        void paintEvent(QPaintEvent *event) override;
        void closeEvent(QCloseEvent *) override;

    private:
        void CreateLayout();

    private:
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        QLineEdit* edt_stream_name_ = nullptr;
        std::shared_ptr<StNetworkSpvrAccessInfo> item_ = nullptr;
        TcPasswordInput* password_input_ = nullptr;

    };

}

#endif //SAILFISH_CLIENT_PC_CREATESTREAMDIALOG_H

//
// Created by hy on 2024/1/17.
//

#include "sized_msg_box.h"
#include <QPushButton>

namespace tc
{

    std::shared_ptr<SizedMessageBox> SizedMessageBox::MakeOkCancelBox(const QString& title, const QString& msg) {
        auto msg_box = std::make_shared<SizedMessageBox>();
        auto btn_ok = msg_box->addButton(QMessageBox::StandardButton::Ok);
        auto btn_cancel = msg_box->addButton(QMessageBox::StandardButton::Cancel);
        msg_box->Resize(400, 180);
        msg_box->setWindowTitle(title);
        msg_box->setText(msg);

        connect(btn_ok, &QPushButton::clicked, msg_box.get(), [=]() {
            msg_box->done(0);
        });
        connect(btn_cancel, &QPushButton::clicked, msg_box.get(), [=]() {
            msg_box->done(1);
        });
        return msg_box;
    }

    std::shared_ptr<SizedMessageBox> SizedMessageBox::MakeOkBox(const QString& title, const QString& msg) {
        auto msg_box = std::make_shared<SizedMessageBox>();
        auto btn_ok = msg_box->addButton(QMessageBox::StandardButton::Ok);
        msg_box->Resize(400, 180);
        msg_box->setWindowTitle(title);
        msg_box->setText(msg);

        connect(btn_ok, &QPushButton::clicked, msg_box.get(), [=]() {
            msg_box->done(0);
        });

        return msg_box;
    }

    std::shared_ptr<SizedMessageBox> SizedMessageBox::MakeErrorOkBox(const QString& title, const QString& msg) {
        auto box = MakeOkBox(title, msg);
        box->setWindowTitle(title);
        box->setWindowIcon(QIcon(":/resources/ic_error.png"));
        return box;
    }

    std::shared_ptr<SizedMessageBox> SizedMessageBox::MakeInfoOkBox(const QString& title, const QString& msg) {
        auto box = MakeOkBox(title, msg);
        box->setWindowTitle(title);
        box->setWindowIcon(QIcon(":/resources/ic_info.png"));
        return box;
    }

}

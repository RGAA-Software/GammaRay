//
// Created by RGAA on 2024/1/17.
//

#include "sized_msg_box.h"
#include "no_margin_layout.h"
#include <QLabel>
#include <QPushButton>

namespace tc
{

    std::shared_ptr<SizedMessageBox> SizedMessageBox::MakeOkBox(const QString& title, const QString& msg) {
        auto msg_box = std::make_shared<SizedMessageBox>(true, false);
        msg_box->Resize(400, 190);
        msg_box->setWindowTitle(title);
        msg_box->lbl_message_->setText(msg);

        connect(msg_box->btn_ok_, &QPushButton::clicked, msg_box.get(), [=]() {
            msg_box->done(0);
        });
        return msg_box;
    }

    std::shared_ptr<SizedMessageBox> SizedMessageBox::MakeOkCancelBox(const QString& title, const QString& msg) {
        auto msg_box = std::make_shared<SizedMessageBox>(true, true);
        msg_box->Resize(400, 190);
        msg_box->setWindowTitle(title);
        msg_box->lbl_message_->setText(msg);

        connect(msg_box->btn_ok_, &QPushButton::clicked, msg_box.get(), [=]() {
            msg_box->done(0);
        });
        connect(msg_box->btn_cancel_, &QPushButton::clicked, msg_box.get(), [=]() {
            msg_box->done(1);
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

    SizedMessageBox::SizedMessageBox(bool ok, bool cancel, QWidget* parent) {
        auto layout = new NoMarginVLayout();
        layout->addSpacing(30);
        {
            auto item_layout = new NoMarginHLayout();
            auto lbl_message = new QLabel(this);
            lbl_message->setWordWrap(true);
            lbl_message->setStyleSheet("font-size: 15px;");
            lbl_message_ = lbl_message;
            item_layout->addSpacing(30);
            item_layout->addWidget(lbl_message);
            item_layout->addSpacing(30);
            layout->addLayout(item_layout);
        }
        layout->addStretch(111);
        {
            auto item_layout = new NoMarginHLayout();
            auto btn_size = QSize(90, 28);
            item_layout->addStretch();
            if (cancel) {
                auto btn_cancel = new QPushButton(tr("CANCEL"), this);
                btn_cancel->setProperty("class", "danger");
                btn_cancel_ = btn_cancel;
                btn_cancel->setFixedSize(btn_size);
                item_layout->addWidget(btn_cancel);
                item_layout->addSpacing(15);
            }

            if (ok) {
                auto btn_ok = new QPushButton(tr("OK"), this);
                btn_ok_ = btn_ok;
                btn_ok->setFixedSize(btn_size);
                item_layout->addWidget(btn_ok);
                item_layout->addSpacing(15);
            }

            layout->addLayout(item_layout);
        }
        layout->addSpacing(15);
        setLayout(layout);
    }

}

//
// Created by RGAA on 2023/8/19.
//

#include "message_dialog.h"

#include <QPainter>

namespace tc
{

    MessageDialog::MessageDialog(const std::shared_ptr<ClientContext> &ctx, const QString &msg, QWidget *parent) : QDialog(
            parent) {
        setFixedSize(400, 220);
        msg_ = msg;
        CreateLayout();
    }

    MessageDialog::~MessageDialog() {

    }

    void MessageDialog::CreateLayout() {
        auto root_layout = new QVBoxLayout();
        root_layout->setSpacing(0);
        root_layout->setContentsMargins(0, 0, 0, 0);

        auto msg_layout = new QHBoxLayout();
        msg_layout->setSpacing(0);
        msg_layout->setContentsMargins(0, 0, 0, 0);

        auto msg_label = new QLabel(this);
        msg_label->setText(msg_);
        msg_label->setWordWrap(true);
        msg_layout->addSpacing(30);
        msg_layout->addWidget(msg_label);
        msg_layout->addSpacing(30);

        root_layout->addSpacing(30);
        root_layout->addLayout(msg_layout);

        // btn
        auto btn_size = QSize(100, 35);
        auto btn_layout = new QHBoxLayout();
        btn_layout->setSpacing(0);
        btn_layout->setContentsMargins(0, 0, 0, 0);

        auto btn_cancel = new QPushButton(tr("Cancel"));
        connect(btn_cancel, &QPushButton::clicked, this, [=, this]() {
            done(DialogButton::kCancel);
        });
        btn_cancel->setFixedSize(btn_size);
        btn_layout->addStretch();
        btn_layout->addWidget(btn_cancel);

        auto btn_sure = new QPushButton(tr("Sure"));
        connect(btn_sure, &QPushButton::clicked, this, [=, this]() {
            done(DialogButton::kSure);
        });
        btn_sure->setFixedSize(btn_size);
        btn_layout->addStretch();
        btn_layout->addWidget(btn_sure);
        btn_layout->addStretch();

        root_layout->addStretch();
        root_layout->addLayout(btn_layout);
        root_layout->addSpacing(30);

        setLayout(root_layout);
    }

    void MessageDialog::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(0xffffff)));
        painter.drawRect(0, 0, this->width(), this->height());
    }

}

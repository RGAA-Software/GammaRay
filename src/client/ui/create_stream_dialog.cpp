//
// Created by RGAA on 2023-08-18.
//

#include "create_stream_dialog.h"

#include "app_message.h"
#include "client_context.h"

#include <QValidator>

#include "message_dialog.h"

namespace tc
{

    CreateStreamDialog::CreateStreamDialog(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : QDialog(parent) {
        context_ = ctx;
        setFixedSize(500, 400);
        CreateLayout();
    }

    CreateStreamDialog::CreateStreamDialog(const std::shared_ptr<ClientContext>& ctx, const StreamItem& item, QWidget* parent) : QDialog(parent) {
        context_ = ctx;
        stream_item_ = item;
        setFixedSize(500, 400);
        CreateLayout();
    }

    CreateStreamDialog::~CreateStreamDialog() {

    }

    void CreateStreamDialog::CreateLayout() {
        auto root_layout = new QVBoxLayout();
        root_layout->setSpacing(0);
        root_layout->setContentsMargins(60,0,60, 0);

        auto label_size = QSize(100, 35);
        auto edit_size = QSize(200, 35);

        // 0. name
        {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("Name"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto edit = new QLineEdit(this);
            ed_name_ = edit;
            if (stream_item_.IsValid()) {
                ed_name_->setText(stream_item_.stream_name.c_str());
            }
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            root_layout->addSpacing(40);
            root_layout->addLayout(layout);

        }

        // 1. host
        {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("Host *"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto edit = new QLineEdit(this);
            ed_host_ = edit;
            if (stream_item_.IsValid()) {
                ed_host_->setText(stream_item_.stream_host.c_str());
            }
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        // 2. port
        {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("Port *"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto edit = new QLineEdit(this);
            auto validator = new QIntValidator(this);
            edit->setValidator(validator);
            ed_port_ = edit;
            ed_port_->setText("20371");
            if (stream_item_.IsValid()) {
                ed_port_->setText(QString::number(stream_item_.stream_port));
            }
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        // 3. bitrate
        if (0) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("Bitrate(M)"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto edit = new QLineEdit(this);
            auto validator = new QIntValidator(this);
            edit->setValidator(validator);
            ed_bitrate_ = edit;
            if (stream_item_.IsValid()) {
                ed_bitrate_->setText(QString::number(stream_item_.encode_bps));
            }
            else {
                ed_bitrate_->setText("5");
            }
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        if (0) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("FPS"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto cb = new QComboBox(this);
            cb_fps_ = cb;
            cb->addItem("15");
            cb->addItem("30");
            cb->addItem("60");
            cb->setFixedSize(edit_size);
            layout->addWidget(cb);
            layout->addStretch();

            if (stream_item_.IsValid()) {
                if (stream_item_.encode_fps == 15) {
                    cb_fps_->setCurrentIndex(0);
                }
                else if (stream_item_.encode_fps == 30) {
                    cb_fps_->setCurrentIndex(1);
                }
                else if (stream_item_.encode_fps == 60) {
                    cb_fps_->setCurrentIndex(2);
                }
            }

            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        // sure or cancel
        {
            auto btn_size = QSize(100, 35);
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            auto btn_cancel = new QPushButton(tr("Cancel"));
            connect(btn_cancel, &QPushButton::clicked, this, [=, this]() {
                done(0);
            });
            btn_cancel->setFixedSize(btn_size);
            layout->addStretch();
            layout->addWidget(btn_cancel);
            layout->addStretch();

            auto btn_sure = new QPushButton(tr("Sure"));
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                if (!GenStream()) {
                    return;
                }
                done(0);
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(btn_size);
            layout->addStretch();

            root_layout->addStretch();
            root_layout->addLayout(layout);
        }
        root_layout->addSpacing(30);

        setLayout(root_layout);
    }

    bool CreateStreamDialog::GenStream() {
        auto host = ed_host_->text().toStdString();
        auto port = std::atoi(ed_port_->text().toStdString().c_str());
        auto name = ed_name_->text().toStdString();
        auto bitrate = [this]() -> int {
            if (ed_bitrate_) {
                return std::atoi(ed_bitrate_->text().toStdString().c_str());
            } else {
                return 0;
            }
        } ();

        if (host.empty() || port == 0) {
            auto dialog = MessageDialog::Make(context_, tr("Please input necessary information !"));
            dialog->exec();
            return false;
        }

        auto func_update_stream = [&](StreamItem& item) {
            item.stream_name = name;
            item.stream_host = host;
            item.stream_port = port;
            item.encode_bps = bitrate;
            item.encode_fps = [=, this]() -> int {
                if (cb_fps_) {
                    return std::atoi(cb_fps_->currentText().toStdString().c_str());
                } else {
                    return 0;
                }
            }();
        };

        if (stream_item_.IsValid()) {
            func_update_stream(stream_item_);
            context_->SendAppMessage(StreamItemUpdated {
                .item_ = stream_item_,
            });
        }
        else {
            StreamItem item;
            func_update_stream(item);
            context_->SendAppMessage(StreamItemAdded {
                .item_ = item,
            });
        }
        return true;
    }

    void CreateStreamDialog::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(0xffffff)));
        painter.drawRect(0, 0, this->width(), this->height());
    }

}
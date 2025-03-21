//
// Created by RGAA on 2023-08-18.
//

#include "create_stream_dialog.h"
#include "client/ct_app_message.h"
#include "client/ct_client_context.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include "message_dialog.h"

namespace tc
{

    CreateStreamDialog::CreateStreamDialog(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : QDialog(parent) {
        context_ = ctx;
        setFixedSize(640, 350);
        CreateLayout();
    }

    CreateStreamDialog::CreateStreamDialog(const std::shared_ptr<ClientContext>& ctx, const StreamItem& item, QWidget* parent) : QDialog(parent) {
        context_ = ctx;
        stream_item_ = item;
        setFixedSize(640, 350);
        CreateLayout();
    }

    CreateStreamDialog::~CreateStreamDialog() = default;

    void CreateStreamDialog::CreateLayout() {
        setWindowTitle(tr("Device Connection Information"));
        auto root_layout = new QVBoxLayout();
        root_layout->setSpacing(0);
        root_layout->setContentsMargins(100,0,60, 0);

        auto label_size = QSize(150, 35);
        auto edit_size = QSize(250, 35);

        // 0. name
        {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            //layout->addStretch();

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
            //layout->addStretch();

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
            //layout->addStretch();

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

        // Remote device id
        if (false) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("Remote Device ID *"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto edit = new QLineEdit(this);
            ed_remote_device_id_ = edit;
            ed_remote_device_id_->setText("");
            if (stream_item_.IsValid()) {
                ed_remote_device_id_->setText(QString::fromStdString(stream_item_.remote_device_id_));
            }
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            root_layout->addSpacing(10);
            root_layout->addLayout(layout);
        }

        if (false) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            //layout->addStretch();

            auto label = new QLabel(this);
            label->setFixedSize(label_size);
            label->setText(tr("Network *"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto group = new QButtonGroup(this);
            auto btn_ws = new QRadioButton(this);
            btn_ws->setText(tr("WebSocket"));
            rb_ws_ = btn_ws;
            layout->addWidget(btn_ws);
            group->addButton(btn_ws);

//            auto btn_udp_kcp = new QRadioButton(this);
//            btn_udp_kcp->setText(tr("UDP"));
//            rb_udp_ = btn_udp_kcp;
//            layout->addSpacing(15);
//            layout->addWidget(btn_udp_kcp);
//            group->addButton(btn_udp_kcp);

            auto btn_relay = new QRadioButton(this);
            btn_relay->setText(tr("Relay"));
            rb_relay_ = btn_relay;
            layout->addSpacing(15);
            layout->addWidget(btn_relay);
            group->addButton(btn_relay);

            layout->addStretch();

            root_layout->addSpacing(10);
            root_layout->addLayout(layout);

            //
            if (stream_item_.IsValid()) {
                if (stream_item_.network_type_ == kStreamItemNtTypeWebSocket) {
                    btn_ws->setChecked(true);
                }
                else if (stream_item_.network_type_ == kStreamItemNtTypeRelay) {
                    btn_relay->setChecked(true);
                }
//                else if (stream_item_.network_type_ == kStreamItemNtTypeUdpKcp) {
//                    btn_udp_kcp->setChecked(true);
//                }
            }
            else {
                btn_ws->setChecked(true);
                connect(btn_ws, &QRadioButton::clicked, this, [=, this](bool checked) {
                    ed_port_->setText("20371");
                });
                connect(btn_relay, &QRadioButton::clicked, this, [=, this](bool checked) {
                    ed_port_->setText("20481");
                });
            }
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
            auto btn_size = QSize(125, 35);
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            auto btn_cancel = new QPushButton(tr("Cancel"));
            btn_cancel->setProperty("class", "danger");
            connect(btn_cancel, &QPushButton::clicked, this, [=, this]() {
                done(0);
            });
            btn_cancel->setFixedSize(btn_size);
            layout->addStretch();
            layout->addWidget(btn_cancel);
            layout->addSpacing(20);

            auto btn_sure = new QPushButton(tr("OK"));
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
        //auto remote_device_id = ed_remote_device_id_ ? ed_remote_device_id_->text().trimmed().replace(" ", "").toStdString() : "";

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
            item.encode_fps = cb_fps_ ? std::atoi(cb_fps_->currentText().toStdString().c_str()) : 0;
//            item.remote_device_id_ = remote_device_id;
            item.network_type_ = [=, this]() -> std::string {
//                if (rb_ws_->isChecked()) {
//                    return kStreamItemNtTypeWebSocket;
//                }
//                else if (rb_udp_ && rb_udp_->isChecked()) {
//                    return kStreamItemNtTypeUdpKcp;
//                }
//                else if (rb_relay_ && rb_relay_->isChecked()) {
//                    return kStreamItemNtTypeRelay;
//                }
                return kStreamItemNtTypeWebSocket;
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
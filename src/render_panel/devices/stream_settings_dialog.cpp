//
// Created by RGAA on 2023-08-18.
//

#include "stream_settings_dialog.h"
#include "client/ct_app_message.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_dialog.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "stream_db_manager.h"

namespace tc
{

    StreamSettingsDialog::StreamSettingsDialog(const std::shared_ptr<GrContext>& ctx, const StreamItem& item, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        db_mgr_ = context_->GetStreamDBManager();
        stream_item_ = item;
        setFixedSize(375, 475);
        CreateLayout();
    }

    StreamSettingsDialog::~StreamSettingsDialog() = default;

    void StreamSettingsDialog::CreateLayout() {
        setWindowTitle(tr("Device Settings"));

        auto item_width = 150;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        //root_layout->addStretch();
        root_layout->addSpacing(40);
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(10);

        // audio
        {
            auto layout = new NoMarginHLayout();

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Enable Audio");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto cb = new QCheckBox(this);
            cb->setChecked(stream_item_.audio_enabled_);
            cb_audio_ = cb;
            layout->addWidget(cb);
            layout->addStretch();
            content_layout->addLayout(layout);

        }
        content_layout->addSpacing(5);

        // clipboard
        {
            auto layout = new NoMarginHLayout();

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Enable Clipboard");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto cb = new QCheckBox(this);
            cb->setChecked(stream_item_.clipboard_enabled_);
            cb_clipboard_ = cb;
            layout->addWidget(cb);
            layout->addStretch();
            content_layout->addLayout(layout);

        }
        content_layout->addSpacing(5);

        // only viewing
        {
            auto layout = new NoMarginHLayout();

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Only Viewing");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto cb = new QCheckBox(this);
            cb->setChecked(stream_item_.only_viewing_);
            cb_only_viewing_ = cb;
            layout->addWidget(cb);
            layout->addStretch();
            content_layout->addLayout(layout);

        }
        // Remote device id
        if (false) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);

            auto label = new QLabel(this);
            //label->setFixedSize(label_size);
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

            content_layout->addSpacing(10);
            content_layout->addLayout(layout);
        }

        if (false) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            //layout->addStretch();

            auto label = new QLabel(this);
            //label->setFixedSize(label_size);
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

            content_layout->addSpacing(10);
            content_layout->addLayout(layout);

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
            }
        }

        // 3. bitrate
        if (0) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            //label->setFixedSize(label_size);
            label->setText(tr("Bitrate(M)"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);

            auto edit = new QLineEdit(this);
            auto validator = new QIntValidator(this);
            edit->setValidator(validator);
            ed_bitrate_ = edit;
            if (stream_item_.IsValid()) {
                ed_bitrate_->setText(QString::number(stream_item_.encode_bps_));
            }
            else {
                ed_bitrate_->setText("5");
            }
            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            content_layout->addSpacing(10);
            content_layout->addLayout(layout);
        }

        if (0) {
            auto layout = new QHBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
            layout->addStretch();

            auto label = new QLabel(this);
            //label->setFixedSize(label_size);
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
                if (stream_item_.encode_fps_ == 15) {
                    cb_fps_->setCurrentIndex(0);
                }
                else if (stream_item_.encode_fps_ == 30) {
                    cb_fps_->setCurrentIndex(1);
                }
                else if (stream_item_.encode_fps_ == 60) {
                    cb_fps_->setCurrentIndex(2);
                }
            }

            content_layout->addSpacing(10);
            content_layout->addLayout(layout);
        }

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new QPushButton(tr("OK"));
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                stream_item_.audio_enabled_ = cb_audio_->isChecked();
                stream_item_.clipboard_enabled_ = cb_clipboard_->isChecked();
                stream_item_.only_viewing_ = cb_only_viewing_->isChecked();
                db_mgr_->UpdateStream(stream_item_);
                this->close();
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(320, 35));

            content_layout->addSpacing(105);
            content_layout->addStretch();
            content_layout->addLayout(layout);
            content_layout->addSpacing(30);
        }

        root_layout_->addStretch();
    }

    void StreamSettingsDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
//        QPainter painter(this);
//        painter.setPen(Qt::NoPen);
//        painter.setBrush(QBrush(QColor(0xffffff)));
//        painter.drawRect(0, 0, this->width(), this->height());
    }

}
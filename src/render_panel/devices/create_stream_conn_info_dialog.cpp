//
// Created by RGAA on 2023-08-18.
//

#include "create_stream_conn_info_dialog.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include "tc_dialog.h"
#include "render_panel/database/stream_item.h"
#include "client/ct_app_message.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/util/conn_info_parser.h"
#include "tc_common_new/log.h"
#include "tc_common_new/http_client.h"
#include "tc_spvr_client/spvr_device_info.h"

namespace tc
{

    CreateStreamConnInfoDialog::CreateStreamConnInfoDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 475);
        CreateLayout();
    }

    CreateStreamConnInfoDialog::~CreateStreamConnInfoDialog() = default;

    void CreateStreamConnInfoDialog::CreateLayout() {
        setWindowTitle(tr("Create a Stream"));

        auto item_width = 320;
        auto edit_size = QSize(item_width, 35);

        auto root_layout = new NoMarginHLayout();
        auto content_layout = new NoMarginVLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();
        root_layout_->addLayout(root_layout);

        content_layout->addSpacing(25);

        // 0. name
        {
            auto layout = new NoMarginVLayout();

            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setText("Device Name");
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QLineEdit(this);
            ed_device_name_ = edit;

            edit->setFixedSize(edit_size);
            layout->addWidget(edit);
            layout->addStretch();

            content_layout->addLayout(layout);

        }

        content_layout->addSpacing(25);

        // 1. host
        {
            auto layout = new NoMarginVLayout();
            auto label = new QLabel(this);
            label->setFixedWidth(item_width);
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->setText(tr("Connection Info *"));
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            layout->addWidget(label);
            layout->addSpacing(10);

            auto edit = new QTextEdit(this);
            ed_conn_info_ = edit;
            edit->setFixedSize(edit_size.width(), 200);
            layout->addWidget(edit);
            layout->addStretch();
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(25);

        // sure button
        {
            auto layout = new NoMarginVLayout();
            auto btn_sure = new QPushButton(tr("OK"));
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                if (!GenStream()) {
                    return;
                }
                this->close();
            });

            layout->addWidget(btn_sure);
            btn_sure->setFixedSize(QSize(item_width, 35));

            content_layout->addSpacing(105);
            content_layout->addLayout(layout);
        }

        content_layout->addSpacing(30);
        root_layout_->addStretch();
    }

    bool CreateStreamConnInfoDialog::GenStream() {
        auto info = ed_conn_info_->toPlainText().toStdString();
        auto name = ed_device_name_->text().toStdString();

        auto conn_info = ConnInfoParser::Parse(info);
        if (!conn_info) {
            TcDialog dialog(tr("Tips"), tr("Parse address failed, please check the address."), this);
            dialog.exec();
            return false;
        }

        auto fn_invalid_dialog = [this]() {
            TcDialog dialog(tr("Tips"), tr("Address is invalid, you may check it or input Host&Port directly."), this);
            dialog.exec();
        };

        if (!conn_info->IsValid()) {
            fn_invalid_dialog();
            return false;
        }

        // check a valid one
        GrConnectionInfo::GrConnectionHost conn_host{};
        bool has_device_id = !conn_info->device_id_.empty();
        // websocket mode: remote device host
        // relay mode: relay server ip
        std::string stream_host;
        // websocket mode: remote device port
        // relay mode: relay server port
        int stream_port = 0;

        if (!has_device_id) {
            if (conn_info->hosts_.size() > 1) {
                for (const auto &host: conn_info->hosts_) {
                    auto client = HttpClient::Make(host.ip_, conn_info->render_srv_port_, "/api/ping", 500);
                    auto r = client->Request();
                    if (r.status == 200) {
                        conn_host = host;
                        LOGI("THIS Worked: {}:{}", host.ip_, conn_info->render_srv_port_);
                        break;
                    }
                }
            } else {
                conn_host = conn_info->hosts_[0];
            }

            if (conn_host.ip_.empty()) {
                fn_invalid_dialog();
                return false;
            }

            // websocket mode
            stream_host = conn_host.ip_;
            stream_port = conn_info->render_srv_port_;
        }
        else {
            auto relay_device_info = context_->GetRelayServerSideDeviceInfo(conn_info->device_id_);
            if (relay_device_info == nullptr) {
                return false;
            }

            // relay mode
            stream_host = relay_device_info->relay_server_ip_;
            stream_port = relay_device_info->relay_server_port_;
        }

        auto settings = Settings::Instance();
        auto func_update_stream = [&](std::shared_ptr<StreamItem>& item) {
            item->remote_device_id_ = conn_info->device_id_;
            item->remote_device_random_pwd_ = conn_info->random_pwd_;
            item->stream_name_ = name.empty() ? (item->remote_device_id_.empty() ? std::format("{}", conn_host.ip_) : item->remote_device_id_) : name;
            item->stream_host_ = stream_host;
            item->stream_port_ = stream_port;
            item->encode_bps_ = 0;
            item->encode_fps_ = 0;
            item->network_type_ = has_device_id ? kStreamItemNtTypeRelay : kStreamItemNtTypeWebSocket;
        };

        std::shared_ptr<StreamItem> item = std::make_shared<StreamItem>();
        func_update_stream(item);
        context_->SendAppMessage(StreamItemAdded {
            .item_ = item,
            .auto_start_ = true,
        });
        return true;
    }

    void CreateStreamConnInfoDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

}
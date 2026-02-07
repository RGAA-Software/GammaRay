//
// Created by RGAA on 2023-08-18.
//

#include "create_stream_conn_info_dialog.h"
#include <QValidator>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "tc_spvr_client/spvr_stream.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/gr_application.h"
#include "render_panel/user/gr_user_manager.h"
#include "render_panel/util/conn_info_parser.h"
#include "tc_common_new/log.h"
#include "tc_common_new/http_client.h"
#include "relay_message.pb.h"
#include "client/ct_stream_item_net_type.h"
#include "infinite_loading.h"

namespace tc
{

    CreateStreamConnInfoDialog::CreateStreamConnInfoDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcCustomTitleBarDialog("", parent) {
        context_ = ctx;
        setFixedSize(375, 475);
        CreateLayout();
    }

    CreateStreamConnInfoDialog::~CreateStreamConnInfoDialog() = default;

    void CreateStreamConnInfoDialog::CreateLayout() {
        setWindowTitle(tcTr("id_create_a_stream"));

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

            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->SetTextId("id_device_name");
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
            auto label = new TcLabel(this);
            label->setFixedWidth(item_width);
            label->setStyleSheet(R"(color: #333333; font-weight: 700; font-size:13px;)");
            label->SetTextId("id_connection_info");
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
            auto btn_sure = new TcPushButton();
            btn_sure->SetTextId("id_ok");
            connect(btn_sure, &QPushButton::clicked, this, [=, this] () {
                GenStream();
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
        // loading dialog
        auto dialog = std::make_shared<InfiniteLoading>(context_, tcTr("id_verifying"));
        dialog->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        dialog->show();

        auto info = ed_conn_info_->toPlainText().toStdString();
        auto name = ed_device_name_->text().toStdString();

        auto conn_info = ConnInfoParser::Parse(info);
        if (!conn_info) {
            TcDialog dialog(tcTr("id_error"), tcTr("id_parse_conn_info_failed"), this);
            dialog.exec();
            return false;
        }

        LOGI("ConnInfo: {}", conn_info->Dump());

        auto fn_invalid_dialog = [this]() {
            TcDialog dialog(tcTr("id_error"), tcTr("id_verify_conn_info_failed"), this);
            dialog.exec();
        };

        if (!conn_info->IsValid()) {
            fn_invalid_dialog();
            return false;
        }

        // check a valid one
        bool has_device_id = !conn_info->device_id_.empty();

        context_->PostTask([=, this]() {
            std::string direct_host;
            int direct_port = 0;
            bool direct_online = false;
            //
            if (!conn_info->hosts_.empty()) {
                for (const auto &host: conn_info->hosts_) {
                    LOGI("Check connecting directly => http://{}:{}/api/ping", host.ip_, conn_info->render_srv_port_);
                    auto client = HttpClient::Make(host.ip_, conn_info->render_srv_port_, "/api/ping", 2000);
                    auto r = client->Request();
                    if (r.status == 200) {
                        direct_online = true;
                        direct_host = host.ip_;
                        direct_port = conn_info->render_srv_port_;
                        LOGI("THIS Worked: {}:{}", host.ip_, conn_info->render_srv_port_);
                        break;
                    }
                    else {
                        LOGI("Test connection for adding stream, can't connect: {}:{}", direct_host, direct_port);
                    }
                }
            }

            std::string relay_host;
            int relay_port = 0;
            std::string relay_appkey;

            if (has_device_id) {
                relay_host = conn_info->relay_host_;
                relay_port = conn_info->relay_port_;
                relay_appkey = conn_info->relay_appkey_;
                if (relay_host.empty() || relay_port <= 0 || relay_appkey.empty()) {
                    context_->PostUITask([this]() {
                        TcDialog dialog(tcTr("id_error"), tcTr("id_dont_have_server_config"), this);
                        dialog.exec();
                    });
                }
            }

            // this is good
            std::shared_ptr<spvr::SpvrStream> item = std::make_shared<spvr::SpvrStream>();
            item->remote_device_id_ = conn_info->device_id_;
            item->remote_device_random_pwd_ = conn_info->random_pwd_;
            item->stream_name_ = name.empty() ? conn_info->device_name_ : name;
            item->stream_host_ = direct_host;
            item->stream_port_ = direct_port;
            item->relay_host_ = relay_host;
            item->relay_port_ = relay_port;
            //item->relay_appkey_ = relay_appkey;
            item->encode_bps_ = 0;
            item->encode_fps_ = 0;
            item->clipboard_enabled_ = true;
            item->direct_online_ = direct_online;
            context_->SendAppMessage(StreamItemAdded {
                .item_ = item,
                .auto_start_ = true,
            });

            if (const auto remote_device_id = item->remote_device_id_; !remote_device_id.empty()) {
                context_->PostTask([=]() {
                    const auto user_mgr = grApp->GetUserManager();
                    user_mgr->AddDeviceForUser(remote_device_id);
                });
            }
            context_->PostUIDelayTask([=, this]() {
                dialog->hide();
                dialog->Close() ;
                this->hide();
                this->close();
            }, 50);

        });

        return true;
    }

    void CreateStreamConnInfoDialog::paintEvent(QPaintEvent *event) {
        TcCustomTitleBarDialog::paintEvent(event);
    }

}
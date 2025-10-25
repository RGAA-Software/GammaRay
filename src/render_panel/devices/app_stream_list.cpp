//
// Created by RGAA on 2023/8/14.
//

#include "app_stream_list.h"

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QtWidgets/QMenu>
#include <QWidget>
#include <QProcess>

#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "widget_helper.h"
#include "stream_messages.h"
#include "stream_item_widget.h"
#include "create_stream_dialog.h"
#include "stream_content.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_app_messages.h"
#include "running_stream_manager.h"
#include "tc_common_new/uid_spacer.h"
#include "tc_common_new/hardware.h"
#include "edit_relay_stream_dialog.h"
#include "stream_settings_dialog.h"
#include "start_stream_loading.h"
#include "input_remote_pwd_dialog.h"
#include "stream_state_checker.h"
#include "tc_profile_client/profile_api.h"
#include "tc_relay_client/relay_api.h"
#include "render_panel/gr_account_manager.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_workspace.h"
#include "render_panel/network/render_api.h"
#include "render_panel/database/stream_db_operator.h"

namespace tc
{

    class MainItemDelegate : public QStyledItemDelegate {
    public:
        explicit MainItemDelegate(QObject* pParent) {}
        ~MainItemDelegate() override = default;

        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            editor->setGeometry(option.rect);
        }
    };

    // - - -- - - -- - - - -- -

    AppStreamList::AppStreamList(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
        db_mgr_ = context_->GetStreamDBManager();
        stream_content_ = (StreamContent*)parent;
        running_stream_mgr_ = context_->GetRunningStreamManager();
        CreateLayout();
        Init();

        setStyleSheet("background-color: #ffffff;");

        //
        state_checker_ = std::make_shared<StreamStateChecker>(context_);
        state_checker_->SetOnCheckedCallback([=, this](const std::vector<std::shared_ptr<StreamItem>>& stream_items) {
            context_->PostUITask([=, this]() {
                int count = stream_list_->count();
                for (int i = 0; i < count; i++) {
                    auto item = stream_list_->item(i);
                    auto widget = (StreamItemWidget*)stream_list_->itemWidget(item);
                    auto stream_id_for_widget = widget->GetStreamId();
                    for (const auto& update_item : stream_items) {
                        if (update_item->stream_id_ == stream_id_for_widget) {
                            widget->SetConnectedState(update_item->online_);
                            break;
                        }
                    }
                }
            });
        });
        state_checker_->Start();
        context_->PostUIDelayTask([=, this]() {
            context_->PostTask([=, this]() {
                state_checker_->UpdateCurrentStreamItems(streams_);
            });
        }, 2200);
    }

    AppStreamList::~AppStreamList() = default;

    void AppStreamList::CreateLayout() {
        auto root_layout = new QHBoxLayout();
        WidgetHelper::ClearMargins(root_layout);

        auto delegate = new MainItemDelegate(this);
        stream_list_ = new QListWidget(this);
        stream_list_->setItemDelegate(delegate);

        stream_list_->setMovement(QListView::Static);
        stream_list_->setViewMode(QListView::IconMode);
        stream_list_->setFlow(QListView::LeftToRight);
        stream_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stream_list_->setResizeMode(QListWidget::Adjust);
        stream_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        stream_list_->setSpacing(15);
        stream_list_->setStyleSheet(R"(
            QListWidget::item {
                color: #000000;
                border: transparent;
                border-bottom: 0px solid #dbdbdb;
            }

            QListWidget::item:hover {
                background-color: none;
            }

            QListWidget::item:selected {
                border-left: 0px solid #777777;
                background-color: none;
            }
        )");

        connect(stream_list_, &QListWidget::customContextMenuRequested, this, [=, this](const QPoint& pos) {
            QListWidgetItem* cur_item = stream_list_->itemAt(pos);
            if (cur_item == nullptr) { return; }
            int index = stream_list_->row(cur_item);
            RegisterActions(index);
        });

        connect(stream_list_, &QListWidget::itemDoubleClicked, this, [=, this](QListWidgetItem *item) {
            int index = stream_list_->row(item);
            auto stream_item = streams_.at(index);
            StartStream(stream_item, false);
        });

        root_layout->addSpacing(10);
        root_layout->addWidget(stream_list_);
        root_layout->addSpacing(10);

        setLayout(root_layout);
    }

    void AppStreamList::Init() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<StreamItemAdded>([=, this](const StreamItemAdded& msg) {
            auto item = msg.item_;
            std::shared_ptr<StreamItem> exist_stream_item = nullptr;
            // by stream id
            {
                auto opt_stream = db_mgr_->GetStreamByStreamId(item->stream_id_);
                if (opt_stream.has_value()) {
                    exist_stream_item = opt_stream.value();
                }
            }

            if (!item->remote_device_id_.empty()) {
                // by remote device id
                auto opt_stream = db_mgr_->GetStreamByRemoteDeviceId(item->remote_device_id_);
                if (opt_stream.has_value()) {
                    exist_stream_item = opt_stream.value();
                }
            }
            else {
                // by host & port
                auto opt_stream = db_mgr_->GetStreamByHostPort(item->stream_host_, item->stream_port_);
                if (opt_stream.has_value()) {
                    exist_stream_item = opt_stream.value();
                }
            }
            if (!exist_stream_item) {
                db_mgr_->AddStream(item);
                exist_stream_item = item;
            }
            else {
                // todo: Check stream info.
                // check password type: random / safety
                // then update it in database
                if (!item->remote_device_id_.empty()) {
                    exist_stream_item->remote_device_id_ = item->remote_device_id_;
                }
                exist_stream_item->stream_host_ = item->stream_host_;
                exist_stream_item->stream_port_ = item->stream_port_;
                if (exist_stream_item->remote_device_random_pwd_ != item->remote_device_random_pwd_ && !item->remote_device_random_pwd_.empty()) {
                    exist_stream_item->remote_device_random_pwd_ = item->remote_device_random_pwd_;
                }
                if (exist_stream_item->remote_device_safety_pwd_ != item->remote_device_safety_pwd_ && !item->remote_device_safety_pwd_.empty()) {
                    exist_stream_item->remote_device_safety_pwd_ = item->remote_device_safety_pwd_;
                }
                db_mgr_->UpdateStream(exist_stream_item);
            }
            LoadStreamItems();

            LOGI("Auto start stream: {}", msg.auto_start_);
            context_->PostUITask([=, this]() {
                if (msg.auto_start_) {
                    StartStream(exist_stream_item, false);
                }
            });
        });

        msg_listener_->Listen<StreamItemUpdated>([=, this](const StreamItemUpdated& msg) {
            db_mgr_->UpdateStream(msg.item_);
            LoadStreamItems();
            LOGI("Update stream : {}", msg.item_->stream_id_);
        });

        msg_listener_->Listen<MsgRemotePeerInfo>([=, this](const MsgRemotePeerInfo& msg) {
            for (auto& stream : streams_) {
                if (stream->stream_id_ == msg.stream_id_) {
                    if (stream->desktop_name_ != msg.desktop_name_ || stream->os_version_ != msg.os_version_) {
                        // update it
                        stream->desktop_name_ = msg.desktop_name_;
                        stream->os_version_ = msg.os_version_;
                        db_mgr_->UpdateStream(stream);

                        context_->PostUITask([=, this]() {

                        });
                    }
                    break;
                }
            }
        });

        msg_listener_->Listen<MsgClientConnectedPanel>([=, this](const MsgClientConnectedPanel& msg) {

        });

        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            context_->PostTask([this]() {
                state_checker_->UpdateCurrentStreamItems(streams_);
            });
        });
    }

    void AppStreamList::RegisterActions(int index) {
        std::vector<QString> actions = {
            tcTr("id_start_control"),
            tcTr("id_stop_control"),
            tcTr("id_only_viewing"),
            tcTr("id_lock_device"),
            tcTr("id_restart_device"),
            tcTr("id_shutdown_device"),
            "",
            tcTr("id_edit"),
            tcTr("id_delete"),
            "",
            tcTr("id_settings"),
        };
        auto menu = new QMenu();
        for (int i = 0; i < actions.size(); i++) {
            const QString& action_name = actions.at(i);
            if (action_name.isEmpty()) {
                menu->addSeparator();
                continue;
            }

            auto action = new QAction(action_name, menu);
            menu->addAction(action);
            connect(action, &QAction::triggered, this, [=, this]() {
                ProcessAction(i, streams_.at(index));
            });
        }
        menu->exec(QCursor::pos());
        delete menu;
    }

    void AppStreamList::ProcessAction(int index, const std::shared_ptr<StreamItem>& item) {
        if (index == 0) {
            // connect
            StartStream(item, false);
        }
        else if (index == 1) {
            // stop
            StopStream(item);
        }
        else if (index == 2) {
            // only viewing
            StartStream(item, true);
        }
        else if (index == 3) {
            // lock device
            LockDevice(item);
        }
        else if (index == 4) {
            // restart device
            RestartDevice(item);
        }
        else if (index == 5) {
            // shutdown device
            ShutdownDevice(item);
        }
        // "" 6
        else if (index == 7) {
            // edit
            EditStream(item);
        }
        else if (index == 8) {
            // delete
            DeleteStream(item);
        }
        // "" 9
        else if (index == 10) {
            ShowSettings(item);
        }
    }

    void AppStreamList::StartStream(const std::shared_ptr<StreamItem>& item, bool force_only_viewing) {
        auto si = db_mgr_->GetStreamByStreamId(item->stream_id_);
        if (!si.has_value()) {
            LOGE("read stream item from db failed: {}", item->stream_id_);
            return;
        }

        const auto& target_item = si.value();
        // may start with [Only Viewing]
        if (force_only_viewing) {
            target_item->only_viewing_ = true;
        }

        // verify in profile server
        if (target_item->IsRelay()) {
            // verify my self
            if (!grApp->CheckLocalDeviceInfoWithPopup()) {
                return;
            }

            // check the remote device in relay server
            auto appkey = grApp->GetAppkey();
            auto srv_remote_device_id = "server_" + item->remote_device_id_;
            LOGI("Will check remote device: {} on relay server: {}:{}", item->remote_device_id_, item->stream_host_, item->stream_port_);
            auto relay_device_info = relay::RelayApi::GetRelayDeviceInfo(item->stream_host_, item->stream_port_, srv_remote_device_id, appkey);
            if (!relay_device_info.has_value()) {
                if (relay_device_info.error() == relay::kRelayRequestFailed) {
                    // network failed
                    TcDialog dialog(tcTr("id_error"), tcTr("id_relay_network_unavailable_recreate"));
                    dialog.exec();
                }
                else {
                    //
                    TcDialog dialog(tcTr("id_error"), tcTr("id_cant_get_remote_device_info"));
                    dialog.exec();
                }
                return;
            }

            // NO password, just input one
            QString input_password;
            if (target_item->remote_device_random_pwd_.empty() && target_item->remote_device_safety_pwd_.empty()) {
                InputRemotePwdDialog dlg_input_pwd(context_);
                if (dlg_input_pwd.exec() == 1) {
                    return;
                }
                input_password = dlg_input_pwd.GetInputPassword();
                if (input_password.isEmpty()) {
                    return;
                }
            }

            auto remote_random_pwd = target_item->remote_device_random_pwd_;
            auto remote_safety_pwd = target_item->remote_device_safety_pwd_;
            if (!input_password.isEmpty() && remote_random_pwd.empty() && remote_safety_pwd.empty()) {
                remote_random_pwd = input_password.toStdString();
                remote_safety_pwd = input_password.toStdString();
            }

            // verify remote
            // password from inputting
            // password from database
            auto verify_result = ProfileApi::VerifyDeviceInfo(settings_->GetSpvrServerHost(),
                                                              settings_->GetSpvrServerPort(),
                                                              target_item->remote_device_id_,
                                                              MD5::Hex(remote_random_pwd),
                                                              MD5::Hex(remote_safety_pwd),
                                                              grApp->GetAppkey());
            if (verify_result == ProfileVerifyResult::kVfNetworkFailed) {
                TcDialog dialog(tcTr("id_connect_failed"), tcTr("id_profile_network_unavailable"), grWorkspace.get());
                dialog.exec();
                return;
            }

            if (verify_result != ProfileVerifyResult::kVfSuccessRandomPwd &&
                verify_result != ProfileVerifyResult::kVfSuccessSafetyPwd &&
                verify_result != ProfileVerifyResult::kVfSuccessAllPwd) {
                // tell user, password is invalid
                TcDialog dialog(tcTr("id_password_invalid"), tcTr("id_password_invalid_msg"), grWorkspace.get());
                dialog.exec();

                // clear the password and restart stream, then you need to input a password
                // clear the memory
                item->remote_device_random_pwd_ = "";
                item->remote_device_safety_pwd_ = "";
                // clear the database
                db_mgr_->UpdateStreamRandomPwd(target_item->stream_id_, "");
                db_mgr_->UpdateStreamSafetyPwd(target_item->stream_id_, "");
                context_->PostUIDelayTask([=, this]() {
                    StartStream(item, false);
                }, 100);
                return;
            }

            LOGI("Verify result, the password type: {}", (int)verify_result);
            // update to database
            if (verify_result == ProfileVerifyResult::kVfSuccessRandomPwd || verify_result == ProfileVerifyResult::kVfSuccessAllPwd) {
                db_mgr_->UpdateStreamRandomPwd(target_item->stream_id_, remote_random_pwd);
                target_item->remote_device_random_pwd_ = remote_random_pwd;
            }
            else if (verify_result == ProfileVerifyResult::kVfSuccessSafetyPwd || verify_result == ProfileVerifyResult::kVfSuccessAllPwd) {
                db_mgr_->UpdateStreamSafetyPwd(target_item->stream_id_, remote_safety_pwd);
                target_item->remote_device_safety_pwd_ = remote_safety_pwd;
            }
        }
        else {
            // get render configuration; to check the render online or not
            {
                auto r = RenderApi::GetRenderConfiguration(target_item->stream_host_, target_item->stream_port_);
                if (!r.has_value()) {
                    TcDialog dialog(tcTr("id_error"), tcTr("id_cant_get_remote_device_info"), grWorkspace.get());
                    dialog.exec();
                    return;
                }
            }

            // verify security password
            if (!target_item->remote_device_safety_pwd_.empty()) {
                auto r = RenderApi::VerifySecurityPassword(target_item->stream_host_, target_item->stream_port_, target_item->remote_device_safety_pwd_);
                auto ok = r.value_or(false);
                for (;;) {
                    LOGI("VerifySecurityPassword result: {}", ok);
                    if (!ok) {
                        LOGI("VerifySecurityPassword result 1: {}", ok);
                        InputRemotePwdDialog dlg_input_pwd(context_);
                        if (dlg_input_pwd.exec() == 1) {
                            return;
                        }
                        auto input_password = dlg_input_pwd.GetInputPassword();
                        if (input_password.isEmpty()) {
                            context_->NotifyAppErrMessage(tcTr("id_error"), tcTr("id_input_necessary_info"));
                            continue;
                        }

                        // md5 pwd
                        auto pwd_md5 = MD5::Hex(input_password.toStdString());

                        r = RenderApi::VerifySecurityPassword(target_item->stream_host_, target_item->stream_port_,
                                                              pwd_md5);
                        ok = r.value_or(false);
                        if (!ok) {
                            context_->NotifyAppErrMessage(tcTr("id_error"), tcTr("id_password_invalid_msg"));
                        } else {
                            // update to database
                            context_->PostDBTask([=, this]() {
                                auto mgr = context_->GetStreamDBManager();
                                mgr->UpdateStreamSafetyPwd(target_item->stream_id_, pwd_md5);
                                LoadStreamItems();
                            });
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        running_stream_mgr_->StartStream(target_item);
    }

    void AppStreamList::StopStream(const std::shared_ptr<StreamItem>& item) {
        auto si = db_mgr_->GetStreamByStreamId(item->stream_id_);
        if (!si.has_value()) {
            LOGE("read stream item from db failed: {}", item->stream_id_);
            return;
        }
        running_stream_mgr_->StopStream(si.value());
    }

    void AppStreamList::LockDevice(const std::shared_ptr<StreamItem>& item) {
        if (!item->online_) {
            context_->NotifyAppErrMessage(tcTr("id_error"), tcTr("id_device_offline"));
            return;
        }

        TcDialog dialog(tcTr("id_warning"), tcTr("id_ask_lock_screen"));
        if (dialog.exec() == kDoneOk) {
            auto msg = std::make_shared<GrSmLockScreen>();
            msg->stream_item_ = item;
            grApp->PostMessage2RemoteRender(msg);
        }
    }

    void AppStreamList::RestartDevice(const std::shared_ptr<StreamItem>& item) {
        if (!item->online_) {
            context_->NotifyAppErrMessage(tcTr("id_error"), tcTr("id_device_offline"));
            return;
        }

        TcDialog dialog(tcTr("id_warning"), tcTr("id_ask_restart_device"));
        if (dialog.exec() == kDoneOk) {
            auto msg = std::make_shared<GrSmRestartDevice>();
            msg->stream_item_ = item;
            grApp->PostMessage2RemoteRender(msg);
        }
    }

    void AppStreamList::ShutdownDevice(const std::shared_ptr<StreamItem>& item) {
        if (!item->online_) {
            context_->NotifyAppErrMessage(tcTr("id_error"), tcTr("id_device_offline"));
            return;
        }

        TcDialog dialog(tcTr("id_warning"), tcTr("id_ask_shutdown_device"));
        if (dialog.exec() == kDoneOk) {
            auto msg = std::make_shared<GrSmShutdownDevice>();
            msg->stream_item_ = item;
            grApp->PostMessage2RemoteRender(msg);
        }
    }

    void AppStreamList::EditStream(const std::shared_ptr<StreamItem>& item) {
        auto si = db_mgr_->GetStreamByStreamId(item->stream_id_);
        if (!si.has_value()) {
            LOGE("read stream item from db failed: {}", item->stream_id_);
            return;
        }
        if (item->IsRelay()) {
            auto dialog = new EditRelayStreamDialog(context_, si.value(), grWorkspace.get());
            dialog->exec();
        }
        else {
            auto dialog = new CreateStreamDialog(context_, si.value(), grWorkspace.get());
            dialog->exec();
        }
    }

    void AppStreamList::DeleteStream(const std::shared_ptr<StreamItem>& item) {
        TcDialog dialog(tcTr("id_warning"), tcTr("id_delete_remote_control"), grWorkspace.get());
        if (dialog.exec() == kDoneOk) {
            // stop it if running
            StopStream(item);
            // delete it from database
            auto mgr = context_->GetStreamDBManager();
            mgr->DeleteStream(item->_id);
            LoadStreamItems();
        }
    }

    void AppStreamList::ShowSettings(const std::shared_ptr<StreamItem>& item) {
        auto si = db_mgr_->GetStreamByStreamId(item->stream_id_);
        if (!si.has_value()) {
            LOGE("read stream item from db failed: {}", item->stream_id_);
            return;
        }
        auto dialog = new StreamSettingsDialog(context_, si.value(), grWorkspace.get());
        dialog->exec();
    }

    QListWidgetItem* AppStreamList::AddItem(const std::shared_ptr<StreamItem>& stream, int index) {
        auto item = new QListWidgetItem(stream_list_);
        item->setSizeHint(QSize(230, 150));
        auto widget = new StreamItemWidget(stream, stream->bg_color_, stream_list_);
        widget->setObjectName(stream->stream_id_.c_str());
        WidgetHelper::AddShadow(widget, 0xbbbbbb, 8);
        widget->SetOnConnectListener([=, this]() {
            StartStream(stream, false);
        });
        widget->SetOnMenuListener([=, this]() {
            RegisterActions(index);
        });

        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(root_layout);
        root_layout->setContentsMargins(2, 0, 2, 0);

        auto layout = new QVBoxLayout();
        layout->addStretch();
        WidgetHelper::ClearMargins(layout);
        root_layout->addLayout(layout);

        auto gap = 0;//5;

        // name
        auto name = new QLabel(stream_list_);
        name->hide();
        name->setObjectName("st_name");
        auto stream_name = stream->stream_name_;
        if (stream->IsRelay()) {
            stream_name = tc::SpaceId(stream_name);
        }
        name->setText(stream_name.c_str());
        name->setStyleSheet(R"(color:#386487; font-size:14px; font-weight:bold; background-color:#909099;)");
        layout->addWidget(name);

        // host
        auto host = new QLabel(stream_list_);
        host->hide();
        host->setObjectName("st_host");
        host->setText(stream->stream_host_.c_str());
        host->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(host);

        //
        auto port = new QLabel(stream_list_);
        port->hide();
        port->setObjectName("st_port");
        port->setText(std::to_string(stream->stream_port_).c_str());
        port->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(port);

        //
        auto bitrate = new QLabel(stream_list_);
        bitrate->hide();
        bitrate->setObjectName("st_bitrate");
        std::string bt_str = std::to_string(stream->encode_bps_) + " Mbps";
        bitrate->setText(bt_str.c_str());
        bitrate->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(bitrate);

        auto fps = new QLabel(stream_list_);
        fps->hide();
        fps->setObjectName("st_fps");
        std::string fps_str = std::to_string(stream->encode_fps_) + " FPS";
        fps->setText(fps_str.c_str());
        fps->setStyleSheet(R"(color:#386487; font-size:14px; )");
        layout->addSpacing(gap);
        layout->addWidget(fps);

        //layout->addStretch();

        root_layout->addLayout(layout);
        //layout->addSpacing(6);
        widget->setLayout(root_layout);
        stream_list_->setItemWidget(item, widget);
        return item;
    }

    QWidget* AppStreamList::GetItemByStreamId(const std::string& stream_id) {
        int count = stream_list_->count();
        for (int i = 0; i < count; i++) {
            auto item = stream_list_->item(i);
            auto widget = stream_list_->itemWidget(item);
            if (widget->objectName().toStdString() == stream_id) {
                return widget;
            }
        }
        return nullptr;
    }

    void AppStreamList::LoadStreamItems() {
        auto db_mgr = context_->GetStreamDBManager();
        streams_ = db_mgr->GetAllStreamsSortByCreatedTime();

        context_->PostUITask([=, this]() {
            int count = stream_list_->count();
            for (int i = 0; i < count; i++) {
                auto item = stream_list_->takeItem(0);
                delete item;
            }

            int index = 0;
            for (auto& stream : streams_) {
                stream->device_id_ = settings_->GetDeviceId();
                AddItem(stream, index++);
            }

            if (!streams_.empty()) {
                stream_content_->HideEmptyTip();
            }
            else {
                stream_content_->ShowEmptyTip();
            }

            // update to stream state checker
            state_checker_->UpdateCurrentStreamItems(streams_);
        });
    }

}
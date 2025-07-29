//
// Created by RGAA on 16/08/2024.
//

#include "panel_clipboard_manager.h"
#include "rd_context.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QImage>
#include <QBuffer>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <shellapi.h>
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/folder_util.h"
#include "tc_message.pb.h"
#include "tc_message_new/proto_converter.h"
#include "tc_message_new/rp_proto_converter.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/clipboard/win/panel_cp_file_stream.h"
#include "render_panel/clipboard/win/panel_cp_virtual_file.h"

namespace tc
{

    ClipboardManager::ClipboardManager(const std::shared_ptr<GrContext>& ctx) : QObject(nullptr) {
        context_ = ctx;
    }

//    static bool GetClipboardFiles(HWND hwnd, std::vector<std::wstring>& files) {
//        files.clear();
//
//        if (!OpenClipboard(hwnd)) {
//            std::cerr << "Failed to open clipboard" << std::endl;
//            return false;
//        }
//
//        // 检查是否有文件被复制
//        HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
//        if (hDrop) {
//            UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
//
//            for (UINT i = 0; i < fileCount; i++) {
//                wchar_t filePath[MAX_PATH];
//                if (DragQueryFile(hDrop, i, filePath, MAX_PATH)) {
//                    files.push_back(filePath);
//                }
//            }
//        }
//
//        CloseClipboard();
//        return true;
//    }

//    void ClipboardManager::OnLocalClipboardUpdated(const std::shared_ptr<MsgClipboardEvent>& msg) {
//        LOGI("**clipboard update, type : {}, msg: {}, file size: {}", (int)msg->clipboard_type_, msg->text_msg_, msg->files_.size());
//        for (const auto& file : msg->files_) {
//            LOGI("** file name: {}, size: {}, full path: {}, ref path: {}", file.file_name_, file.total_size_, file.full_path_, file.ref_path_);
//        }
//
//        if (msg->clipboard_type_ == MsgClipboardType::kText) {
//            // send it to remote
//            tc::Message m;
//            m.set_type(tc::kClipboardInfo);
//            auto sub = m.mutable_clipboard_info();
//            sub->set_type(ClipboardType::kClipboardText);
//            sub->set_msg(msg->text_msg_);
//            auto buffer = ProtoAsData(&m);
//            // todo::
//            //plugin_->DispatchAllStreamMessage(buffer);
//        }
//        else if (msg->clipboard_type_ == MsgClipboardType::kFiles && !msg->files_.empty()) {
//            tc::Message m;
//            m.set_type(tc::kClipboardInfo);
//            auto sub = m.mutable_clipboard_info();
//            sub->set_type(ClipboardType::kClipboardFiles);
//            for (const auto& file : msg->files_) {
//                auto pf = sub->mutable_files()->Add();
//                pf->set_file_name(file.file_name_);
//                pf->set_full_path(file.full_path_);
//                pf->set_ref_path(file.ref_path_);
//                pf->set_total_size(file.total_size_);
//            }
//            auto buffer = ProtoAsData(&m);
//            // todo::
//            //plugin_->DispatchAllStreamMessage(buffer);
//        }
//    }

    void ClipboardManager::OnRemoteClipboardInfo(std::shared_ptr<Message> msg) {
        if (msg->type() == MessageType::kClipboardInfo) {
            auto sub = msg->clipboard_info();
            LOGI("Remote Clipboard info, type : {}", (int)sub.type());
            if (sub.type() == ClipboardType::kClipboardText) {
                auto in_text = sub.msg();
                auto is_same = false;
                for (int i = 0; i < 100; i++) {
                    QClipboard *board = QGuiApplication::clipboard();
                    if (board->text() == in_text) {
                        is_same = true;
                        LOGI("Already same with clipboard, ignore: {}", in_text);
                        break;
                    }
                    board->setText(QString::fromStdString(in_text));
                    if (board->ownsClipboard() && board->text() == in_text) {
                        is_same = true;
                        LOGI("*** update remote clipboard info: {}", in_text);
                        break;
                    } else {
                        LOGE("Can't update remote clipboard, not own it.");
                    }
                    TimeUtil::DelayBySleep(5);
                }
                if (is_same) {
                    // to panel
                    //auto event = std::make_shared<GrPluginRemoteClipboardResp>();
                    //event->content_type_ = (int)sub.type();
                    //event->remote_info_ = sub.msg();
                    // todo::
                    //plugin_->CallbackEvent(event);

                    // notify clipboard monitor
                    auto sub_resp = msg->clipboard_info_resp();
                    context_->SendAppMessage(MsgRemoteClipboardResp {
                        .text_msg_ = sub_resp.msg(),
                    });

                    // send back
                    tc::Message resp_msg;
                    resp_msg.set_type(tc::kClipboardInfoResp);
                    auto resp_sub = resp_msg.mutable_clipboard_info_resp();
                    resp_sub->set_type(ClipboardType::kClipboardText);
                    resp_sub->set_msg(in_text);
                    auto buffer = ProtoAsData(&resp_msg);
                    // now
                    auto rp_msg = tc::MakeRpRawRenderMessage(msg->stream_id(), msg->device_id(), resp_msg.SerializeAsString(), true);
                    context_->GetApplication()->PostMessage2Renderer(rp_msg);

                    // before
                    //plugin_->DispatchAllStreamMessage(buffer);
                }
            }/*
            else if (sub.type() == ClipboardType::kClipboardImage) {
                auto in_image = sub.msg();
                QImage image;
                image.loadFromData((uchar*)in_image.c_str(), in_image.size(), "PNG");
                if (image.isNull()) {
                    LOGE("An invalid image...");
                    return;
                }
                LOGI("In image size: {}, {}x{}", in_image.size(), image.width(), image.height());
                for (int i = 0; i < 100; i++) {
                    QClipboard *board = QGuiApplication::clipboard();
                    board->setImage(image);
                    if (board->ownsClipboard()) {
                        LOGI("set image Success: {}", i);
                        break;
                    }
                    LOGI("Will try next: {}", i);
                    TimeUtil::DelayBySleep(5);
                }
            }*/

            if (sub.type() == ClipboardType::kClipboardFiles) {
                const auto& files = msg->clipboard_info().files();
                std::vector<ClipboardFile> target_files;
                for (auto& file : files) {
                    ClipboardFile cpy_file;
                    cpy_file.CopyFrom(file);
                    target_files.push_back(file);
                    LOGI("Clipboard file: {}", file.file_name());
                }

                context_->PostUITask([=, this]() {
                    if (!virtual_file_) {
                        virtual_file_ = tc::CreateVirtualFile(IID_IDataObject, (void **) &data_object_, context_);
                    }
                    if (!data_object_) {
                        LOGE("DataObject is null!");
                        return;
                    }

                    ::OleInitialize(nullptr);

                    bool cleared_clipboard = false;
                    for (int i = 0; i < 100; i++) {
                        auto hr = ::OleSetClipboard(nullptr);
                        if (hr == S_OK) {
                            cleared_clipboard = true;
                            break;
                        }
                        TimeUtil::DelayBySleep(10);
                    }
                    if (!cleared_clipboard) {
                        LOGE("Empty clipboard failed!");
                        return;
                    }

                    TimeUtil::DelayBySleep(10);

                    bool set_clipboard = false;
                    for (int i = 0; i < 100; i++) {
                        auto hr = ::OleSetClipboard(data_object_);
                        if (hr == S_OK) {
                            set_clipboard = true;
                            break;
                        }
                    }
                    if (!set_clipboard) {
                        LOGE("Set clipboard failed!");
                        return;
                    }
                    ::CloseClipboard();
                    ::OleUninitialize();

                    auto device_id = msg->device_id();
                    auto stream_id = msg->stream_id();
                    virtual_file_->OnClipboardFilesInfo(device_id, stream_id, target_files);
                });
            }
        }
        else if (msg->type() == MessageType::kClipboardRespBuffer) {
            if (virtual_file_) {
                virtual_file_->OnClipboardRespBuffer(msg->cp_resp_buffer());
            }
        }
    }

}

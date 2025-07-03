//
// Created by RGAA on 16/08/2024.
//

#include "clipboard_manager.h"
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
#include "clipboard_plugin.h"
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    ClipboardManager::ClipboardManager(ClipboardPlugin* plugin) : QObject(nullptr) {
        this->plugin_ = plugin;
    }

    static bool GetClipboardFiles(HWND hwnd, std::vector<std::wstring>& files) {
        files.clear();

        if (!OpenClipboard(hwnd)) {
            std::cerr << "Failed to open clipboard" << std::endl;
            return false;
        }

        // 检查是否有文件被复制
        HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
        if (hDrop) {
            UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

            for (UINT i = 0; i < fileCount; i++) {
                wchar_t filePath[MAX_PATH];
                if (DragQueryFile(hDrop, i, filePath, MAX_PATH)) {
                    files.push_back(filePath);
                }
            }
        }

        CloseClipboard();
        return true;
    }

    void ClipboardManager::OnLocalClipboardUpdated(const std::shared_ptr<MsgClipboardEvent>& msg) {
        LOGI("**clipboard update, type : {}, msg: {}, file size: {}", (int)msg->clipboard_type_, msg->text_msg_, msg->files_.size());
        for (const auto& file : msg->files_) {
            LOGI("** file name: {}, size: {}, full path: {}, ref path: {}", file.file_name_, file.total_size_, file.full_path_, file.ref_path_);
        }

        if (msg->clipboard_type_ == MsgClipboardType::kText) {
            // send it to remote
            tc::Message m;
            m.set_type(tc::kClipboardInfo);
            auto sub = m.mutable_clipboard_info();
            sub->set_type(ClipboardType::kClipboardText);
            sub->set_msg(msg->text_msg_);
            plugin_->DispatchAllStreamMessage(m.SerializeAsString());
        }
        else if (msg->clipboard_type_ == MsgClipboardType::kFiles && !msg->files_.empty()) {
            tc::Message m;
            m.set_type(tc::kClipboardInfo);
            auto sub = m.mutable_clipboard_info();
            sub->set_type(ClipboardType::kClipboardFiles);
            for (const auto& file : msg->files_) {
                auto pf = sub->mutable_files()->Add();
                pf->set_file_name(file.file_name_);
                pf->set_full_path(file.full_path_);
                pf->set_ref_path(file.ref_path_);
                pf->set_total_size(file.total_size_);
            }
            plugin_->DispatchAllStreamMessage(m.SerializeAsString());
        }
    }

    void ClipboardManager::OnRemoteClipboardInfo(const std::shared_ptr<Message>& msg) {
        auto sub = msg->clipboard_info();
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
                auto event = std::make_shared<GrPluginRemoteClipboardResp>();
                auto sub = msg->clipboard_info_resp();
                event->content_type_ = (int)sub.type();
                event->remote_info_ = sub.msg();
                plugin_->CallbackEvent(event);

                // send back
                tc::Message resp_msg;
                resp_msg.set_type(tc::kClipboardInfoResp);
                auto resp_sub = resp_msg.mutable_clipboard_info_resp();
                resp_sub->set_type(ClipboardType::kClipboardText);
                resp_sub->set_msg(in_text);
                plugin_->DispatchAllStreamMessage(resp_msg.SerializeAsString());
            }
        }
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
        }
    }

}

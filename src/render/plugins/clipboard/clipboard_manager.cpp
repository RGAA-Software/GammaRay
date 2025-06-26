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

    void ClipboardManager::OnClipboardUpdated(const std::shared_ptr<MsgClipboardUpdate>& msg) {
        QClipboard *board = QGuiApplication::clipboard();
        auto mime_data = const_cast<QMimeData*>(board->mimeData());
        bool has_urls = mime_data->hasUrls();
        LOGI("clipboard updated, has url ? {}", has_urls);
        if (has_urls) {
            auto urls = mime_data->urls();
            LOGI("Has urls: {}", has_urls);

            // URL:         file:///C:/Users/xx/Documents/aaa.png
            // Full Path:   C:/Users/xx/Documents/aaa.png
            // Ref Path:    aaa.png
            // Base Folder: C:/Users/xx/Documents

            auto fn_make_cp_file=
                [=, this](const QString& base_folder_path, const QString& full_path) -> std::optional<ClipboardFile> {
                    QFileInfo file_info(full_path);
                    if (!file_info.exists()) {
                        return std::nullopt;
                    }
                    auto cpy_full_path = full_path;
                    if (!cpy_full_path.contains(base_folder_path)) {
                        LOGE("not same folder, {} => {}", base_folder_path.toStdString(), full_path.toStdString());
                        return std::nullopt;
                    }
                    auto ref_path = cpy_full_path.mid(base_folder_path.size()+1);

                    auto cp_file = ClipboardFile();
                    cp_file.set_full_path(full_path.toStdString());
                    cp_file.set_file_name(file_info.fileName().toStdString());
                    cp_file.set_ref_path(ref_path.toStdString());
                    cp_file.set_total_size(file_info.size());
                    return cp_file;
                };

            // find base folder
            QString base_folder_path = "";
            for (auto& url : urls) {
                auto full_path = url.toLocalFile();
                QFileInfo file_info(full_path);
                base_folder_path = file_info.dir().path();
                break;
            }
            LOGI("Clipboard, base folder path: {}", base_folder_path.toStdString());
            if (base_folder_path.isEmpty()) {
                LOGE("Clipboard base folder path is empty.");
                return;
            }

            // retrieve all files
            std::vector<ClipboardFile> cp_files;
            for (auto& url : urls) {
                auto full_path = url.toLocalFile();
                QFileInfo file_info(full_path);
                if (file_info.isDir()) {
                    FolderUtil::VisitAllByQt(full_path.toStdString(), [&](VisitResult&& r) {
                        auto cp_file = fn_make_cp_file(base_folder_path, QString::fromStdWString(r.path_));
                        if (cp_file) {
                            cp_files.push_back(cp_file.value());
                        }
                    });
                }
                else {
                    auto cp_file = fn_make_cp_file(base_folder_path, full_path);
                    if (cp_file.has_value()) {
                        cp_files.push_back(cp_file.value());
                    }
                }

                LOGI("url: {}, path: {}", url.toString().toStdString(), url.toLocalFile().toStdString());
            }

            // debug
            LOGI("Total files: {}", cp_files.size());
            for (const auto& file : cp_files) {
                LOGI("==> full path: {}, ref path: {}, total size: {}", file.full_path(), file.ref_path(), file.total_size());
            }

            tc::Message m;
            m.set_type(tc::kClipboardInfo);
            auto sub = m.mutable_clipboard_info();
            sub->set_type(ClipboardType::kClipboardFiles);
            for (const auto& file : cp_files) {
                auto pf = sub->mutable_files()->Add();
                pf->set_file_name(file.file_name());
                pf->set_full_path(file.full_path());
                pf->set_ref_path(file.ref_path());
                pf->set_total_size(file.total_size());
            }
            plugin_->DispatchAllStreamMessage(m.SerializeAsString());
        }
        else {
            auto text = board->text();
            if (!text.isEmpty()) {
                if (text.isEmpty() || text == remote_info_) {
                    LOGI("Same with remote: {}", text.toStdString());
                    return;
                }
                LOGI("===> new Text: {}", text.toStdString());

                auto event = std::make_shared<GrPluginClipboardEvent>();
                event->type_ = ClipboardType::kClipboardText;
                event->msg_ = text.toStdString();
                this->plugin_->CallbackEvent(event);

                // send to remote
                tc::Message m;
                m.set_type(tc::kClipboardInfo);
                auto sub = m.mutable_clipboard_info();
                sub->set_type(ClipboardType::kClipboardText);
                sub->set_msg(text.toStdString());
                plugin_->DispatchAllStreamMessage(m.SerializeAsString());

                remote_info_ = text;
            }
        }
    }

    void ClipboardManager::UpdateRemoteInfo(const std::shared_ptr<Message>& msg) {
        QMetaObject::invokeMethod(this, [=, this]() {
            auto sub = msg->clipboard_info();
            LOGI("===>2 clipboard type: {}", (int)sub.type());
            if (sub.type() == ClipboardType::kClipboardText) {
                auto in_text = sub.msg();
                auto updated = false;
                for (int i = 0; i < 100; i++) {
                    QClipboard *board = QGuiApplication::clipboard();
                    if (board->text() == in_text) {
                        LOGI("Already same with clipboard, ignore: {}", in_text);
                        return;
                    }
                    board->setText(QString::fromStdString(in_text));
                    if (board->ownsClipboard() && board->text() == in_text) {
                        updated = true;
                        LOGI("*** update remote clipboard info: {}", in_text);
                        break;
                    } else {
                        LOGE("Can't update remote clipboard, not own it.");
                    }
                    TimeUtil::DelayBySleep(5);
                }
                if (updated) {
                    remote_info_ = QString::fromStdString(in_text);
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
        });
    }

}

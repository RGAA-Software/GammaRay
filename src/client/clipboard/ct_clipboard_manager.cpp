//
// Created by RGAA on 16/08/2024.
//

#include "ct_clipboard_manager.h"
#include "client/ct_client_context.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QBuffer>
#include <QMimeData>
#include <QUrl>
#include <vector>
#include <optional>
#include "tc_common_new/log.h"
#include "client/ct_app_message.h"
#include "client/ct_settings.h"
#include "tc_common_new/time_util.h"
#include "tc_message.pb.h"
#include "tc_common_new/file.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/file_util.h"
#include "win/win_message_loop.h"

namespace tc
{

    ClipboardManager::ClipboardManager(const std::shared_ptr<ClientContext> &ctx) : QObject(nullptr) {
        context_ = ctx;

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<ClipboardUpdatedMsg>([=, this](const ClipboardUpdatedMsg& msg) {
            context_->PostUITask([=, this]() {
                this->OnClipboardUpdated();
            });
        });
    }

    void ClipboardManager::Start() {
        msg_loop_ = WinMessageLoop::Make(context_);
        msg_loop_->Start();
    }

    void ClipboardManager::Stop() {
        if (msg_loop_) {
            msg_loop_->Stop();
        }
    }

    void ClipboardManager::OnClipboardUpdated() {
        if (!Settings::Instance()->clipboard_on_) {
            return;
        }

        LOGI("===> OnClipboardUpdated!");

        QClipboard *board = QGuiApplication::clipboard();
        auto mime_data = const_cast<QMimeData*>(board->mimeData());

        bool has_urls = mime_data->hasUrls();
        auto text = board->text();
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
                    cp_file.set_total_size((int32_t)file_info.size());
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

            context_->SendAppMessage(ClipboardMessage{
                    .type_ = ClipboardType::kClipboardFiles,
                    .files_ = cp_files,
            });
        }
        else if (!text.isEmpty()) {
            LOGI("info: {}, remote: {}", text.toStdString(), remote_info_.toStdString());
            if (text == remote_info_) {
                return;
            }
            LOGI("===> new Text: {}", text.toStdString());

            context_->SendAppMessage(ClipboardMessage{
                    .type_ = ClipboardType::kClipboardText,
                    .msg_ = text.toStdString(),
            });
            remote_info_ = text;
        }
    }

    void ClipboardManager::UpdateRemoteInfo(const QString& in_text) {
        if (!Settings::Instance()->clipboard_on_) {
            LOGI("clipboard is not on!");
            return;
        }
        context_->PostUITask([=, this]() {
            auto updated = false;
            auto count = 0;
            for (int i = 0; i < 50; i++) {
                QClipboard *board = QGuiApplication::clipboard();
                if (board->text() == in_text) {
                    LOGI("Already same with clipboard, ignore: {}", in_text.toStdString());
                    return;
                }
                board->setText(in_text);
                if (board->ownsClipboard() && board->text() == in_text) {
                    updated = true;
                    LOGI("*** update remote clipboard info: {}", in_text.toStdString());
                    break;
                }
                else {
                    LOGE("Can't update remote clipboard, not own it.");
                }
                count++;
                TimeUtil::DelayBySleep(5);
            }
            LOGI("update remote clipboard info used count: {}", count);
            if (updated) {
                remote_info_ = in_text;
            }
        });
    }

}

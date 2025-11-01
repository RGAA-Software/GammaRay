#include "win_panel_message_loop.h"
#include <iostream>
#include <wtsapi32.h>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include <QString>
#include <QApplication>
#include <QClipboard>
#include "tc_common_new/log.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_app_messages.h"
#include "win_panel_message_window.h"
#include "tc_render_panel_message.pb.h"
#include "tc_common_new/folder_util.h"
#include "tc_message_new/rp_proto_converter.h"

using namespace tcrp;

namespace tc
{

    constexpr char kWindowName[] = "GammaRay_panel_MessageWindow";


    void CALLBACK WinMessageLoop::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
    {
        if (event == EVENT_SYSTEM_DESKTOPSWITCH)
        {
            std::cout << "Desktop switch event detected." << std::endl;
        }
    }

    WinMessageLoop::WinMessageLoop(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        context_ = app_->GetContext();
        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgRemoteClipboardResp>([=, this](const MsgRemoteClipboardResp& msg) {
            remote_info_ = QString::fromStdString(msg.text_msg_);
            LOGI("===> Remote is :{}", remote_info_.toStdString());
        });
    }

    WinMessageLoop::~WinMessageLoop() {

    }

    void WinMessageLoop::CreateMessageWindow() {
        message_window_ = WinMessageWindow::Make(context_, shared_from_this());
    }

    void WinMessageLoop::OnClipboardUpdate(HWND hwnd) {
        if (!app_->IsRendererConnected()) {
            LOGE("render is offline, clipboard not work!");
            return;
        }

        QClipboard *board = QGuiApplication::clipboard();
        auto mime_data = const_cast<QMimeData*>(board->mimeData());
        bool has_urls = mime_data->hasUrls();
        auto text = board->text();

        auto fn_send_text = [=, this]() {
            LOGI("info: {}, remote: {}", text.toStdString(), remote_info_.toStdString());
            if (text == remote_info_) {
                return;
            }
            LOGI("===> new Text: {}", text.toStdString());

            tcrp::RpMessage msg;
            msg.set_type(RpMessageType::kRpClipboardEvent);
            auto sub = msg.mutable_clipboard_info();
            sub->set_type(RpClipboardType::kRpClipboardText);
            sub->set_msg(text.toStdString());
            app_->PostMessage2Renderer(tc::RpProtoAsData(&msg));

        };

        if (has_urls) {
            // URL:         file:///C:/Users/xx/Documents/aaa.png
            // Full Path:   C:/Users/xx/Documents/aaa.png
            // Ref Path:    aaa.png
            // Base Folder: C:/Users/xx/Documents

            auto urls = mime_data->urls();
            auto fn_make_cp_file=
                [=, this](const QString& base_folder_path, const QString& full_path) -> std::optional<RpClipboardFile> {
                    QFileInfo file_info(full_path);
                    if (!file_info.exists()) {
                        return std::nullopt;
                    }
                    auto cpy_full_path = full_path;
                    if (!cpy_full_path.contains(base_folder_path)) {
                        LOGE("not same folder, {} => {}", base_folder_path.toStdString(), full_path.toStdString());
                        return std::nullopt;
                    }

                    // C:/ or C:/Users/xx/Documents
                    int mid_idx_offset = 1;
                    if (base_folder_path.lastIndexOf("/") == base_folder_path.size()-1) {
                        mid_idx_offset = 0;
                    }

                    auto ref_path = cpy_full_path.mid(base_folder_path.size() + mid_idx_offset);

                    auto cp_file = RpClipboardFile();
                    cp_file.set_full_path(full_path.toStdString());
                    cp_file.set_file_name(file_info.fileName().toStdString());
                    cp_file.set_ref_path(ref_path.toStdString());
                    cp_file.set_total_size((int64_t)file_info.size());
                    LOGI("Copy file size: {}", cp_file.total_size());
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
            std::vector<RpClipboardFile> cp_files;
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

                //LOGI("url: {}, path: {}", url.toString().toStdString(), url.toLocalFile().toStdString());
            }

            // debug
            LOGI("Total files: {}", cp_files.size());
            for (const auto& file : cp_files) {
                LOGI("==> full path: {}, ref path: {}, total size: {}", file.full_path(), file.ref_path(), file.total_size());
            }

            if (!cp_files.empty()) {
                tcrp::RpMessage msg;
                msg.set_type(RpMessageType::kRpClipboardEvent);
                auto sub = msg.mutable_clipboard_info();
                sub->set_type(RpClipboardType::kRpClipboardFiles);
                for (const auto &file: cp_files) {
                    auto target_file = sub->mutable_files()->Add();
                    target_file->CopyFrom(file);
                }
                app_->PostMessage2Renderer(tc::RpProtoAsData(&msg));
            }
            else {
                if (!text.isEmpty()) {
                    fn_send_text();
                }
            }
        }
        else if (!text.isEmpty()) {
            fn_send_text();
        }
    }

    void WinMessageLoop::OnWinSessionChange(uint32_t message) {

    }

    void WinMessageLoop::Start() {
        CreateMessageWindow();
        thread_ = std::thread(std::bind(&WinMessageLoop::ThreadFunc, this));
    }

    void WinMessageLoop::Stop() {
        LOGI("WinMessageLoop stopping...");
        message_window_->CloseWindow();
        if (thread_.joinable()) {
            thread_.join();
        }
        LOGI("WinMessageLoop stoped.");
    }

    void WinMessageLoop::ThreadFunc() {
        // https://github.com/dchapyshev/aspia
        if (!message_window_->Create(kWindowName)) {
            LOGE("WinMessageLoop create window error.");
            return;
        }
        LOGI("WinMessageWindow create success");
        HWND hwnd = nullptr;
        hwnd = message_window_->GetHwnd();
        if (!hwnd) {
            LOGE("WinMessageLoop hwnd is nullptr.");
            return;
        }

        AddClipboardFormatListener(hwnd);
        LOGI("AddClipboardFormatListener already add WinMessageWindow");

        if (!WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_ALL_SESSIONS)) {
            LOGE("WTSRegisterSessionNotification error: %d", GetLastError());
            return;
        }

        HWINEVENTHOOK hEventHook = SetWinEventHook(EVENT_SYSTEM_DESKTOPSWITCH, EVENT_SYSTEM_DESKTOPSWITCH, nullptr, &WinMessageLoop::WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

        if (hEventHook == nullptr)
        {
            std::cout << "Failed to set event hook." << std::endl;
            return;
        }

        int bRet = 0;
        MSG msg{};
        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
            if (bRet == -1) {
                break;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        UnhookWinEvent(hEventHook);
    }

}
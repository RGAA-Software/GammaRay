//
// Created by RGAA on 16/08/2024.
//

#include "clipboard_manager.h"
#include "client_context.h"
#include <QGuiApplication>
#include <QClipboard>
#include "tc_common_new/log.h"
#include "app_message.h"
#include "settings.h"

namespace tc
{

    ClipboardManager::ClipboardManager(const std::shared_ptr<ClientContext> &ctx) : QObject(nullptr) {
        context_ = ctx;
    }

    void ClipboardManager::Monitor() {
        QClipboard *board = QGuiApplication::clipboard();
        connect(board, &QClipboard::dataChanged, this, [=, this]() {
            if (!Settings::Instance()->clipboard_on_) {
                return;
            }
            auto info = board->text();
            LOGI("info: {}, remote: {}", info.toStdString(), remote_info_.toStdString());
            if (info.isEmpty() || info == remote_info_) {
                return;
            }
            QString text = board->text();
            LOGI("===> new Text: {}", text.toStdString());

            context_->SendAppMessage(ClipboardMessage{
                .msg_ = text.toStdString(),
            });
        });
    }

    void ClipboardManager::UpdateRemoteInfo(const QString& info) {
        if (!Settings::Instance()->clipboard_on_) {
            return;
        }
        remote_info_ = info;
        context_->PostUITask([=, this]() {
            QClipboard *board = QGuiApplication::clipboard();
            board->setText(remote_info_);
            LOGI("remote clipboard info: {}", info.toStdString());
        });
    }

}

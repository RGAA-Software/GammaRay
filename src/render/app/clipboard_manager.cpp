//
// Created by RGAA on 16/08/2024.
//

#include "clipboard_manager.h"
#include "context.h"
#include <QGuiApplication>
#include <QClipboard>
#include "tc_common_new/log.h"
#include "app_messages.h"

namespace tc
{

    ClipboardManager::ClipboardManager(const std::shared_ptr<Context> &ctx) : QObject(nullptr) {
        context_ = ctx;
    }

    void ClipboardManager::Monitor() {
        QClipboard *board = QGuiApplication::clipboard();
        connect(board, &QClipboard::dataChanged, this, [=, this]() {
            auto info = board->text();
            if (info.isEmpty() || info == remote_info_) {
                LOGI("Same with remote: {}", info.toStdString());
                return;
            }
            QString text = board->text();
            LOGI("===> new Text: {}", text.toStdString());

            context_->SendAppMessage(ClipboardMessage {
                .msg_ = text.toStdString(),
            });
        });
    }

    void ClipboardManager::UpdateRemoteInfo(const QString& info) {
        QMetaObject::invokeMethod(this, [=, this]() {
            remote_info_ = info;
            QClipboard *board = QGuiApplication::clipboard();
            board->setText(remote_info_);
            LOGI("remote clipboard info: {}", info.toStdString());
        });
    }

}

//
// Created by RGAA on 16/08/2024.
//

#include "clipboard_manager.h"
#include "rd_context.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QImage>
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "app_messages.h"
#include "tc_message.pb.h"

namespace tc
{

    ClipboardManager::ClipboardManager(const std::shared_ptr<RdContext> &ctx) : QObject(nullptr) {
        context_ = ctx;
    }

    void ClipboardManager::Monitor() {
        QClipboard *board = QGuiApplication::clipboard();
        connect(board, &QClipboard::dataChanged, this, [=, this]() {
            auto text = board->text();
            if (!text.isEmpty()) {
                if (text.isEmpty() || text == remote_info_) {
                    LOGI("Same with remote: {}", text.toStdString());
                    return;
                }
                LOGI("===> new Text: {}", text.toStdString());

                context_->SendAppMessage(ClipboardMessage {
                    .type_ = ClipboardType::kClipboardText,
                    .msg_ = text.toStdString(),
                });
                remote_info_ = text;
            }
        });
    }

    void ClipboardManager::UpdateRemoteInfo(std::shared_ptr<Message>&& msg) {
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

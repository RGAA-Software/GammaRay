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
#include "tc_message.pb.h"
#include "clipboard_plugin.h"
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    ClipboardManager::ClipboardManager(ClipboardPlugin* plugin) : QObject(nullptr) {
        this->plugin_ = plugin;
    }

    void ClipboardManager::OnClipboardUpdate() {
        QClipboard *board = QGuiApplication::clipboard();
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
            plugin_->PostToAllStreamMessage(m.SerializeAsString());

            remote_info_ = text;
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

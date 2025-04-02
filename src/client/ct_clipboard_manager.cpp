//
// Created by RGAA on 16/08/2024.
//

#include "client/ct_clipboard_manager.h"
#include "client/ct_client_context.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QBuffer>
#include <QMimeData>
#include <QUrl>
#include "tc_common_new/log.h"
#include "client/ct_app_message.h"
#include "client/ct_settings.h"
#include "tc_common_new/time_util.h"
#include "tc_message.pb.h"
#include "tc_common_new/file.h"

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

            QMimeData* mime_data = const_cast<QMimeData*>(board->mimeData());

            // 优先尝试获取已压缩的数据(PNG/JPEG)
            QByteArray compressedData;
            if (mime_data->hasFormat("image/png")) {
                // 获取PNG格式的原始压缩数据
                compressedData = mime_data->data("image/png");
                LOGI("获取到PNG格式数据，大小:", compressedData.size());
            } else if (mime_data->hasFormat("image/jpeg")) {
                // 获取JPEG格式的原始压缩数据
                compressedData = mime_data->data("image/jpeg");
                LOGI("获取到JPG格式数据，大小:", compressedData.size());
            } else if (mime_data->hasImage()) {
                LOGI(" !!!! has image ,but not jpg, png");
                // 如果没有压缩格式，获取QImage并压缩为PNG
//                QImage image = mime_data->imageData();
//                QBuffer buffer(&compressedData);
//                buffer.open(QIODevice::WriteOnly);
//                image.save(&buffer, "PNG");  // 压缩为PNG格式
//                qDebug() << "将图像压缩为PNG，大小:" << compressedData.size() << "字节";
            } else {
//                qDebug() << "剪贴板中没有图像数据";
//                return;
                LOGI("Dont have any IMAGE!");
            }

            bool has_urls = mime_data->hasUrls();
            auto text = board->text();
            if (has_urls) {
                auto urls = mime_data->urls();
                LOGI("Has urls: {}", has_urls);
                for (auto& url : urls) {
                    LOGI("url: {}, path: {}", url.toString().toStdString(), url.toLocalFile().toStdString());
                    auto file = File::OpenForReadB(url.toLocalFile().toStdString());
                    auto data = file->ReadAllAsString();
                    context_->SendAppMessage(ClipboardMessage{
                        .type_ = ClipboardType::kClipboardImage,
                        .msg_ = data,
                    });
                }
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
        });
    }

    void ClipboardManager::UpdateRemoteInfo(const QString& in_text) {
        if (!Settings::Instance()->clipboard_on_) {
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

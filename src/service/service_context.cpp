//
// Created by RGAA on 21/10/2024.
//

#include "service_context.h"
#include "service_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/file_util.h"

namespace tc
{
    ServiceContext::ServiceContext(int port) {
        listening_port_ = port;
        // message notifier
        msg_notifier_ = std::make_shared<MessageNotifier>();

        // pool
        iopool_ = std::make_shared<asio2::iopool>();
        iopool_->start();

        // shared preference
        sp_ = SharedPreference::Instance();
        auto exe_path = QString::fromStdWString(FolderUtil::GetCurrentFilePath()).toStdString();
        auto folder_path = FileUtil::GetFileFolder(exe_path) + "/gr_data";
        LOGI("Folder path: {}", folder_path);
        if (!sp_->Init(folder_path, "gammaray_service.dat")) {
            LOGE("Init gammaray_service.data failed!");
        }

        // timers
        timer_ = std::make_shared<asio2::timer>();
        std::vector<int> time_durations = {
            1000, 3000,
        };
        for (auto& duration : time_durations) {
            timer_->start_timer(std::format("tid:{}", duration), duration, [=, this]() {
                if (duration == 1000) {
                    this->SendAppMessage(MsgTimer1S{});
                }
                else if (duration == 3000) {
                    this->SendAppMessage(MsgTimer3S{});
                }
            });
        }
    }

    void ServiceContext::PostBgTask(std::function<void()>&& task) {
        iopool_->post(std::move(task));
    }

    std::shared_ptr<MessageListener> ServiceContext::CreateMessageListener() {
        return msg_notifier_->CreateListener();
    }

    std::string ServiceContext::GetAppExeFolderPath() {
        auto exe_path = QString::fromStdWString(FolderUtil::GetCurrentFilePath()).toStdString();
        return FileUtil::GetFileFolder(exe_path);
    }

}

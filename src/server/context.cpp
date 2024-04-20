//
// Created by RGAA on 2023/12/18.
//

#include "context.h"
#include "tc_common_new/task_runtime.h"
#include "tc_common_new/message_notifier.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace tc
{
    std::shared_ptr<Context> Context::Make() {
        return std::make_shared<Context>();
    }

    Context::Context() {
        task_runtime_ = std::make_shared<TaskRuntime>();
        msg_notifier_ = std::make_shared<MessageNotifier>();

        asio2_iopool_ = std::make_shared<asio2::iopool>();
        asio2_iopool_->start();
    }

    std::shared_ptr<TaskRuntime> Context::GetTaskRuntime() {
        return task_runtime_;
    }

    std::shared_ptr<MessageNotifier> Context::GetMessageNotifier() {
        return msg_notifier_;
    }

    uint64_t Context::PostInTaskRuntime(std::function<void()>&& task) {
        return task_runtime_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    std::shared_ptr<asio2::iopool> Context::GetAsio2IoPool() {
        return asio2_iopool_;
    }

    std::string Context::GetCurrentExeFolder() {
        auto exe_folder_path = boost::filesystem::initial_path<boost::filesystem::path>();
        return exe_folder_path.string();
    }

}
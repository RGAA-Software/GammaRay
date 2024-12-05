//
// Created by RGAA on 2024-03-30.
//

#include "render_manager.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/shared_preference.h"
#include "service_messages.h"
#include "service_context.h"

#include <QCoreApplication>

static const std::string kGammaRayName = "GammaRay.exe";
static const std::string kGammaRayServerName = "GammaRayServer.exe";
static const std::string kKeyWorkDir = "render_work_dir";
static const std::string kKeyAppPath = "render_app_path";
static const std::string kKeyAppArgs = "render_app_args";

namespace tc
{

    RenderManager::RenderManager(const std::shared_ptr<ServiceContext>& ctx) {
        context_ = ctx;
        msg_listener_ = context_->CreateMessageListener();
        msg_listener_->Listen<MsgTimer3S>([=, this](const MsgTimer3S& msg) {
            context_->PostBgTask([=, this]() {
                this->CheckRenderAlive();
            });
        });
    }

    RenderManager::~RenderManager() {
        Exit();
    }

    bool RenderManager::StartServer(const std::string& _work_dir, const std::string& _app_path, const std::vector<std::string>& _args) {
        std::stringstream ss;
        for (auto& arg : _args) {
            ss << arg << " ";
        }
        ss << std::endl;
        LOGI("Start server \nwork_dir: {} \napp_path: {} \nargs:{}", _work_dir, _app_path, ss.str());

        auto sp = context_->GetSp();
        auto exist_work_dir = sp->Get(kKeyWorkDir);
        auto exist_app_path = sp->Get(kKeyAppPath);
        auto exist_app_args = sp->Get(kKeyAppArgs);
        if (render_process_ != nullptr) {
            if (exist_work_dir != _work_dir || exist_app_path != _app_path || exist_app_args != ss.str()) {
                StopServer();
            }
        }

        this->work_dir_ = _work_dir;
        this->app_path_ = _app_path;
        this->app_args_ = ss.str();

        bool start_result = StartServerInternal(this->work_dir_, this->app_path_, this->app_args_);
        CheckRenderAlive();
        return start_result;
    }

    bool RenderManager::StopServer() {
        if (render_process_) {
            ProcessHelper::CloseProcess(render_process_->pid_);
            render_process_ = nullptr;
        }
        return true;
    }

    bool RenderManager::ReStart() {
        this->StopServer();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return this->StartServer("", "", {});
    }

    void RenderManager::Exit() {
        StopServer();
    }

    void RenderManager::CheckRenderAlive() {
        auto processes = ProcessHelper::GetProcessList(false);
        if (processes.empty()) {
            LOGW("Can't get process list.");
            return;
        }

        for (auto& p : processes) {
            //LOGI("p.exe_name: {}", p->exe_full_path_);
            if (p->exe_full_path_.find(kGammaRayServerName) != std::string::npos) {
                //LOGI("Yes, find it.");
                is_render_alive_ = true;
                render_process_ = p;

                auto sp = context_->GetSp();
                sp->Put(kKeyWorkDir, this->work_dir_);
                sp->Put(kKeyAppPath, this->app_path_);
                sp->Put(kKeyAppArgs, this->app_args_);

                return;
            }
        }
        is_render_alive_ = false;
        render_process_ = nullptr;

        // restart it
        StartServerInternal(this->work_dir_, this->app_path_, this->app_args_);
    }

    bool RenderManager::StartServerInternal(const std::string& _work_dir, const std::string& _app_path, const std::string& args) {
        QString work_dir = QString::fromStdString(_work_dir);
        QString current_path = QString::fromStdString(std::format("{}/{} {}", _work_dir, kGammaRayServerName, args));
        return ProcessUtil::StartProcessAsUser(current_path.toStdWString(), work_dir.toStdWString(), false);
    }

    bool RenderManager::IsRenderAlive() const {
        return is_render_alive_;
    }

}
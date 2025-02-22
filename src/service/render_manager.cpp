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
                auto processes = ProcessHelper::GetProcessList(false);
                if (!this->CheckRenderAlive(processes) && !this->work_dir_.empty() && !this->app_path_.empty() && !this->app_args_.empty()) {
                    LOGI("GammaRayRender.exe not exist! Will start!");
                    StartServerInternal(this->work_dir_, this->app_path_, this->app_args_);
                }

                if (!this->CheckPanelAlive(processes)) {
                    LOGI("GammaRay.exe not exist!, Will start it");
                    QString work_dir = QString::fromStdString(this->work_dir_);
                    QString current_path = QString::fromStdString(std::format("{}/{}", this->work_dir_, kGammaRayName));
                    return ProcessUtil::StartProcessAsUser(current_path.toStdWString(), work_dir.toStdWString(), false);
                }
            });
        });

        auto sp = context_->GetSp();
        this->work_dir_ = sp->Get(kKeyWorkDir);
        this->app_path_ = sp->Get(kKeyAppPath);
        this->app_args_ = sp->Get(kKeyAppArgs);
    }

    RenderManager::~RenderManager() {

    }

    bool RenderManager::StartServer(const std::string& _work_dir, const std::string& _app_path, const std::vector<std::string>& _args) {
        std::stringstream ss;
        for (auto& arg : _args) {
            ss << arg << " ";
        }
        ss << std::endl;
        LOGI("Start render \nwork_dir: {} \napp_path: {} \nargs:{}", _work_dir, _app_path, ss.str());

        auto sp = context_->GetSp();
        auto exist_work_dir = sp->Get(kKeyWorkDir);
        auto exist_app_path = sp->Get(kKeyAppPath);
        auto exist_app_args = sp->Get(kKeyAppArgs);
        auto processes = ProcessHelper::GetProcessList(false);
        CheckRenderAlive(processes);
        if (render_process_ != nullptr && is_render_alive_) {
            if (exist_work_dir != _work_dir || exist_app_path != _app_path || exist_app_args != ss.str()) {
                StopServer();
            }
            else {
                LOGI("Render is already alive, pid: {}", render_process_->pid_);
                return true;
            }
        }

        this->work_dir_ = _work_dir;
        this->app_path_ = _app_path;
        this->app_args_ = ss.str();

        bool start_result = StartServerInternal(this->work_dir_, this->app_path_, this->app_args_);
        processes = ProcessHelper::GetProcessList(false);
        CheckRenderAlive(processes);
        return start_result;
    }

    bool RenderManager::StopServer() {
        if (render_process_) {
            ProcessHelper::CloseProcess(render_process_->pid_);
            render_process_ = nullptr;
        }

        // kill all
        auto processes = ProcessHelper::GetProcessList(false);
        for (auto& p : processes) {
            if (p->exe_full_path_.find(kGammaRayRenderName) != std::string::npos) {
                ProcessHelper::CloseProcess(p->pid_);
            }
        }

        return true;
    }

    bool RenderManager::ReStart(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& _args) {
        std::stringstream ss;
        for (auto& arg : _args) {
            ss << arg << " ";
        }
        ss << std::endl;
        LOGI("ReStart render \nwork_dir: {} \napp_path: {} \nargs:{}", work_dir, app_path, ss.str());

        this->work_dir_ = work_dir;
        this->app_path_ = app_path;
        this->app_args_ = ss.str();

        this->StopServer();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (this->work_dir_.empty() || this->app_path_.empty() || this->app_args_.empty()) {
            return false;
        }
        return StartServerInternal(work_dir_, app_path_, app_args_);
    }

    void RenderManager::Exit() {
        StopServer();
    }

    bool RenderManager::CheckRenderAlive(const std::vector<std::shared_ptr<ProcessInfo>>& processes) {
        for (auto& p : processes) {
            //LOGI("p.exe_name: {}", p->exe_full_path_);
            if (p->exe_full_path_.find(kGammaRayRenderName) != std::string::npos) {
                //LOGI("Yes, find it.");
                is_render_alive_ = true;
                render_process_ = p;

                auto sp = context_->GetSp();
                sp->Put(kKeyWorkDir, this->work_dir_);
                sp->Put(kKeyAppPath, this->app_path_);
                sp->Put(kKeyAppArgs, this->app_args_);

                return true;
            }
        }
        is_render_alive_ = false;
        render_process_ = nullptr;

        return false;
    }

    bool RenderManager::CheckPanelAlive(const std::vector<std::shared_ptr<ProcessInfo>>& processes) {
        for (auto& p : processes) {
            if (p->exe_full_path_.find(kGammaRayName) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    bool RenderManager::StartServerInternal(const std::string& _work_dir, const std::string& _app_path, const std::string& args) {
        QString work_dir = QString::fromStdString(_work_dir);
        QString current_path = QString::fromStdString(std::format("{}/{} {}", _work_dir, kGammaRayRenderName, args));
        return ProcessUtil::StartProcessAsUser(current_path.toStdWString(), work_dir.toStdWString(), false);
    }

    bool RenderManager::IsRenderAlive() const {
        return is_render_alive_;
    }

}
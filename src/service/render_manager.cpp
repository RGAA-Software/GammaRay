//
// Created by RGAA on 2024-03-30.
//

#include "render_manager.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/shared_preference.h"
#include "service_messages.h"
#include "service_context.h"

#include <QCoreApplication>
#include <TlHelp32.h>
#include <wtsapi32.h>
#include <UserEnv.h>

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
                if (processes.size() < 10) {
                    LOGE("To little processes, check your TaskManager.");
                    return;
                }
                this->CheckAliveRenders(processes);
                if (!IsDesktopRenderAlive() && !this->desktop_work_dir_.empty() && !this->desktop_app_path_.empty() && !this->desktop_app_args_.empty()) {
                    LOGI("GammaRayRender.exe not exist! Will start!");
                    StartDesktopRenderInternal(this->desktop_work_dir_, this->desktop_app_path_, this->desktop_app_args_);
                }

                if (!this->CheckPanelAlive(processes)) {

                }

            });
        });

        auto sp = context_->GetSp();
        this->desktop_work_dir_ = sp->Get(kKeyWorkDir);
        this->desktop_app_path_ = sp->Get(kKeyAppPath);
        this->desktop_app_args_ = sp->Get(kKeyAppArgs);
        LOGI("Load render info in storage:");
        LOGI("Work dir: {}", desktop_work_dir_);
        LOGI("App path: {}", desktop_app_path_);
        LOGI("App args: {}", desktop_app_args_);
    }

    RenderManager::~RenderManager() {

    }

    bool RenderManager::StartDesktopRender(const std::string& _work_dir, const std::string& _app_path, const std::vector<std::string>& _args) {
        std::stringstream ss;
        for (auto& arg : _args) {
            ss << arg << " ";
        }
        ss << std::endl;
        LOGI("Start desktop render \nwork_dir: {} \napp_path: {} \nargs:{}", _work_dir, _app_path, ss.str());

        auto sp = context_->GetSp();
        auto exist_work_dir = sp->Get(kKeyWorkDir);
        auto exist_app_path = sp->Get(kKeyAppPath);
        auto exist_app_args = sp->Get(kKeyAppArgs);
        auto processes = ProcessHelper::GetProcessList(false);
        CheckAliveRenders(processes);
        if (desktop_render_process_ != nullptr && is_desktop_render_alive_) {
            if (exist_work_dir != _work_dir || exist_app_path != _app_path || exist_app_args != ss.str()) {
                StopDesktopRender();
            }
            else {
                LOGI("Render is already alive, pid: {}", desktop_render_process_->pid_);
                return true;
            }
        }

        this->desktop_work_dir_ = _work_dir;
        this->desktop_app_path_ = _app_path;
        this->desktop_app_args_ = ss.str();

        bool start_result = StartDesktopRenderInternal(this->desktop_work_dir_, this->desktop_app_path_, this->desktop_app_args_);
        processes = ProcessHelper::GetProcessList(false);
        CheckAliveRenders(processes);
        return start_result;
    }

    bool RenderManager::StopDesktopRender() {
        if (desktop_render_process_) {
            ProcessHelper::CloseProcess(desktop_render_process_->pid_);
            desktop_render_process_ = nullptr;
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

    bool RenderManager::ReStartDesktopRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& _args) {
        std::stringstream ss;
        for (auto& arg : _args) {
            ss << arg << " ";
        }
        ss << std::endl;
        LOGI("ReStartDesktopRender render \nwork_dir: {} \napp_path: {} \nargs:{}", work_dir, app_path, ss.str());

        this->desktop_work_dir_ = work_dir;
        this->desktop_app_path_ = app_path;
        this->desktop_app_args_ = ss.str();

        this->StopDesktopRender();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (this->desktop_work_dir_.empty() || this->desktop_app_path_.empty() || this->desktop_app_args_.empty()) {
            return false;
        }
        return StartDesktopRenderInternal(desktop_work_dir_, desktop_app_path_, desktop_app_args_);
    }

    void RenderManager::Exit() {
        StopDesktopRender();
    }

    void RenderManager::CheckAliveRenders(const std::vector<std::shared_ptr<ProcessInfo>>& processes) {
        bool found_desktop_render = false;
        std::map<RenderProcessId, std::shared_ptr<RenderProcess>> ps;
        for (auto& p : processes) {
            // not GammaRayRender.exe
            if (p->exe_full_path_.find(kGammaRayRenderName) == std::string::npos) {
                continue;
            }

            // desktop mode
            if (p->exe_cmdline_.find("--app_mode=desktop") != std::string::npos) {
                //LOGI("Yes, find it, path with cmdline: {}", p->exe_cmdline_);
                found_desktop_render = true;
                is_desktop_render_alive_ = true;
                desktop_render_process_ = p;

                auto sp = context_->GetSp();
                sp->Put(kKeyWorkDir, this->desktop_work_dir_);
                sp->Put(kKeyAppPath, this->desktop_app_path_);
                sp->Put(kKeyAppArgs, this->desktop_app_args_);
            }

            // others
            if (p->exe_cmdline_.find("--app_mode=inner") != std::string::npos) {
                // TODO: parse ws port
                auto rd_process = std::make_shared<RenderProcess>();
                ps.insert({0, rd_process});
            }
        }

        if (!found_desktop_render) {
            is_desktop_render_alive_ = false;
            desktop_render_process_ = nullptr;
        }

        render_processes_.ClearAndBatchInsert(ps);
    }

    bool RenderManager::CheckPanelAlive(const std::vector<std::shared_ptr<ProcessInfo>>& processes) {
        for (auto& p : processes) {
            if (p->exe_full_path_.find(kGammaRayName) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    bool RenderManager::StartDesktopRenderInternal(const std::string& _work_dir, const std::string& _app_path, const std::string& args) {
        QString work_dir = QString::fromStdString(_work_dir);
        QString current_path = QString::fromStdString(std::format("{}/{} {}", _work_dir, kGammaRayRenderName, args));
        return ProcessUtil::StartProcessInSameUser(current_path.toStdWString(), work_dir.toStdWString(), false);
    }

    bool RenderManager::IsDesktopRenderAlive() const {
        return is_desktop_render_alive_;
    }

    // others
    bool RenderManager::StartRender(const std::string& _work_dir, const std::string& _app_path, const std::vector<std::string>& args) {
        std::stringstream ss;
        for (auto& arg : args) {
            ss << arg << " ";
        }
        ss << std::endl;
        LOGI("Start render \nwork_dir: {} \napp_path: {} \nargs:{}", _work_dir, _app_path, ss.str());

        auto processes = ProcessHelper::GetProcessList(false);
        CheckAliveRenders(processes);
        //todo: alive ? , return true

        QString work_dir = QString::fromStdString(_work_dir);
        QString current_path = QString::fromStdString(std::format("{}/{} {}", _work_dir, kGammaRayRenderName, ss.str()));
        return ProcessUtil::StartProcessInSameUser(current_path.toStdWString(), work_dir.toStdWString(), false);

        bool start_result = StartDesktopRenderInternal(this->desktop_work_dir_, this->desktop_app_path_, ss.str());
        processes = ProcessHelper::GetProcessList(false);
        CheckAliveRenders(processes);

        // todo: alive ? true : false

        return false;
    }

    bool RenderManager::ReStartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args) {
        return false;
    }

    bool RenderManager::StopRender(RenderProcessId id) {
        return false;
    }


}
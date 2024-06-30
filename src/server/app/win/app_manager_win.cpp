//
// Created by RGAA on 2023-12-21.
//

#include "app_manager_win.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/file_ext.h"
#include "tc_common_new/log.h"
#include "easyhook/easyhook.h"
#include "context.h"
#include "settings/settings.h"
#include "app/steam_game.h"
#include "app/app_messages.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/win32/win_helper.h"
#include <shellapi.h>
#include <filesystem>
#include <QList>
#include <QString>

#pragma comment(lib, "Shell32.lib")

namespace tc
{

    constexpr auto kInjectorName = "tc_graphics_util.exe";
    constexpr auto kX86DllName = "";
    constexpr auto kX64DllName = "tc_graphics.dll";

    AppManagerWinImpl::AppManagerWinImpl(const std::shared_ptr<Context>& ctx) : AppManager(ctx) {
        settings_ = Settings::Instance();
    }

    AppManagerWinImpl::~AppManagerWinImpl() {

    }

    void AppManagerWinImpl::Init() {
        AppManager::Init();

        steam_game_ = std::make_shared<SteamGame>(context_);
        steam_game_->RequestSteamGames();

        if (settings_->capture_.IsVideoHook()) {
            msg_listener_->Listen<MsgTimer100>([=, this](const auto &msg) {
                context_->PostTask([=, this]() {
                    this->InjectCaptureDllIfNeeded();
                    if (target_pid_ > 0) {
                        auto infos = tc::AppManagerWinImpl::SearchWindowByPid(target_pid_);
                        target_window_info_ = GetTargetWindowInfo(infos);
                    }
                });
            });
        } else {
            msg_listener_->Listen<MsgTimer2000>([=, this](const auto &msg) {
                context_->PostTask([=, this]() {
                    this->InjectCaptureDllIfNeeded();
                });
            });
        }
    }

    static std::string GetExeFolderPath() {
        char file_path[MAX_PATH + 1] = {0};
        GetModuleFileNameA(nullptr, file_path, MAX_PATH);
        (strrchr(file_path, '\\'))[0] = 0;
        return file_path;
    }

    bool AppManagerWinImpl::StartProcessWithHook() {
        auto config_exe_path = settings_->app_.game_path_;
        bool is_steam_url = settings_->app_.IsSteamUrl();
        //StringExt::ToWString(config_exe_path);
        //auto exe_path = std::filesystem::u8path(config_exe_path);
        if (!std::filesystem::exists(StringExt::ToWString(config_exe_path)) && !is_steam_url) {
            LOGE("Exe not exists: {}", config_exe_path);
            return false;
        }

        std::wstring exec = StringExt::ToWString(config_exe_path);
        std::wstring arguments = StringExt::ToWString(settings_->app_.game_arguments_);
        std::wstring x86_dll;
        std::wstring x64_dll = L"tc_graphics.dll";

        InjectParams inject_params {};
        std::string folder_path = GetExeFolderPath();
        memcpy(inject_params.host_exe_folder, folder_path.c_str(), folder_path.size());
        inject_params.listening_port = settings_->transmission_.listening_port_;
        inject_params.shm_client_to_host_buffer_size = settings_->GetShmBufferSize();
        inject_params.send_video_frame_by_shm = settings_->capture_.send_video_frame_by_shm_;

        // steam prefix
        if (is_steam_url) {
            ShellExecuteW(nullptr, nullptr, exec.c_str(), nullptr, nullptr , SW_SHOW );
            return true;
        }

#if 0
        // use easyhook to start
        auto result = RhCreateAndInject(
                const_cast<wchar_t *>(exec.data()),
                const_cast<wchar_t *>(arguments.data()),
                0,
                EASYHOOK_INJECT_DEFAULT,
                const_cast<wchar_t *>(x86_dll.data()),
                const_cast<wchar_t *>(x64_dll.data()),
                &inject_params,
                sizeof(InjectParams),
                &target_pid_
        );
        if (result >= 0) {
            LOGI("Start & Hook success...");
            return true;
        } else {
            LOGI("Start & Hook failed: {}", (int)result);
            return false;
        }
#endif
        std::vector<std::string> args;
        auto u8_exec = StringExt::ToUTF8(exec);
        auto game_args = QString::fromStdString(settings_->app_.game_arguments_);
        auto split_game_args = game_args.split(' ');
        for (const auto& arg: split_game_args) {
            if (!arg.trimmed().isEmpty()) {
                args.push_back(arg.toStdString());
            }
        }

        LOGI("we will use normal method to start, exe: {}", u8_exec);
        target_pid_ = ProcessUtil::StartProcess(u8_exec, args, true, false);
        LOGI("After started, the pid is: {}", target_pid_);
        return target_pid_ > 0;
    }

    bool AppManagerWinImpl::StartProcess() {
        BOOL ret = false;
        auto config_exe_path = settings_->app_.game_path_;
        bool is_steam_url = settings_->app_.IsSteamUrl();
        std::wstring exec = StringExt::ToWString(config_exe_path);
        std::wstring arguments = StringExt::ToWString(settings_->app_.game_arguments_);

        // steam url
        if (is_steam_url) {
            if (!steam_game_->Ready()) {
                LOGW("Steam not ready, will request again...");
                steam_game_->RequestSteamGames();
            }
            const std::wstring& target_exec = exec;
            if (config_exe_path.find("bigpicture") != std::string::npos) {
                // steam big picture mode
                // target_exec = "steam.exe steam://open/bigpicture'"
                auto big_pic_mode = "steam://open/bigpicture";
                auto steam_exe_path = std::format(R"({} {})", steam_game_->GetSteamExePath(), big_pic_mode);
                LOGI("Steam start in big picture mode, steam exe: {}, mode: {}", steam_exe_path, big_pic_mode);
                auto steam_exe_folder = steam_game_->GetSteamInstalledPath();
                ProcessUtil::StartProcessInWorkDir(steam_exe_folder, steam_exe_path, {});
            } else {
                LOGI("Steam url: {}", config_exe_path);
                ShellExecuteW(nullptr, nullptr, target_exec.c_str(), nullptr, nullptr , SW_SHOW );
            }
            return true;
        }

        if (!std::filesystem::exists(StringExt::ToWString(config_exe_path)) && !is_steam_url) {
            LOGE("Exe not exists: {}", config_exe_path);
            return false;
        }

        std::vector<std::string> args;
        auto u8_exec = StringExt::ToUTF8(exec);
        auto game_args = QString::fromStdString(settings_->app_.game_arguments_);
        auto split_game_args = game_args.split(' ');
        for (const auto& arg: split_game_args) {
            if (!arg.trimmed().isEmpty()) {
                args.push_back(arg.toStdString());
            }
        }
        LOGI("we will use normal method to start, exe: {}", u8_exec);
        target_pid_ = ProcessUtil::StartProcess(u8_exec, args, true, false);
        return ret;
    }

    void AppManagerWinImpl::InjectCaptureDllIfNeeded() {
        if (this->injected_) {
            return;
        }
        bool is_steam_url = settings_->app_.IsSteamUrl();
        if (!is_steam_url) {
            InjectCaptureDllForNormalApp();
            return;
        }
        InjectCaptureDllForSteamApp();
    }

    void AppManagerWinImpl::InjectCaptureDllForSteamApp() {
        std::vector<std::string> split_path;
        StringExt::Split(settings_->app_.game_path_, split_path, "/");
        if (split_path.empty()) {
            return;
        }
        auto steam_id = std::atoi(split_path.at(split_path.size()-1).c_str());
        if (steam_id <= 0) {
            return;
        }

        if (!steam_game_->Ready()) {
            LOGW("Steam not ready.");
            steam_game_->RequestSteamGames();
            if (!steam_game_->Ready()) {
                return;
            }
        }

        // find exe paths by steam app id
        auto installed_games = steam_game_->GetInstalledGames();
        auto it = std::find_if(installed_games.begin(), installed_games.end(), [steam_id](const SteamAppPtr& app) {
            return app->app_id_ == steam_id;
        });
        if (it == installed_games.end()) {
            return;
        }
        SteamAppPtr target_app = *it;

        // find pids by exes
        std::vector<ProcessInfoPtr> processes_info;
        for (const std::string& exe_name : target_app->exe_names_) {
            auto processes = ProcessHelper::GetProcessList();
            for (auto& process : processes) {
                auto process_exe_name = FileExt::GetFileNameFromPath(process->exe_full_path_);
                if (process_exe_name == exe_name) {
                    //LOGI("find target process exe: {}", exe_name);
                    auto ret = WinHelper::FindHwndByPid(process->pid_);
                    if (ret.ok_ && ret.value_) {
                        DWORD process_id;
                        auto thread_id = GetWindowThreadProcessId(ret.value_, &process_id);
                        process->thread_id_ = thread_id;
                        //LOGI("xxx PID:{} , TID:{}, origin PID:{}", process_id, thread_id, process.pid_);
                    }
                    processes_info.push_back(process);

                    //添加，关闭应用时直接退出所有应用
                    if (settings_->capture_.capture_video_type_ == Capture::kCaptureScreen) {
                        AddFoundPid(process);
                    }
                    break;
                }
            }
        }

        // inject it
        // 单独采集的时候才尝试hook
        if (settings_->capture_.IsVideoHook()) {
            for (const auto &process: processes_info) {
                auto result = WinHelper::IsDllInjected(process->pid_, kX86DllName, kX64DllName);
                auto process_exe_name = FileExt::GetFileNameFromPath(process->exe_full_path_);
                if (result.ok_ && result.value_) {
                    continue;
                }

                //
                AddFoundPid(process);

                // before inject
                MsgBeforeInject msg_before_inject{
                    .steam_app_ = target_app,
                    .pid_ = process->pid_,
                };
                context_->SendAppMessage(msg_before_inject);

                bool injected = InjectDll(process->pid_, 0, process->is_x86_, kX86DllName, kX64DllName);
                this->injected_ = injected;
                if (injected) {
                    LOGI("Inject success for pid: {}, exe: {}", process->pid_, process_exe_name);
                    target_pid_ = process->pid_;
                    MsgObsInjected msg_injected;
                    msg_injected.steam_app_ = target_app;
                    msg_injected.pid_ = process->pid_;
                    context_->SendAppMessage(msg_injected);
                } else {
                    LOGE("Inject capture dll failed for pid: {}, is x86:{}, exe: {}", process->pid_, process->is_x86_, process_exe_name);
                }
            }
        }
    }

    void AppManagerWinImpl::InjectCaptureDllForNormalApp() {
        if (target_pid_ <= 0) {
            //LOGW("running pid：{} is invalid for : {}", target_pid_, settings_->app_.game_path_);
            return;
        }

        auto processes = ProcessHelper::GetProcessList();
        ProcessInfoPtr target_process_info;
        for (const auto& process : processes) {
            if (process->pid_ == target_pid_) {
                target_process_info = process;
                auto process_exe_name = FileExt::GetFileNameFromPath(process->exe_full_path_);
                //LOGI("Find the pid: {}, exe : {}", process.pid_, process.exe_name_);
                break;
            }
        }
        if (!target_process_info->Valid()) {
            auto target_exe_name = FileExt::GetFileNameFromPath(settings_->app_.game_path_);
            LOGE("Can't find app to inject, pid: {}, search for by exe: {}", target_pid_, target_exe_name);
            uint32_t pid_by_exe = 0;
            for (const auto& process : processes) {
                auto process_exe_name = FileExt::GetFileNameFromPath(process->exe_full_path_);
                if (process_exe_name == target_exe_name) {
                    //LOGI("find target process exe: {}, pid: {}", target_exe_name, pid_by_exe);
                    pid_by_exe = process->pid_;
                    break;
                }
            }
            if (pid_by_exe == 0) {
                LOGE("find by exe failed, return.");
                return;
            }
            target_pid_ = pid_by_exe;
        }

        if (settings_->capture_.capture_video_type_ == Capture::kCaptureScreen) {
            AddFoundPid(target_process_info);
        }
        if (settings_->capture_.IsVideoHook()){
            // 单独采集的时候才尝试hook
            auto result = WinHelper::IsDllInjected(target_process_info->pid_, kX86DllName, kX64DllName);
            auto process_exe_name = FileExt::GetFileNameFromPath(target_process_info->exe_full_path_);
            if (result.ok_ && result.value_) {
                //LOGI("Pid: {} for: {} is already injected....", target_process_info.pid_, process_exe_name);
                return;
            } else {
                LOGI("Not injected, will inject for pid: {}, exe: {}", target_process_info->pid_, process_exe_name);
            }

            AddFoundPid(target_process_info);

            // before inject
            MsgBeforeInject msg_before_inject{
                .pid_ = target_process_info->pid_,
            };
            context_->SendAppMessage(msg_before_inject);

            bool injected = InjectDll(target_process_info->pid_, target_process_info->thread_id_,
                                      target_process_info->is_x86_, kX86DllName, kX64DllName);
            this->injected_ = injected;
            if (injected) {
                LOGI("Inject success for pid: {}, exe: {}", target_process_info->pid_, process_exe_name);
                MsgObsInjected msg_injected;
                SteamAppPtr mock_app = SteamApp::Make();
                mock_app->exes_.push_back(settings_->app_.game_path_);
                msg_injected.steam_app_ = mock_app;
                msg_injected.pid_ = target_process_info->pid_;
                context_->SendAppMessage(msg_injected);
            } else {
                LOGE("Inject capture dll failed for pid: {}, is x86:{}, exe: {}", target_process_info->pid_,
                     target_process_info->is_x86_, process_exe_name);
            }
        }
    }

    bool AppManagerWinImpl::InjectDll(uint32_t pid, uint32_t tid, bool is_x86, const std::string& x86_dll, const std::string& x64_dll) {
        // "C:/xxx/inject-helper64.exe" "C:\ProgramData\obs-studio-hook\graphics-hook64.dll" 1 40780
        auto current_exe_path = std::filesystem::current_path().string();
        auto injector_path = std::format("{}/{}", current_exe_path, kInjectorName);
        StringExt::Replace(injector_path, "\\", "/");
        auto target_dll = std::format("{}/{}", current_exe_path, x64_dll);
        StringExt::Replace(target_dll, "\\", "/");

        LOGI("Inject: {} {} pid: {}, tid: {}", injector_path, target_dll, pid, tid);

        std::vector<std::string> args;
        args.push_back(target_dll);
        args.emplace_back(tid == 0 ? "0" : "1"); // (0 / 1)
        args.push_back(std::to_string(pid));
        auto inject_result = ProcessUtil::StartProcessAndWait(injector_path, args);
        return inject_result;
    }

    void AppManagerWinImpl::Exit() {
        CloseCurrentApp();
    }

    void* AppManagerWinImpl::GetWindowHandle() {
        return target_window_info_.win_handle;
    }

    void AppManagerWinImpl::CloseCurrentApp() {
        for (const auto& pi : found_process_info_) {
            LOGI("Will kill target pid: {}, exe: {}", pi->pid_, pi->exe_full_path_);
            ProcessUtil::KillProcess(pi->pid_);
        }
    }

    WindowInfos AppManagerWinImpl::SearchWindowByPid(uint32_t pid) {
        auto infos = ProcessHelper::GetWindowInfoByPid(pid, 256);
        if (!infos.infos.empty()) {
            for (auto& info : infos.infos) {
                //LOGI("pid : {}, exe : {}, title : {}, class : {}",
                //         info.pid, StringExt::ToUTF8(info.exe_name).c_str(), StringExt::ToUTF8(info.title).c_str(), info.claxx );
                auto size = info.GetWindowSize();
            }
        }
        return infos;
    }

    WindowInfo AppManagerWinImpl::GetTargetWindowInfo(const WindowInfos& infos) {
        WindowInfo info;
        if (infos.infos.empty()) {
            return info;
        }

        for (const auto& wif : infos.infos) {
            auto size = wif.GetWindowSize();
            if (size.first <= 10 && size.second <= 10) {
                continue;
            }
            info = wif;
            break;
        }

        if (info.win_handle) {
            //LOG_INFO("Old handle : %p,", info.win_handle);
            auto handle = GetParent(info.win_handle);
            while (handle) {
                info.win_handle = handle;
                handle = GetParent(handle);
            }
            //LOG_INFO("New handle : %p,", info.win_handle);
        }

        return info;
    }

    void AppManagerWinImpl::AddFoundPid(const ProcessInfoPtr& target_pi) {
        bool exist = false;
        for (auto& pi : found_process_info_) {
            if (pi->pid_ == target_pi->pid_) {
                exist = true;
            }
        }
        if (!exist) {
            found_process_info_.push_back(target_pi);
        }
        //LOGI("found pid count: {}", found_process_info_.size());
    }

}
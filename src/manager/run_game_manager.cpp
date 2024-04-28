//
// Created by hy on 2024/4/26.
//

#include "run_game_manager.h"

#include "gr_context.h"
#include "db/db_game_manager.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/time_ext.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_message_new/tc_message.pb.h"

#include <shellapi.h>

using namespace nlohmann;

namespace tc
{

    RunGameManager::RunGameManager(const std::shared_ptr<GrContext>& ctx) {
        this->gr_ctx_ = ctx;
        this->steam_mgr_ = ctx->GetSteamManager();
        this->db_game_manager_ = ctx->GetDBGameManager();
    }

    RunGameManager::~RunGameManager() {

    }

    Response<bool, std::string> RunGameManager::StartGame(const std::string& game_path, const std::vector<std::string>& args) {
        auto resp = Response<bool, std::string>::Make(false, "");
        auto ok_resp = [&]() {
            resp.ok_ = true;
            resp.value_ = "Start success";
        };

        // big picture mode
        {
            if (game_path.find("bigpicture") != std::string::npos) {
                this->StartSteamGame(game_path, args);
                ok_resp();
                return resp;
            }
        }

        // 1. find in database
        {
            auto game = FindInDBGameManager(game_path);
            if (game != nullptr) {
                LOGI("find: {} in database", game_path);
                if (game->IsSteamGame()) {
                    LOGI("{} is a steam game.", game_path);
                    // 1.1 this is a game that installed by steam
                    this->StartSteamGame(game_path, args);
                    ok_resp();
                    return resp;
                } else {
                    LOGI("{} is a normal game", game_path);
                    // 1.2 this is a game that added by user
                    auto r = this->StartNormalGame(game_path, args);
                    LOGI("start result: {}, {}, {}", r.ok_, r.value_, r.msg_);
                    if (r.ok_) {

                    }
                    return r;
                }
            }
        }

        // 2. find in steam manager
        {
            auto game = FindInSteamManager(game_path);
            if (!game) {
                resp.value_ = "Not find game in both db & steam manager.";
                return resp;
            }
            ok_resp();
            this->StartSteamGame(game_path, args);
        }
        return resp;
    }

    bool RunGameManager::StartSteamGame(const std::string &game_path, const std::vector<std::string>& args) {
        const std::wstring& target_exec = StringExt::ToWString(game_path);
        if (game_path.find("bigpicture") != std::string::npos) {
            // steam big picture mode
            // target_exec = "steam.exe steam://open/bigpicture'"
            auto big_pic_mode = "steam://open/bigpicture";
            auto steam_exe_path = std::format(R"({} {})", this->steam_mgr_->GetSteamExePath(), big_pic_mode);
            LOGI("Steam start in big picture mode, steam exe: {}, mode: {}", steam_exe_path, big_pic_mode);
            auto steam_exe_folder = this->steam_mgr_->GetSteamInstalledPath();
            return ProcessUtil::StartProcessInWorkDir(steam_exe_folder, steam_exe_path, {});
        } else {
            LOGI("Steam url: {}", game_path);
            ShellExecuteW(nullptr, nullptr, target_exec.c_str(), nullptr, nullptr , SW_SHOW );
            return true;
        }
    }

    Response<bool, std::string> RunGameManager::StopGame(const std::string& game_path) {
        auto resp = Response<bool, std::string>::Make(false, "");

        return resp;
    }

    Response<bool, std::string> RunGameManager::StartNormalGame(const std::string& game_path, const std::vector<std::string>& args) {
        auto resp = Response<bool, std::string>::Make(false, "");
        if (!std::filesystem::exists(StringExt::ToWString(game_path))) {
            resp.value_ = std::format("Exe not exists: {}", game_path);
            LOGE(resp.value_);
            return resp;
        }

        auto pid = ProcessUtil::StartProcess(game_path, args);
        resp.ok_ = pid > 0;
        resp.value_ = "Start success";
        resp.msg_ = std::to_string(pid);
        return resp;
    }

    std::shared_ptr<SteamApp> RunGameManager::FindInSteamManager(const std::string& game_path) {
        if (!SteamManager::IsSteamPath(game_path)) {
            LOGI("path: {} is not a steam url", game_path);
            return nullptr;
        }

        auto app_id = SteamManager::ParseSteamIdFromPath(game_path);
        if (app_id.empty()) {
            LOGE("Invalid steam url: {}", game_path);
            return nullptr;
        }
        const auto& games = this->steam_mgr_->GetInstalledGames();
        for (auto& game : games) {
            if (game->app_id_ == std::atoi(app_id.c_str())) {
                return game;
            }
        }
        return nullptr;
    }

    std::shared_ptr<TcDBGame> RunGameManager::FindInDBGameManager(const std::string& game_path) {
        if (SteamManager::IsSteamPath(game_path)) {
            // find by steam id
            auto app_id_str = SteamManager::ParseSteamIdFromPath(game_path);
            auto app_id = std::atoi(app_id_str.c_str());
            if (app_id == 0) {
                LOGE("Parse steam id failed: {}", app_id_str);
                return nullptr;
            }
            return this->db_game_manager_->GetGameByGameId(app_id);

        } else {
            // find by path
            return this->db_game_manager_->GetGameByExePath(game_path);
        }
    }

    void RunGameManager::CheckRunningGame() {
        auto beg = TimeExt::GetCurrentTimestamp();
        auto running_processes = ProcessHelper::GetProcessList();
        auto db_games = this->db_game_manager_->GetAllGames();
        LOGI("running process: {}", running_processes.size());
        LOGI("db games: {}",db_games.size());

        running_games_.clear();
        for (const auto& game : db_games) {
            if (game->IsSteamGame()) {
                std::vector<uint32_t> running_pids;

                for (auto& exe : game->exes_) {
                    std::string exe_full_path = exe;
                    StringExt::Replace(exe_full_path, "\\", "/");
                    for (const auto& rp : running_processes) {
                        std::string rp_exe_full_path = rp.exe_full_path_;
                        StringExt::Replace(rp_exe_full_path, "\\", "/");
                        //std::cout << "exe_name: " << exe_full_path << ", rp exe name: " << rp_exe_full_path << "\n";
                        if (rp_exe_full_path == exe_full_path) {
                            LOGI("********Find running game: {}", exe_full_path);
                            running_pids.push_back(rp.pid_);
                            break;
                        }
                    }
                }
                if (!running_pids.empty()) {
                    auto rg = std::make_shared<RunningGame>();
                    rg->game_ = game;
                    rg->pids_ = running_pids;
                    running_games_.push_back(rg);
                }

            } else {
                // normal(added by user) game

            }
        }
        auto end = TimeExt::GetCurrentTimestamp();
        LOGI("check game alive used : {}ms", end - beg);
    }

    std::string RunGameManager::GetRunningGamesAsJson() {
        json obj = json::array();
        for (auto& rg : running_games_) {
            json item;
            item["game_id"] = rg->game_->game_id_;
            item["game_exes"] = rg->game_->game_exes_;
            obj.push_back(item);
        }
        return obj.dump(2);
    }

    std::string RunGameManager::GetRunningGamesAsProto() {
        tc::Message msg;
        msg.set_type(tc::MessageType::kOnlineGames);
        auto online_games = msg.mutable_online_games();
        for (auto& rg : running_games_) {
            auto game = online_games->Add();
            game->set_game_id(rg->game_->game_id_);
            game->set_game_exes(rg->game_->game_exes_);
        }
        return msg.SerializeAsString();
    }

    std::vector<uint32_t> RunGameManager::GetRunningGameIds() {
        std::vector<uint32_t> game_ids;
        for (auto& rg : running_games_) {
            game_ids.push_back(rg->game_->game_id_);
        }
        return game_ids;
    }

}

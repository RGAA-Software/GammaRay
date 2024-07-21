//
// Created by RGAA  on 2024/4/26.
//

#include "gr_run_game_manager.h"

#include "gr_context.h"
#include "db/db_game_manager.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_message_new/tc_message.pb.h"
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <shellapi.h>

using namespace nlohmann;

namespace tc
{

    GrRunGameManager::GrRunGameManager(const std::shared_ptr<GrContext>& ctx) {
        this->gr_ctx_ = ctx;
        this->steam_mgr_ = ctx->GetSteamManager();
        this->db_game_manager_ = ctx->GetDBGameManager();
    }

    GrRunGameManager::~GrRunGameManager() {

    }

    Response<bool, std::string> GrRunGameManager::StartGame(const std::string& game_path, const std::vector<std::string>& args) {
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

    bool GrRunGameManager::StartSteamGame(const std::string &game_path, const std::vector<std::string>& args) {
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

    Response<bool, std::string> GrRunGameManager::StopGame(const std::string& game_id) {
        auto resp = Response<bool, std::string>::Make(false, "");
        if (game_process_) {
            game_process_->kill();
            game_process_ = nullptr;
        }

        for (const auto& running_game : running_games_ ) {
            if (std::to_string(running_game->game_->game_id_) == game_id) {
                for (const auto& pid : running_game->pids_) {
                    ProcessUtil::KillProcess(pid);
                    LOGI("Kill {} for pid: {}", running_game->game_->game_name_, pid);
                }
                break;
            }
        }
        return resp;
    }

    Response<bool, std::string> GrRunGameManager::StartNormalGame(const std::string& game_path, const std::vector<std::string>& args) {
        auto resp = Response<bool, std::string>::Make(false, "");
        if (!std::filesystem::exists(StringExt::ToWString(game_path))) {
            resp.value_ = std::format("Exe not exists: {}", game_path);
            LOGE(resp.value_);
            return resp;
        }

        if (game_process_) {
            game_process_->kill();
        }
        game_process_ = new QProcess();
        game_process_->start(game_path.c_str());
        game_process_->waitForStarted();
        auto pid = game_process_->processId();
        resp.ok_ = pid > 0;
        resp.value_ = "Start success";
        resp.msg_ = std::to_string(pid);
        return resp;
    }

    std::shared_ptr<SteamApp> GrRunGameManager::FindInSteamManager(const std::string& game_path) {
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

    std::shared_ptr<TcDBGame> GrRunGameManager::FindInDBGameManager(const std::string& game_path) {
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

    void GrRunGameManager::CheckRunningGame() {
        auto beg = TimeExt::GetCurrentTimestamp();
        running_processes_ = ProcessHelper::GetProcessList();
        auto db_games = this->db_game_manager_->GetAllGames();
        LOGI("running process: {}", running_processes_.size());
        LOGI("db games: {}",db_games.size());
        for (auto& rp : running_processes_) {
            if (rp->icon_) {
                QString icons_path = qApp->applicationDirPath() + "/www/icons";
                QDir dir;
                if (!dir.exists(icons_path)) {
                    dir.mkdir(icons_path);
                }
                rp->icon_name_ = MD5::Hex(rp->exe_full_path_) + ".png";
                auto icon_file_path = icons_path + "/" + rp->icon_name_.c_str();
                if (QFile::exists(icon_file_path)) {
                    continue;
                }
                auto image = QImage::fromHICON(rp->icon_);
                auto pixmap = QPixmap::fromImage(image);
                pixmap.save(icon_file_path);
            }
        }

        running_games_.clear();
        for (const auto& game : db_games) {
            std::vector<uint32_t> running_pids;
            for (const std::string& exe : game->exes_) {
                std::string exe_full_path = exe;
                StringExt::Replace(exe_full_path, "\\", "/");
                for (const auto& rp : running_processes_) {
                    std::string rp_exe_full_path = rp->exe_full_path_;
                    StringExt::Replace(rp_exe_full_path, "\\", "/");
                    if (rp_exe_full_path == exe_full_path) {
                        LOGI("********Found running game: {}", exe_full_path);
                        running_pids.push_back(rp->pid_);
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
        }
        auto end = TimeExt::GetCurrentTimestamp();
        LOGI("check game alive used : {}ms", end - beg);
    }

    std::string GrRunGameManager::GetRunningGamesAsJson() {
        json obj = json::array();
        for (auto& rg : running_games_) {
            json item;
            item["game_id"] = rg->game_->game_id_;
            item["game_exes"] = rg->game_->game_exes_;
            obj.push_back(item);
        }
        return obj.dump(2);
    }

    std::string GrRunGameManager::GetRunningGamesAsProto() {
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

    std::vector<uint64_t> GrRunGameManager::GetRunningGameIds() {
        std::vector<uint64_t> game_ids;
        for (auto& rg : running_games_) {
            game_ids.push_back(rg->game_->game_id_);
        }
        return game_ids;
    }

    std::vector<std::shared_ptr<ProcessInfo>> GrRunGameManager::GetRunningProcesses() {
        return running_processes_;
    }

}

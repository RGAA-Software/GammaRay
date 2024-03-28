#include "game_model.h"
#include "tc_common_new/log.h"
#include "tc_steam_manager_new/steam_entities.h"

#include <shellapi.h>

namespace tc
{

    GameModel::GameModel(QObject *parent) {

    }

    int GameModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent)
        return games_.length();
    }

    QVariant GameModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid() || index.row() < 0)
            return QVariant();

        if (index.row() >= games_.count()) {
            qWarning() << "SatelliteModel: Index out of bound";
            return QVariant();
        }

        auto &game = games_.at(index.row());
        switch (role) {
            case NameRole:
                return game->name_.c_str();
            case SteamUrlRole:
                return game->steam_url_.c_str();
            case AppIdRole:
                return game->app_id_;
            case InstalledRole:
                return game->is_installed_;
            case CoverUrlRole:
                return game->cover_url_.c_str();
            default:
                break;
        }
        return "";
    }

    QHash<int, QByteArray> GameModel::roleNames() const {
        QHash<int, QByteArray> roleNames;
        roleNames.insert(NameRole, "NameRole");
        roleNames.insert(SteamUrlRole, "SteamUrlRole");
        roleNames.insert(AppIdRole, "AppIdRole");
        roleNames.insert(InstalledRole, "InstalledRole");
        roleNames.insert(CoverUrlRole, "CoverUrlRole");
        return roleNames;
    }

    Q_INVOKABLE void GameModel::AddGame(const std::shared_ptr<SteamApp> &game) {
        beginResetModel();
        games_.append(game);
        endResetModel();
    }

    Q_INVOKABLE void GameModel::OnItemClicked(int app_id) {
        LOGI("Clicked: {}", app_id);
        for (auto& game : games_) {
            if (game->app_id_ == app_id) {
                LOGI("will start: {}", game->steam_url_);
                ShellExecuteW(0, 0, QString::fromStdString(game->steam_url_).toStdWString().c_str(), 0, 0 , SW_SHOW );
            }
        }
    }

}

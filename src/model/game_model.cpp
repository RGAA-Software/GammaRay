#include "game_model.h"
#include "tc_common_new/log.h"

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
                return game.name_;
            case SteamUrlRole:
                return game.steam_url_;
            case AppIdRole:
                return game.app_id_;
            case InstalledRole:
                return game.installed_;
            case CoverUrlRole:
                return game.cover_url_;
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

    Q_INVOKABLE void GameModel::AddGame(const Game &game) {
        beginResetModel();
        games_.append(game);
        endResetModel();
    }

    Q_INVOKABLE void GameModel::OnItemClicked(int app_id) {
        LOGI("Clicked: {}", app_id);
        for (auto& game : games_) {
            if (game.app_id_ == app_id) {
                LOGI("will start: {}", game.steam_url_.toStdString());
                ShellExecuteW(0, 0, game.steam_url_.toStdWString().c_str(), 0, 0 , SW_SHOW );
            }
        }
    }

}

#include "game_model.h"
#include "tc_common/log.h"

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
        return roleNames;
    }

    Q_INVOKABLE void GameModel::AddGame(const Game &game) {
        beginResetModel();
        games_.append(game);
        endResetModel();
    }

    Q_INVOKABLE void GameModel::OnItemClicked(int app_id) {
        LOGI("Clicked: {}", app_id);
    }

}

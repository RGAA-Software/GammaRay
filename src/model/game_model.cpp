#include "game_model.h"

namespace tc
{

GameModel::GameModel(QObject* parent) {

}

int GameModel::rowCount(const QModelIndex& parent) const {
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

    auto& game = games_.at(index.row());
    switch (role)
    {
    case NameRole:
        return game.name_;
    case SteamUrlRole:
        return game.steam_url_;
    default:
        break;
    }
}

QHash<int, QByteArray> GameModel::roleNames() const {
    QHash<int, QByteArray> roleNames;
    roleNames.insert(NameRole, "NameRole");
    roleNames.insert(SteamUrlRole, "SteamUrlRole");
    return roleNames;
}

Q_INVOKABLE void GameModel::AddGame(const QString& name, int age) {
    beginResetModel();
    auto game = Game();
    game.name_ = name;
    games_.append(game);
    endResetModel();
}

}

#pragma once

#include <QString>
#include <QAbstractListModel>
#include <QList>

#include "src/entity/game.h"

namespace tc
{

class GameModel : public QAbstractListModel {
    Q_OBJECT
public:

    enum Roles {
        NameRole = Qt::UserRole + 1,
        SteamUrlRole,
        AppIdRole,
        InstalledRole,
        CoverUrlRole,
        RunningRole,
        UpdatingRole,
    };

    explicit GameModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void AddGame(const Game& name);
    Q_INVOKABLE void OnItemClicked(int app_id);

private:

    QList<Game> games_;

};

}

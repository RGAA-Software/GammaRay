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

    enum Roles{
        NameRole = Qt::UserRole + 1,
        SteamUrlRole
    };

    GameModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void AddGame(const QString& name, int age);

private:

    QList<Game> games_;

};

}

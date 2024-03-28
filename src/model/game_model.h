#pragma once

#include <QString>
#include <QAbstractListModel>
#include <QList>

namespace tc
{

    class SteamApp;

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

        explicit GameModel(QObject *parent = nullptr);

        [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

        [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        Q_INVOKABLE void AddGame(const std::shared_ptr<SteamApp>& name);

        Q_INVOKABLE void OnItemClicked(int app_id);

    private:

        QList<std::shared_ptr<SteamApp>> games_;

    };

}

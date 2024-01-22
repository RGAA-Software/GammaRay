#pragma once

#include <QString>

namespace tc
{

    class Game {
    public:
        static Game Make(const QString& name,
                         int app_id,
                         int installed,
                         int running,
                         int update);

        static Game Make(int app_id,
                         const QString& name,
                         const QString& installed_path);

        std::string Dump();

    public:
        QString name_;
        QString steam_url_;
        QString installed_path_;
        QString cover_url_;
        int app_id_;
        int installed_;
        int running_;
        int updating_;
    };

}

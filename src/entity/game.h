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

        std::string Dump();

    public:
        QString name_;
        QString steam_url_;
        int app_id_;
        int installed_;
        int running_;
        int updating_;
    };

}

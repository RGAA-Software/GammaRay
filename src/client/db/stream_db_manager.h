//
// Created by RGAA on 2023-08-17.
//

#ifndef SAILFISH_CLIENT_PC_STREAMDBMANAGER_H
#define SAILFISH_CLIENT_PC_STREAMDBMANAGER_H

#include <any>
#include <vector>
#include <memory>
#include <string>

namespace tc
{

    class StreamItem;

    class StreamDBManager {
    public:

        StreamDBManager();

        ~StreamDBManager();

        static std::string GenUUID();

        void AddStream(StreamItem &stream);

        void UpdateStream(const StreamItem &stream);

        std::vector<StreamItem> GetAllStreams();

        void DeleteStream(int id);

        int RandomColor();

    private:

        void CreateTables();

    private:

        std::any db_storage;

    };

}

#endif //SAILFISH_CLIENT_PC_STREAMDBMANAGER_H

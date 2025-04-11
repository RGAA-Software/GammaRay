//
// Created by RGAA on 2023-08-17.
//

#ifndef SAILFISH_CLIENT_PC_STREAMDBMANAGER_H
#define SAILFISH_CLIENT_PC_STREAMDBMANAGER_H

#include <any>
#include <vector>
#include <memory>
#include <string>
#include <optional>

namespace tc
{

    class StreamItem;

    class StreamDBManager {
    public:

        StreamDBManager();
        ~StreamDBManager();
        static std::string GenUUID();
        void AddStream(StreamItem &stream);
        bool UpdateStream(const StreamItem &stream);
        bool UpdateStreamRandomPwd(const std::string& stream_id, const std::string& random_pwd);
        std::vector<StreamItem> GetAllStreams();
        std::vector<StreamItem> GetAllStreamsSortByCreatedTime(bool increase = false);
        std::vector<StreamItem> GetStreamsSortByCreatedTime(int page, int page_size, bool increase = false);
        std::optional<StreamItem> GetStream(const std::string& stream_id);
        void DeleteStream(int id);
        int RandomColor();
        bool HasStream(const std::string& stream_id);

    private:

        void CreateTables();

    private:

        std::any db_storage_;

    };

}

#endif //SAILFISH_CLIENT_PC_STREAMDBMANAGER_H

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

namespace spvr
{
    class SpvrStream;
}

namespace tc
{
    class GrDatabase;

    class StreamDBOperator {
    public:

        StreamDBOperator(const std::shared_ptr<GrDatabase>& db);
        ~StreamDBOperator();
        static std::string GenUUID();
        void AddStream(const std::shared_ptr<spvr::SpvrStream>& stream);
        bool UpdateStream(std::shared_ptr<spvr::SpvrStream> stream);
        bool UpdateStreamRandomPwd(const std::string& stream_id, const std::string& random_pwd);
        bool UpdateStreamSafetyPwd(const std::string& stream_id, const std::string& safety_pwd);
        //std::vector<spvr::SpvrStream> GetAllStreams();
        std::vector<std::shared_ptr<spvr::SpvrStream>> GetAllStreamsSortByCreatedTime(bool increase = false);
        std::vector<std::shared_ptr<spvr::SpvrStream>> GetStreamsSortByCreatedTime(int page, int page_size, bool increase = false);
        std::optional<std::shared_ptr<spvr::SpvrStream>> GetStreamByStreamId(const std::string& stream_id);
        std::optional<std::shared_ptr<spvr::SpvrStream>> GetStreamByHostPort(const std::string& host, int port);
        std::optional<std::shared_ptr<spvr::SpvrStream>> GetStreamByRemoteDeviceId(const std::string& remote_device_id);
        void DeleteStream(int id);
        int RandomColor();
        bool HasStream(const std::string& stream_id);
        void Clear();

    private:
        //void CreateTables();

    private:
        std::shared_ptr<GrDatabase> db_ = nullptr;

    };

}

#endif //SAILFISH_CLIENT_PC_STREAMDBMANAGER_H

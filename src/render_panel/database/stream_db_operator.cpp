//
// Created by RGAA on 2023-08-17.
//

#include "render_panel/database/stream_db_operator.h"

#include <QUuid>
#include <QRandomGenerator>
#include <QApplication>
extern "C" {
#include <sqlite3.h>
}
#include <sqlite_orm/sqlite_orm.h>

#include <vector>
#include <algorithm>

#include "gr_database.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/time_util.h"
#include "tc_spvr_client/spvr_stream.h"

using namespace sqlite_orm;

namespace tc
{

    StreamDBOperator::StreamDBOperator(const std::shared_ptr<GrDatabase>& db) {
        db_ = db;
        //CreateTables();
    }

    StreamDBOperator::~StreamDBOperator() {

    }

    void StreamDBOperator::AddStream(const std::shared_ptr<spvr::SpvrStream>& stream) {
        if (stream->stream_id_.empty()) {
            stream->stream_id_ = GenUUID();
        }
        //stream.stream_id = MD5::Hex(stream.stream_id);
        stream->bg_color_ = RandomColor();
        stream->created_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        stream->updated_timestamp_ = stream->created_timestamp_;
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.insert(*stream);
    }

    bool StreamDBOperator::HasStream(const std::string& stream_id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<spvr::SpvrStream>(where(c(&spvr::SpvrStream::stream_id_) == stream_id));
        return !streams.empty();
    }

    bool StreamDBOperator::UpdateStream(std::shared_ptr<spvr::SpvrStream> stream) {
        stream->updated_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<spvr::SpvrStream>(where(c(&spvr::SpvrStream::stream_id_) == stream->stream_id_));
        if (streams.size() == 1) {
            storage.update(*stream);
        }
        return true;
    }

    bool StreamDBOperator::UpdateStreamRandomPwd(const std::string& stream_id, const std::string& random_pwd) {
        auto opt_stream = GetStreamByStreamId(stream_id);
        if (!opt_stream.has_value()) {
            return false;
        }
        auto stream = opt_stream.value();
        stream->updated_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        stream->remote_device_random_pwd_ = random_pwd;

        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<spvr::SpvrStream>(where(c(&spvr::SpvrStream::stream_id_) == stream_id));
        if (streams.size() == 1) {
            storage.update(*stream);
        }
        return true;
    }

    bool StreamDBOperator::UpdateStreamSafetyPwd(const std::string& stream_id, const std::string& safety_pwd) {
        auto opt_stream = GetStreamByStreamId(stream_id);
        if (!opt_stream.has_value()) {
            return false;
        }
        auto stream = opt_stream.value();
        stream->updated_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        stream->remote_device_safety_pwd_ = safety_pwd;

        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<spvr::SpvrStream>(where(c(&spvr::SpvrStream::stream_id_) == stream_id));
        if (streams.size() == 1) {
            storage.update(*stream);
        }
        return true;
    }

    std::optional<std::shared_ptr<spvr::SpvrStream>> StreamDBOperator::GetStreamByStreamId(const std::string& stream_id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all_pointer<spvr::SpvrStream>(where(c(&spvr::SpvrStream::stream_id_) == stream_id));
        if (streams.empty()) {
            return std::nullopt;
        }
        auto target_stream = std::move(streams[0]);
        return target_stream;
    }

    std::optional<std::shared_ptr<spvr::SpvrStream>> StreamDBOperator::GetStreamByHostPort(const std::string& host, int port) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all_pointer<spvr::SpvrStream>(where(c(&spvr::SpvrStream::stream_host_) == host and c(&spvr::SpvrStream::stream_port_) == port));
        if (streams.empty()) {
            return std::nullopt;
        }
        auto target_stream = std::move(streams[0]);
        return target_stream;
    }

    std::optional<std::shared_ptr<spvr::SpvrStream>> StreamDBOperator::GetStreamByRemoteDeviceId(const std::string& remote_device_id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all_pointer<spvr::SpvrStream>(where(c(&spvr::SpvrStream::remote_device_id_) == remote_device_id));
        if (streams.empty()) {
            return std::nullopt;
        }
        std::shared_ptr<spvr::SpvrStream> target_stream = std::move(streams[0]);
        return target_stream;
    }

//    std::vector<spvr::SpvrStream> StreamDBManager::GetAllStreams() {
//        using Storage = decltype(db_->GetStorageTypeValue());
//        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
//        return storage.get_all<spvr::SpvrStream>();
//    }

    std::vector<std::shared_ptr<spvr::SpvrStream>> StreamDBOperator::GetAllStreamsSortByCreatedTime(bool increase) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto unique_streams = storage.get_all_pointer<spvr::SpvrStream>([=]() -> auto {
            if (increase) {
                return order_by(&spvr::SpvrStream::created_timestamp_);
            } else {
                return order_by(&spvr::SpvrStream::created_timestamp_).desc();
            }
        }());

        std::vector<std::shared_ptr<spvr::SpvrStream>> streams;
        for (auto& st : unique_streams) {
            streams.push_back(std::move(st));
        }
        return streams;
    }

    std::vector<std::shared_ptr<spvr::SpvrStream>> StreamDBOperator::GetStreamsSortByCreatedTime(int page, int page_size, bool increase) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        int offset_size = (page - 1) * page_size;
        auto unique_streams = storage.get_all_pointer<spvr::SpvrStream>(
            where(length(&spvr::SpvrStream::remote_device_id_) > 1),
            order_by(&spvr::SpvrStream::created_timestamp_).desc(),
            limit(page_size, offset(offset_size))
        );

        std::vector<std::shared_ptr<spvr::SpvrStream>> streams;
        for (auto& ust : unique_streams) {
            streams.push_back(std::move(ust));
        }
        return streams;
    }

    void StreamDBOperator::DeleteStream(int id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.remove<spvr::SpvrStream>(id);
    }

    std::string StreamDBOperator::GenUUID() {
        QUuid id = QUuid::createUuid();
        QString str_id = id.toString();
        str_id.remove("{").remove("}").remove("-");
        return str_id.toStdString();
    }

    int StreamDBOperator::RandomColor() {
        // Colors form [Claude Monet]'s arts
        static std::vector<int> colors = {
            0xBFC8D7, 0xE2D2D2, 0xE3E2B4, 0xA2B59F,
            0xF7EAE2, 0xEADB80, 0xAEDDEF, 0xE1B4D3,
            0xE1F1E7, 0xB2D3C5, 0xCFDD8E, 0xE4BEB3,
            0xF2EEE5, 0xE5C1C5, 0xC3E2DD, 0x6ECEDA,
            0xf6a288, 0x2d4a91, 0xf3dfcb, 0x112046,
            0x63B38E, 0x9F7857, 0xFEA087, 0xF6DFC0,
            0xE49A15, 0xffffd2, 0xfcbad3, 0xaa96da,
            0x61c0bf, 0xbbded6, 0xfae3d9, 0xffb6b9

        };
        int random_idx = QRandomGenerator::global()->bounded(0, (int) colors.size());
        return colors.at(random_idx);
    }
}

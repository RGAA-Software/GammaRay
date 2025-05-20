//
// Created by RGAA on 2023-08-17.
//

#include "stream_db_manager.h"

#include <QUuid>
#include <QRandomGenerator>
#include <QApplication>
#include <sqlite_orm/sqlite_orm.h>

#include <vector>
#include <sqlite3.h>
#include <algorithm>

#include "stream_item.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/time_util.h"

using namespace sqlite_orm;

namespace tc
{

    StreamDBManager::StreamDBManager() {
        CreateTables();
    }

    StreamDBManager::~StreamDBManager() {

    }

    static auto BindAppDatabase(const std::string &name) {
        auto st = make_storage(name, make_table("stream",
            make_column("id", &StreamItem::_id, primary_key()),
            make_column("stream_id", &StreamItem::stream_id_),
            make_column("stream_name", &StreamItem::stream_name_),
            make_column("encode_bps", &StreamItem::encode_bps_),
            make_column("audio_enabled", &StreamItem::audio_enabled_, default_value(0)),
            make_column("clipboard_enabled", &StreamItem::clipboard_enabled_, default_value(0)),
            make_column("only_viewing", &StreamItem::only_viewing_, default_value(0)),
            make_column("show_max_window", &StreamItem::show_max_window_, default_value(0)),
            make_column("split_windows", &StreamItem::split_windows_, default_value(0)),
            make_column("audio_capture_mode", &StreamItem::audio_capture_mode_),
            make_column("stream_host", &StreamItem::stream_host_),
            make_column("stream_port", &StreamItem::stream_port_),
            make_column("bg_color", &StreamItem::bg_color_),
            make_column("encode_fps", &StreamItem::encode_fps_),
            make_column("network_type", &StreamItem::network_type_),
            make_column("connect_type", &StreamItem::connect_type_),
            make_column("device_id", &StreamItem::device_id_),
            make_column("device_random_pwd", &StreamItem::device_random_pwd_),
            make_column("device_safety_pwd", &StreamItem::device_safety_pwd_),
            make_column("remote_device_id", &StreamItem::remote_device_id_),
            make_column("remote_device_random_pwd", &StreamItem::remote_device_random_pwd_),
            make_column("remote_device_safety_pwd", &StreamItem::remote_device_safety_pwd_),
            make_column("created_timestamp", &StreamItem::created_timestamp_, default_value(0)),
            make_column("updated_timestamp", &StreamItem::updated_timestamp_, default_value(0)),
            make_column("enable_p2p", &StreamItem::enable_p2p_, default_value(0)),
            make_column("desktop_name", &StreamItem::desktop_name_),
            make_column("os_version", &StreamItem::os_version_)
        ));
        if (!name.empty()) {
            auto r = st.sync_schema(true);
            for (const auto& [key, value] : r) {
                LOGI("SYNC db {} schema: {} => {}", name, key, (int)value);
            }
        }
        return st;
    }

    static auto GetStorageTypeValue() {
        return BindAppDatabase("");
    }

    void StreamDBManager::CreateTables() {
        auto db_path = qApp->applicationDirPath() + "/gr_data/gr_streams.db";
        // 2. bind
        db_storage_ = BindAppDatabase(db_path.toStdString());
//        using Storage = decltype(GetStorageTypeValue());
//        auto storage = std::any_cast<Storage>(db_storage);
//        storage.sync_schema(true);
    }

    void StreamDBManager::AddStream(const std::shared_ptr<StreamItem>& stream) {
        if (stream->stream_id_.empty()) {
            stream->stream_id_ = GenUUID();
        }
        //stream.stream_id = MD5::Hex(stream.stream_id);
        stream->bg_color_ = RandomColor();
        stream->created_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        stream->updated_timestamp_ = stream->created_timestamp_;
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        storage.insert(*stream);
    }

    bool StreamDBManager::HasStream(const std::string& stream_id) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto streams = storage.get_all<StreamItem>(where(c(&StreamItem::stream_id_) == stream_id));
        return !streams.empty();
    }

    bool StreamDBManager::UpdateStream(std::shared_ptr<StreamItem> stream) {
        stream->updated_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto streams = storage.get_all<StreamItem>(where(c(&StreamItem::stream_id_) == stream->stream_id_));
        if (streams.size() == 1) {
            storage.update(*stream);
        }
        return true;
    }

    bool StreamDBManager::UpdateStreamRandomPwd(const std::string& stream_id, const std::string& random_pwd) {
        auto opt_stream = GetStream(stream_id);
        if (!opt_stream.has_value()) {
            return false;
        }
        auto stream = opt_stream.value();
        stream->updated_timestamp_ = TimeUtil::GetCurrentTimestamp();
        stream->remote_device_random_pwd_ = random_pwd;

        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto streams = storage.get_all<StreamItem>(where(c(&StreamItem::stream_id_) == stream_id));
        if (streams.size() == 1) {
            storage.update(*stream);
        }
        return true;
    }

    std::optional<std::shared_ptr<StreamItem>> StreamDBManager::GetStream(const std::string& stream_id) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto streams = storage.get_all_pointer<StreamItem>(where(c(&StreamItem::stream_id_) == stream_id));
        if (streams.empty()) {
            return std::nullopt;
        }
        auto target_stream = std::move(streams[0]);
        return target_stream;
    }

    std::optional<std::shared_ptr<StreamItem>> StreamDBManager::GetStream(const std::string& host, int port) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto streams = storage.get_all_pointer<StreamItem>(where(c(&StreamItem::stream_host_) == host and c(&StreamItem::stream_port_) == port));
        if (streams.empty()) {
            return std::nullopt;
        }
        auto target_stream = std::move(streams[0]);
        return target_stream;
    }

    std::optional<std::shared_ptr<StreamItem>> StreamDBManager::GetStreamByRemoteDeviceId(const std::string& remote_device_id) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto streams = storage.get_all_pointer<StreamItem>(where(c(&StreamItem::remote_device_id_) == remote_device_id));
        if (streams.empty()) {
            return std::nullopt;
        }
        std::shared_ptr<StreamItem> target_stream = std::move(streams[0]);
        return target_stream;
    }

//    std::vector<StreamItem> StreamDBManager::GetAllStreams() {
//        using Storage = decltype(GetStorageTypeValue());
//        auto storage = std::any_cast<Storage>(db_storage_);
//        return storage.get_all<StreamItem>();
//    }

    std::vector<std::shared_ptr<StreamItem>> StreamDBManager::GetAllStreamsSortByCreatedTime(bool increase) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        auto unique_streams = storage.get_all_pointer<StreamItem>([=]() -> auto {
            if (increase) {
                return order_by(&StreamItem::created_timestamp_);
            } else {
                return order_by(&StreamItem::created_timestamp_).desc();
            }
        }());

        std::vector<std::shared_ptr<StreamItem>> streams;
        for (auto& st : unique_streams) {
            streams.push_back(std::move(st));
        }
        return streams;
    }

    std::vector<std::shared_ptr<StreamItem>> StreamDBManager::GetStreamsSortByCreatedTime(int page, int page_size, bool increase) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        int offset_size = (page - 1) * page_size;
        auto unique_streams = storage.get_all_pointer<StreamItem>(
            where(c(&StreamItem::network_type_) == "relay"),
            order_by(&StreamItem::created_timestamp_).desc(),
            limit(page_size, offset(offset_size))
        );

        std::vector<std::shared_ptr<StreamItem>> streams;
        for (auto& ust : unique_streams) {
            streams.push_back(std::move(ust));
        }
        return streams;
    }

    void StreamDBManager::DeleteStream(int id) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage_);
        storage.remove<StreamItem>(id);
    }

    std::string StreamDBManager::GenUUID() {
        QUuid id = QUuid::createUuid();
        QString str_id = id.toString();
        str_id.remove("{").remove("}").remove("-");
        return str_id.toStdString();
    }

    int StreamDBManager::RandomColor() {
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

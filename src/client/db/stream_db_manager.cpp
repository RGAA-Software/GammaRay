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

#include "stream_item.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"

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
            make_column("stream_id", &StreamItem::stream_id),
            make_column("stream_name", &StreamItem::stream_name),
            make_column("encode_bps", &StreamItem::encode_bps),
            make_column("audio_enabled", &StreamItem::audio_enabled),
            make_column("audio_capture_mode", &StreamItem::audio_capture_mode),
            make_column("stream_host", &StreamItem::stream_host),
            make_column("stream_port", &StreamItem::stream_port),
            make_column("bg_color", &StreamItem::bg_color),
            make_column("encode_fps", &StreamItem::encode_fps),
            make_column("network_type", &StreamItem::network_type_),
            make_column("connect_type", &StreamItem::connect_type_),
            make_column("client_id", &StreamItem::client_id_),
            make_column("client_random_pwd", &StreamItem::client_random_pwd_),
            make_column("client_safety_pwd", &StreamItem::client_safety_pwd_)
        ));
        return st;
    }

    static auto GetStorageTypeValue() {
        return BindAppDatabase("");
    }

    void StreamDBManager::CreateTables() {
        auto db_path = qApp->applicationDirPath() + "/gr_data/stream.db";
        // 2. bind
        db_storage = BindAppDatabase(db_path.toStdString());
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
        storage.sync_schema();
    }

    void StreamDBManager::AddStream(StreamItem &stream) {
        if (stream.stream_id.empty()) {
            stream.stream_id = GenUUID();
        }
        stream.stream_id = MD5::Hex(stream.stream_id);
        stream.bg_color = RandomColor();
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
        storage.insert(stream);
    }

    bool StreamDBManager::HasStream(const std::string& stream_id) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
        auto streams = storage.get_all<StreamItem>(where(c(&StreamItem::stream_id) == stream_id));
        return !streams.empty();
    }

    void StreamDBManager::UpdateStream(const StreamItem &stream) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
        auto streams = storage.get_all<StreamItem>(where(c(&StreamItem::stream_id) == stream.stream_id));
        if (streams.size() == 1) {
            storage.update(stream);
        }
    }

    std::vector<StreamItem> StreamDBManager::GetAllStreams() {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
        return storage.get_all<StreamItem>();
    }

    void StreamDBManager::DeleteStream(int id) {
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
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

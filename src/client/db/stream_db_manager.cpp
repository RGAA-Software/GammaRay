//
// Created by RGAA on 2023-08-17.
//

#include "stream_db_manager.h"

#include <QUuid>
#include <QRandomGenerator>
#include <sqlite_orm/sqlite_orm.h>

#include <vector>
#include <sqlite3.h>

#include "stream_item.h"
#include "tc_common_new/log.h"

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
            make_column("stream_target", &StreamItem::stream_target),
            make_column("monitor_capture_method", &StreamItem::monitor_capture_method),
            make_column("exe_path", &StreamItem::exe_path),
            make_column("capture_mode", &StreamItem::capture_mode),
            make_column("encoder_type", &StreamItem::encoder_type),
            make_column("encoder_hw", &StreamItem::encoder_hw),
            make_column("encode_bps", &StreamItem::encode_bps),
            make_column("audio_enabled", &StreamItem::audio_enabled),
            make_column("audio_capture_mode", &StreamItem::audio_capture_mode),
            make_column("gpu_router_enabled", &StreamItem::gpu_router_enabled),
            make_column("gpu_router_policy", &StreamItem::gpu_router_policy),
            make_column("frame_resize_enabled", &StreamItem::frame_resize_enabled),
            make_column("frame_resize_width", &StreamItem::frame_resize_width),
            make_column("frame_resize_height", &StreamItem::frame_resize_height),
            make_column("replay_mode", &StreamItem::replay_mode),
            make_column("stream_host", &StreamItem::stream_host),
            make_column("stream_port", &StreamItem::stream_port),
            make_column("app_args", &StreamItem::app_args),
            make_column("auto_exit", &StreamItem::auto_exit),
            make_column("auto_exit_period", &StreamItem::auto_exit_period),
            make_column("enable_multi_players", &StreamItem::enable_multi_players),
            make_column("bg_color", &StreamItem::bg_color),
            make_column("encode_fps", &StreamItem::encode_fps)
        ));
        return st;
    }

    static auto GetStorageTypeValue() {
        return BindAppDatabase("");
    }

    void StreamDBManager::CreateTables() {
        // 1. create database if needed
        sqlite3 *db;
        auto rc = sqlite3_open("stream.db", &db);
        if (rc) {
            LOGE("Create stream.db failed !");
            return;
        }

        char *msg = nullptr;
        auto sql_stream_table = R"(
			CREATE TABLE IF NOT EXISTS stream (
				id                     INTEGER PRIMARY KEY AUTOINCREMENT,
				stream_id			   TEXT,
				stream_name            TEXT,
				stream_target          TEXT,
				monitor_capture_method TEXT,
				exe_path               TEXT,
				capture_mode           TEXT,
				encoder_type           TEXT,
				encoder_hw             TEXT,
				encode_bps             INTEGER,
				audio_enabled          INTEGER,
				audio_capture_mode     TEXT,
				gpu_router_enabled     INTEGER,
				gpu_router_policy      TEXT,
				frame_resize_enabled   INTEGER,
				frame_resize_width     INTEGER,
				frame_resize_height    INTEGER,
				replay_mode            TEXT,
                stream_host            TEXT,
				stream_port            INTEGER,
				app_args               TEXT,
				auto_exit              INTEGER,
				auto_exit_period       INTEGER,
				enable_multi_players   INTEGER,
                bg_color               INTEGER,
                encode_fps             INTEGER
			);
		)";

        rc = sqlite3_exec(db, sql_stream_table,
                          [](void *NotUsed, int argc, char **argv, char **col_name) -> int { return 0; }, 0, &msg);
        if (rc) {
            LOGE("Create table failed : {}", msg);
            return;
        }

        sqlite3_close(db);

        // 2. bind
        db_storage = BindAppDatabase("stream.db");
    }

    void StreamDBManager::AddStream(StreamItem &stream) {
        stream.stream_id = GenUUID();
        stream.bg_color = RandomColor();
        using Storage = decltype(GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_storage);
        storage.insert(stream);
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
        };
        int random_idx = QRandomGenerator::global()->bounded(0, (int) colors.size());
        return colors.at(random_idx);
    }
}

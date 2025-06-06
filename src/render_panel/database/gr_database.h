//
// Created by RGAA on 29/05/2025.
//

#ifndef GAMMARAY_GR_DATABASE_H
#define GAMMARAY_GR_DATABASE_H

#include <any>
#include <memory>
#include <sqlite_orm/sqlite_orm.h>
#include "db_game.h"
#include "visit_record.h"
#include "file_transfer_record.h"
#include "render_panel/database/stream_item.h"

using namespace sqlite_orm;

namespace tc
{

    class GrContext;
    class StreamDBOperator;
    class DBGameOperator;
    class VisitRecordOperator;
    class FileTransferRecordOperator;

    class GrDatabase : public std::enable_shared_from_this<GrDatabase> {
    public:
        explicit GrDatabase(const std::shared_ptr<GrContext>& ctx);
        bool Init();
        auto InitAppDatabase(const std::string& name) {
            auto st = make_storage(name,
                make_table("games",
                    make_column("id", &TcDBGame::id_, primary_key().autoincrement()),
                    make_column("game_id", &TcDBGame::game_id_),
                    make_column("game_name", &TcDBGame::game_name_),
                    make_column("game_installed_dir", &TcDBGame::game_installed_dir_),
                    make_column("game_exes", &TcDBGame::game_exes_),
                    make_column("game_exe_names", &TcDBGame::game_exe_names_),
                    make_column("is_installed", &TcDBGame::is_installed_),
                    make_column("steam_url", &TcDBGame::steam_url_),
                    make_column("cover_name", &TcDBGame::cover_name_),
                    make_column("engine_type", &TcDBGame::engine_type_),
                    make_column("cover_url", &TcDBGame::cover_url_)
                ),
                make_table("stream",
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
                ),
                make_table("visit_record",
                    make_column("id", &VisitRecord::id_, primary_key()),
                    make_column("the_conn_id", &VisitRecord::the_conn_id_),
                    make_column("conn_type", &VisitRecord::conn_type_),
                    make_column("begin", &VisitRecord::begin_),
                    make_column("end", &VisitRecord::end_),
                    make_column("duration", &VisitRecord::duration_),
                    make_column("visitor_device", &VisitRecord::visitor_device_),
                    make_column("target_device", &VisitRecord::target_device_)
                ),
                make_table("file_transfer_record",
                    make_column("id", &FileTransferRecord::id_, primary_key()),
                    make_column("the_file_id", &FileTransferRecord::the_file_id_),
                    make_column("begin", &FileTransferRecord::begin_),
                    make_column("end", &FileTransferRecord::end_),
                    make_column("visitor_device", &FileTransferRecord::visitor_device_),
                    make_column("target_device", &FileTransferRecord::target_device_),
                    make_column("direction", &FileTransferRecord::direction_),
                    make_column("file_detail", &FileTransferRecord::file_detail_),
                    make_column("success", &FileTransferRecord::success_)
                )
            );
            return st;
        }
        auto GetStorageTypeValue() {
            return InitAppDatabase("");
        }

        std::any GetDbStorage() {
            return db_storage_;
        }

        std::shared_ptr<VisitRecordOperator> GetVisitRecordOp();
        std::shared_ptr<FileTransferRecordOperator> GetFileTransferRecordOp();
        std::shared_ptr<StreamDBOperator> GetStreamDBOperator();
        std::shared_ptr<DBGameOperator> GetDBGameOperator();

    private:

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::any db_storage_;
        std::shared_ptr<VisitRecordOperator> visit_record_op_ = nullptr;
        std::shared_ptr<FileTransferRecordOperator> ft_record_op_ = nullptr;
        std::shared_ptr<StreamDBOperator> stream_operator_ = nullptr;
        std::shared_ptr<DBGameOperator> db_game_operator_ = nullptr;
    };

}

#endif //GAMMARAY_GR_DATABASE_H

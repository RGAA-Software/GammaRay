//
// Created by RGAA on 29/05/2025.
//

#include "gr_database.h"
#include "visit_record.h"
#include "visit_record_operator.h"
#include "file_transfer_record_operator.h"
#include "stream_db_operator.h"
#include "db_game_operator.h"
#include "tc_common_new/folder_util.h"
#include <QApplication>

namespace tc
{

    GrDatabase::GrDatabase(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
    }

    bool GrDatabase::Init() {
        std::string base_path = QString::fromStdWString(FolderUtil::GetProgramDataPath()).toStdString();
        auto db_path = base_path + "/gr_data/gr_data.db";
        auto storage = InitAppDatabase(db_path);
        storage.sync_schema();
        db_storage_ = storage;

        stream_operator_ = std::make_shared<StreamDBOperator>(shared_from_this());
        db_game_operator_ = std::make_shared<DBGameOperator>(context_, shared_from_this());
        visit_record_op_ = std::make_shared<VisitRecordOperator>(context_, shared_from_this());
        ft_record_op_ = std::make_shared<FileTransferRecordOperator>(context_, shared_from_this());
        return true;
    }

    std::shared_ptr<VisitRecordOperator> GrDatabase::GetVisitRecordOp() {
        return visit_record_op_;
    }

    std::shared_ptr<FileTransferRecordOperator> GrDatabase::GetFileTransferRecordOp() {
        return ft_record_op_;
    }

    std::shared_ptr<StreamDBOperator> GrDatabase::GetStreamDBOperator() {
        return stream_operator_;
    }

    std::shared_ptr<DBGameOperator> GrDatabase::GetDBGameOperator() {
        return db_game_operator_;
    }
}
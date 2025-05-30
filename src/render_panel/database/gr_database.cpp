//
// Created by RGAA on 29/05/2025.
//

#include "gr_database.h"
#include "visit_record.h"
#include "visit_record_operator.h"
#include <QApplication>

namespace tc
{

    GrDatabase::GrDatabase() {

    }

    bool GrDatabase::Init() {
        auto db_path = qApp->applicationDirPath() + "/gr_data/gr_data.db";
        auto storage = InitAppDatabase(db_path.toStdString());
        db_storage_ = storage;
        storage.sync_schema();
        return true;
    }

    std::shared_ptr<VisitRecordOperator> GrDatabase::GetVisitRecordOp() {
        return visit_record_op_;
    }

    std::shared_ptr<FileTransferRecordOperator> GrDatabase::GetFileTransferRecordOp() {
        return ft_record_op_;
    }

}
//
// Created by RGAA on 29/05/2025.
//

#ifndef GAMMARAY_GR_DATABASE_H
#define GAMMARAY_GR_DATABASE_H

#include <memory>

namespace tc
{

    class VisitRecordOperator;
    class FileTransferRecordOperator;

    class GrDatabase {
    public:
        GrDatabase();
        bool Init();

        std::shared_ptr<VisitRecordOperator> GetVisitRecordOp();
        std::shared_ptr<FileTransferRecordOperator> GetFileTransferRecordOp();

    private:
        std::shared_ptr<VisitRecordOperator> visit_record_op_ = nullptr;
        std::shared_ptr<FileTransferRecordOperator> ft_record_op_ = nullptr;
    };

}

#endif //GAMMARAY_GR_DATABASE_H

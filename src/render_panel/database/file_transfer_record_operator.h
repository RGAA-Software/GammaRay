//
// Created by RGAA on 29/05/2025.
//

#ifndef GAMMARAY_FILE_TRANSFER_RECORD_OPERATOR_H
#define GAMMARAY_FILE_TRANSFER_RECORD_OPERATOR_H

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace tc
{

    class GrContext;
    class GrDatabase;
    class FileTransferRecord;

    class FileTransferRecordOperator {
    public:
        FileTransferRecordOperator(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<GrDatabase>& db);

        void InsertFileTransferRecord(const std::shared_ptr<FileTransferRecord>& record);
        void UpdateVisitRecord(const std::string& the_file_id, int64_t end_timestamp);
        std::optional<std::shared_ptr<FileTransferRecord>> GetFileTransferRecordByFileId(const std::string& the_file_id);
        std::vector<std::shared_ptr<FileTransferRecord>> QueryFileTransferRecords(int page, int page_size);
        void Delete(int id);
        void DeleteAll();
        int GetTotalCounts();

    private:
        std::shared_ptr<GrDatabase> db_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
    };

}

#endif //GAMMARAY_VISIT_RECORD_OPERATOR_H

//
// Created by RGAA on 29/05/2025.
//

#include "file_transfer_record_operator.h"
#include "gr_database.h"
#include "file_transfer_record.h"

namespace tc
{

    FileTransferRecordOperator::FileTransferRecordOperator(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<GrDatabase>& db) {
        context_ = ctx;
        db_ = db;
    }

    void FileTransferRecordOperator::InsertFileTransferRecord(const std::shared_ptr<FileTransferRecord>& record) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.insert(*record);
    }

    void FileTransferRecordOperator::UpdateVisitRecord(const std::string& the_file_id, int64_t end_timestamp) {
        auto opt_record = GetFileTransferRecordByFileId(the_file_id);
        if (!opt_record.has_value()) {
            return;
        }

        const auto& record = opt_record.value();
        record->end_ = end_timestamp;
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<FileTransferRecord>(where(c(&FileTransferRecord::the_file_id_) == the_file_id));
        if (!streams.empty()) {
            storage.update(*record);
        }
    }

    std::optional<std::shared_ptr<FileTransferRecord>> FileTransferRecordOperator::GetFileTransferRecordByFileId(const std::string& the_file_id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto records = storage.get_all_pointer<FileTransferRecord>(where(c(&FileTransferRecord::the_file_id_) == the_file_id));
        if (records.empty()) {
            return std::nullopt;
        }
        auto record = std::move(records[0]);
        return record;
    }

    std::vector<std::shared_ptr<FileTransferRecord>> FileTransferRecordOperator::QueryFileTransferRecords(int page, int page_size) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto qrs = storage.get_all_pointer<FileTransferRecord>(
                                                    order_by(&FileTransferRecord::id_),
                                                    limit(page_size, offset((page-1)*page_size)));
        std::vector<std::shared_ptr<FileTransferRecord>> records;
        for (auto& r : qrs) {
            records.push_back(std::move(r));
        }
        return records;
    }

    void FileTransferRecordOperator::Delete(int id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.remove<FileTransferRecord>(id);
    }

    void FileTransferRecordOperator::DeleteAll() {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.remove_all<FileTransferRecord>();
    }

    int FileTransferRecordOperator::GetTotalCounts() {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto count = storage.count<FileTransferRecord>();
        return count;
    }

}
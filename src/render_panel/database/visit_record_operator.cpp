//
// Created by RGAA on 29/05/2025.
//

#include "visit_record_operator.h"
#include "gr_database.h"
#include "visit_record.h"

namespace tc
{

    VisitRecordOperator::VisitRecordOperator(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<GrDatabase>& db) {
        context_ = ctx;
        db_ = db;
    }

    void VisitRecordOperator::InsertVisitRecord(const std::shared_ptr<VisitRecord>& record) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.insert(*record);
    }

    void VisitRecordOperator::UpdateVisitRecord(const std::string& conn_id, int64_t end_timestamp, int64_t duration) {
        auto opt_record = GetVisitRecordConnId(conn_id);
        if (!opt_record.has_value()) {
            return;
        }

        const auto& record = opt_record.value();
        record->end_ = end_timestamp;
        record->duration_ = duration;
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<VisitRecord>(where(c(&VisitRecord::conn_id_) == conn_id));
        if (!streams.empty()) {
            storage.update(*record);
        }
    }

    std::optional<std::shared_ptr<VisitRecord>> VisitRecordOperator::GetVisitRecordConnId(const std::string& conn_id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto records = storage.get_all_pointer<VisitRecord>(where(c(&VisitRecord::conn_id_) == conn_id));
        if (records.empty()) {
            return std::nullopt;
        }
        auto record = std::move(records[0]);
        return record;
    }

    std::vector<std::shared_ptr<VisitRecord>> VisitRecordOperator::QueryVisitRecords(int page, int page_size) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto qrs = storage.get_all_pointer<VisitRecord>(
                                                    order_by(&VisitRecord::id_),
                                                    limit(page_size, offset((page-1)*page_size)));
        std::vector<std::shared_ptr<VisitRecord>> records;
        for (auto& r : qrs) {
            records.push_back(std::move(r));
        }
        return records;
    }

    void VisitRecordOperator::Delete(int id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.remove<VisitRecord>(id);
    }

    void VisitRecordOperator::DeleteAll() {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        storage.remove_all<VisitRecord>();
    }

    int VisitRecordOperator::GetTotalCounts() {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto count = storage.count<VisitRecord>();
        return count;
    }

}
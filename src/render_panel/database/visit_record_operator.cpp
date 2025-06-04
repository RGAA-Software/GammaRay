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

    void VisitRecordOperator::UpdateVisitRecord(const std::string& the_conn_id, int64_t end_timestamp, int64_t duration) {
        auto opt_record = GetVisitRecordByConnId(the_conn_id);
        if (!opt_record.has_value()) {
            return;
        }

        auto record = opt_record.value();
        record->end_ = end_timestamp;
        record->duration_ = duration;
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all<VisitRecord>(where(c(&VisitRecord::the_conn_id_) == the_conn_id));
        if (streams.size() == 1) {
            storage.update(*record);
        }
    }

    std::optional<std::shared_ptr<VisitRecord>> VisitRecordOperator::GetVisitRecordByConnId(const std::string& the_conn_id) {
        using Storage = decltype(db_->GetStorageTypeValue());
        auto storage = std::any_cast<Storage>(db_->GetDbStorage());
        auto streams = storage.get_all_pointer<VisitRecord>(where(c(&VisitRecord::the_conn_id_) == the_conn_id));
        if (streams.empty()) {
            return std::nullopt;
        }
        auto target_stream = std::move(streams[0]);
        return target_stream;
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
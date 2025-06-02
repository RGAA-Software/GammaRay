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
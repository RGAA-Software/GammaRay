//
// Created by RGAA on 29/05/2025.
//

#ifndef GAMMARAY_VISIT_RECORD_OPERATOR_H
#define GAMMARAY_VISIT_RECORD_OPERATOR_H

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace tc
{

    class GrContext;
    class GrDatabase;
    class VisitRecord;

    class VisitRecordOperator {
    public:
        VisitRecordOperator(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<GrDatabase>& db);

        void InsertVisitRecord(const std::shared_ptr<VisitRecord>& record);
        void UpdateVisitRecord(const std::string& stream_id, int64_t end_timestamp, int64_t duration);
        std::optional<std::shared_ptr<VisitRecord>> GetVisitRecordByStreamId(const std::string& stream_id);
        std::vector<std::shared_ptr<VisitRecord>> QueryVisitRecords(int page, int page_size);
        void Delete(int id);
        void DeleteAll();
        int GetTotalCounts();

    private:
        std::shared_ptr<GrDatabase> db_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
    };

}

#endif //GAMMARAY_VISIT_RECORD_OPERATOR_H

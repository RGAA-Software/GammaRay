//
// Created by RGAA on 28/05/2025.
//

#ifndef GAMMARAY_ST_SECURITY_VISITOR_H
#define GAMMARAY_ST_SECURITY_VISITOR_H

#include "tab_base.h"
#include <memory>
#include <QListWidget>

namespace tc
{

    class PageWidget;
    class GrApplication;
    class VisitRecord;
    class VisitRecordOperator;
    class StSecurityVisitorItemWidget;

    class StSecurityVisitor : public TabBase {
    public:
        StSecurityVisitor(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        void OnTranslate() override;

    private:
        QListWidgetItem* AddItem(const std::shared_ptr<VisitRecord>& record);
        void LoadPage(int page);
        void RegisterActions(int index);
        void ProcessCopy(const std::shared_ptr<VisitRecord>& record);
        void ProcessCopyAsJson(const std::shared_ptr<VisitRecord>& record);
        void ProcessDelete(const std::shared_ptr<VisitRecord>& record);

    private:
        PageWidget* page_widget_ = nullptr;
        QListWidget* list_widget_ = nullptr;
        std::shared_ptr<VisitRecordOperator> visit_op_ = nullptr;
        std::vector<std::shared_ptr<VisitRecord>> records_;
        StSecurityVisitorItemWidget* header_item_ = nullptr;
    };

}
#endif //GAMMARAY_ST_SECURITY_VISITOR_H

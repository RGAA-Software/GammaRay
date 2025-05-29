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

    class StSecurityVisitor : public TabBase {
    public:
        StSecurityVisitor(const std::shared_ptr<GrApplication>& app, QWidget *parent);

    private:
        QListWidgetItem* AddItem(const std::shared_ptr<VisitRecord>& item_info);
        void LoadPage(int page);

    private:
        PageWidget* page_widget_ = nullptr;
        QListWidget* list_widget_ = nullptr;
        std::vector<std::shared_ptr<VisitRecord>> records_;
    };

}
#endif //GAMMARAY_ST_SECURITY_VISITOR_H

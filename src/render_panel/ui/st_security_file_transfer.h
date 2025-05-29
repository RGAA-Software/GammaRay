//
// Created by RGAA on 28/05/2025.
//

#ifndef GAMMARAY_ST_SECURITY_FILE_TRANSFER_H
#define GAMMARAY_ST_SECURITY_FILE_TRANSFER_H

#include "tab_base.h"
#include <memory>
#include <vector>
#include <QListWidget>

namespace tc
{

    class PageWidget;
    class GrApplication;
    class FileTransferRecord;

    class StSecurityFileTransfer : public TabBase {
    public:
        StSecurityFileTransfer(const std::shared_ptr<GrApplication>& app, QWidget *parent);

    private:
        QListWidgetItem* AddItem(const std::shared_ptr<FileTransferRecord>& item_info);
        void LoadPage(int page);

    private:
        PageWidget* page_widget_ = nullptr;
        QListWidget* list_widget_ = nullptr;
        std::vector<std::shared_ptr<FileTransferRecord>> records_;
    };

}
#endif //GAMMARAY_ST_SECURITY_VISITOR_H

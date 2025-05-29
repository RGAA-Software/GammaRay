//
// Created by RGAA on 30/04/2025.
//

#ifndef GAMMARAY_ST_SECURITY_VISITOR_ITEM_WIDGET_H
#define GAMMARAY_ST_SECURITY_VISITOR_ITEM_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QBrush>
#include <QPen>
#include <QLabel>

namespace tc
{

    class GrContext;
    class GrApplication;
    class FileTransferRecord;

    class StSecurityFileTransferItemWidget : public QWidget {
    public:
        StSecurityFileTransferItemWidget(const std::shared_ptr<GrApplication>& app,
                           const std::shared_ptr<FileTransferRecord>& item_info,
                           QWidget* parent);
        void paintEvent(QPaintEvent *event) override;
        void UpdateStatus();
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;

    private:
        void UpdatePluginStatus(bool enabled);
        void SwitchPluginStatusInner(bool enabled);

    private:
        std::shared_ptr<FileTransferRecord> item_info_;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        QLabel* lbl_enabled_ = nullptr;
        bool enter_ = false;
    };

}

#endif //GAMMARAY_ST_PLUGIN_ITEM_WIDGET_H

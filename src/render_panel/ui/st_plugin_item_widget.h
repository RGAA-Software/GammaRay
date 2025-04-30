//
// Created by RGAA on 30/04/2025.
//

#ifndef GAMMARAY_ST_PLUGIN_ITEM_WIDGET_H
#define GAMMARAY_ST_PLUGIN_ITEM_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QBrush>
#include <QPen>
#include <QLabel>

namespace tc
{

    class GrApplication;
    class PluginItemInfo;

    class StPluginItemWidget : public QWidget {
    public:
        StPluginItemWidget(const std::shared_ptr<GrApplication>& app,
                           const std::shared_ptr<PluginItemInfo>& item_info,
                           QWidget* parent);
        void paintEvent(QPaintEvent *event) override;

        void UpdateStatus(const std::shared_ptr<PluginItemInfo>& item_info);

    private:
        void SetEnabled(bool enabled);

    private:
        std::shared_ptr<PluginItemInfo> item_info_;
        QLabel* lbl_enabled_ = nullptr;
    };

}

#endif //GAMMARAY_ST_PLUGIN_ITEM_WIDGET_H

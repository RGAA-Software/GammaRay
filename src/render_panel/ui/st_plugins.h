//
// Created by RGAA on 29/04/2025.
//

#ifndef GAMMARAY_ST_PLUGINS_H
#define GAMMARAY_ST_PLUGINS_H

#include <QLabel>
#include "tab_base.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QString>
#include <QPaintEvent>

namespace tcrp
{
    class RpPluginInfo;
}

namespace tc
{

    class PluginItemInfo {
    public:
        std::string id_;
        std::shared_ptr<tcrp::RpPluginInfo> info_ = nullptr;
    };

    class StPlugins : public TabBase {
    public:
        explicit StPlugins(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StPlugins() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        void RefreshListWidget();
        void UpdateItemStatus();
        QListWidgetItem* AddItem(const std::shared_ptr<PluginItemInfo>& item, int index);

    private:
        QListWidget* stream_list_ = nullptr;
        std::vector<std::shared_ptr<PluginItemInfo>> items_info_;
    };

}



#endif //GAMMARAY_ST_PLUGINS_H

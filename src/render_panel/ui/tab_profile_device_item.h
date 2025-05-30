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
#include "tc_qt_widget/click_listener.h"

namespace tc
{

    class GrContext;
    class GrApplication;
    class AccountDevice;

    class TabProfileDeviceItemWidget : public QWidget {
    public:
        TabProfileDeviceItemWidget(const std::shared_ptr<GrApplication>& app,
                           const std::shared_ptr<AccountDevice>& item_info,
                           QWidget* parent);
        void paintEvent(QPaintEvent *event) override;
        void UpdateStatus();
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void SetOnItemClickListener(OnItemValueClickListener<AccountDevice>&& listener);

    private:
        std::shared_ptr<AccountDevice> item_info_;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        QLabel* lbl_enabled_ = nullptr;
        bool enter_ = false;
        bool pressed_ = false;
        OnItemValueClickListener<AccountDevice> click_listener_;
    };

}

#endif //GAMMARAY_ST_PLUGIN_ITEM_WIDGET_H

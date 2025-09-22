//
// Created by RGAA on 21/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_INFO_WIDGET_H
#define GAMMARAYPREMIUM_HW_INFO_WIDGET_H

#include <deque>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

namespace tc
{

    class SysInfo;
    class TcLabel;

    class HWInfoWidget : public QWidget {
    public:
        explicit HWInfoWidget(QWidget* parent = nullptr);
        void OnSysInfoCallback(const std::shared_ptr<SysInfo>& si);
        void paintEvent(QPaintEvent *event) override;

    private:
        void RefreshInternal();

    private:
        std::deque<std::shared_ptr<SysInfo>> sys_info_hist_;
        TcLabel* lbl_pc_name_ = nullptr;
        TcLabel* lbl_os_version_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_HW_INFO_WIDGET_H

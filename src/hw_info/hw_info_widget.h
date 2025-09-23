//
// Created by RGAA on 21/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_INFO_WIDGET_H
#define GAMMARAYPREMIUM_HW_INFO_WIDGET_H

#include <deque>
#include <vector>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

namespace tc
{

    class SysInfo;
    class TcLabel;

    class EditableLine {
    public:
        TcLabel* title_ = nullptr;
        TcLabel* value_ = nullptr;
    };

    class HWInfoWidget : public QWidget {
    public:
        explicit HWInfoWidget(QWidget* parent = nullptr);
        void OnSysInfoCallback(const std::shared_ptr<SysInfo>& si);
        void paintEvent(QPaintEvent *event) override;

    private:
        void RefreshInternal();
        void GenDiskList(const std::shared_ptr<SysInfo>& sys_info);
        void GenNetworkList(const std::shared_ptr<SysInfo>& sys_info);

    private:
        std::deque<std::shared_ptr<SysInfo>> sys_info_hist_;
        TcLabel* lbl_pc_name_ = nullptr;
        TcLabel* lbl_os_version_ = nullptr;
        TcLabel* lbl_cpu_ = nullptr;
        TcLabel* lbl_memory_ = nullptr;
        TcLabel* lbl_uptime_ = nullptr;
        QWidget* disk_widget_ = nullptr;
        std::vector<EditableLine> lbl_disks_;
        QWidget* net_widget_ = nullptr;
        std::vector<EditableLine> lbl_networks_;
    };

}

#endif //GAMMARAYPREMIUM_HW_INFO_WIDGET_H

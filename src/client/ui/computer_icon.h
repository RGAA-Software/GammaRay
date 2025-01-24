//
// Created by RGAA on 17/08/2024.
//

#ifndef GAMMARAYPC_COMPUTER_ICON_H
#define GAMMARAYPC_COMPUTER_ICON_H

#include "base_widget.h"

namespace tc
{

    class ComputerIcon : public BaseWidget {
    public:
        explicit ComputerIcon(const std::shared_ptr<ClientContext>& ctx, int idx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void UpdateSelectedState(bool selected);
        void SetMonitorName(const std::string& name);
        int GetMonitorIndex() const;
        std::string GetMonitorName();
        bool IsSelected();

    private:
        QPixmap pixmap_;
        int monitor_index_ = 0;
        std::string monitor_name_{};
        bool enter_ = false;
        bool pressed_ = false;
        int icon_size_ = 22;
        bool selected_ = false;
    };

}

#endif //GAMMARAYPC_COMPUTER_ICON_H

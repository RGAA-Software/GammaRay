//
// Created by RGAA on 17/07/2024.
//

#ifndef GAMMARAYPC_NOTIFICATION_ITEM_H
#define GAMMARAYPC_NOTIFICATION_ITEM_H

#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QProgressBar>
#include <string>
#include <QPixmap>
#include <QListWidgetItem>

namespace tc
{

    class ClientContext;

    enum class NotificationState {
        kNotificationIdle,
        kNotificationSuccess,
        kNotificationFailed,
    };

    class NotificationItem : public QWidget {
    public:
        explicit NotificationItem(const std::shared_ptr<ClientContext>& ctx, const std::string& nid, const std::string& icon_path = "", QWidget* parent = nullptr);

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

        void UpdateTitle(const std::string& title);
        void UpdateProgress(int progress);
        void UpdateProgressUpdateTime(uint64_t time);
        void UpdateProgressDataSize(uint64_t size);
        void SetState(const NotificationState& state);
        bool IsProgressOver();
        void SetBelongToItem(QListWidgetItem* item);
        QListWidgetItem* BelongToItem();

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        bool enter_ = false;
        bool pressed_ = false;
        QLabel* icon_ = nullptr;
        QLabel* title_ = nullptr;
        QLabel* sub_title_ = nullptr;
        QLabel* progress_info_ = nullptr;
        QLabel* transfer_info_ = nullptr;
        QProgressBar* progress_ = nullptr;
        std::string nid_;
        QPixmap icon_pixmap_;
        uint64_t progress_update_time_ = 0;
        uint64_t last_progress_update_time_ = 0;
        uint64_t progress_data_size_ = 0;
        uint64_t last_progress_data_size_ = 0;
        bool progress_over_ = false;
        std::string last_speed_str_;
        NotificationState state_ = NotificationState::kNotificationIdle;
        QListWidgetItem* belong_to_ = nullptr;
    };

}

#endif //GAMMARAYPC_NOTIFICATION_ITEM_H

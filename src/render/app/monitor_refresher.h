//
// Created by RGAA on 20/02/2025.
//

#ifndef GAMMARAY_MONITOR_REFRESHER_H
#define GAMMARAY_MONITOR_REFRESHER_H

#include <QWidget>
#include <memory>

namespace tc
{

    class RdContext;
    class MessageListener;

    // Widget
    class MonitorRefreshWidget : public QWidget {
    public:
        explicit MonitorRefreshWidget(const std::shared_ptr<RdContext>& ctx, QWidget* parent);
        void paintEvent(QPaintEvent *event) override;

    private:
        std::shared_ptr<RdContext> context_ = nullptr;
    };

    // Refresher
    class MonitorRefresher  {
    public:
        explicit MonitorRefresher(const std::shared_ptr<RdContext>& ctx, QWidget* parent);

    private:
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //GAMMARAY_MONITOR_REFRESHER_H

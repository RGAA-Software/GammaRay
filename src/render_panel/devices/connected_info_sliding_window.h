#pragma once
#include <QWidget>
#include <qpainter.h>
#include <qevent.h>

namespace tcrp
{
    class RpConnectedClientInfo;
}

namespace tc {

    class NoMarginVLayout;
    class NoMarginHLayout;
    class GrContext;
    class ConnectedInfoTag;
    class ConnectedInfoPanel;

    // 被客户端连接上来后，显示连接者的一些信息
    class ConnectedInfoSlidingWindow : public QWidget {
    public:
        ConnectedInfoSlidingWindow(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent* event) override;
        bool eventFilter(QObject* obj, QEvent* event) override;
        void UpdateInfo(const std::shared_ptr<tcrp::RpConnectedClientInfo>& info);
    private:
        void InitView();
    private:
        NoMarginHLayout* main_hbox_layout_ = nullptr;
        ConnectedInfoTag* tag_ = nullptr;
        ConnectedInfoPanel* panel_ = nullptr;
        std::shared_ptr<tcrp::RpConnectedClientInfo> info_ = nullptr;
        std::shared_ptr<GrContext> ctx_ = nullptr;
    };
}
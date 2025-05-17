//
// Created by RGAA on 2023/8/19.
//

#ifndef SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H
#define SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPen>
#include <QBrush>
#include <QPaintEvent>
#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <memory>

namespace tc
{

    class StreamItem;
    class TcPushButton;

    using OnConnectListener = std::function<void()>;
    using OnMenuListener = std::function<void()>;

    class StreamItemWidget : public QWidget {
    public:

        explicit StreamItemWidget(const std::shared_ptr<StreamItem>& item, int bg_color, QWidget* parent = nullptr);
        ~StreamItemWidget() override;

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

        void SetOnConnectListener(OnConnectListener&& listener);
        void SetOnMenuListener(OnMenuListener&& listener);

    private:
        std::shared_ptr<StreamItem> item_;
        int bg_color_ = 0;
        QPixmap icon_;
        QPixmap bg_pixmap_;
        bool enter_ = false;
        QBitmap mask_;
        int radius_ = 10;
        TcPushButton* btn_conn_ = nullptr;
        QWidget* btn_option_ = nullptr;
        OnConnectListener conn_listener_;
        OnMenuListener menu_listener_;
    };

}

#endif //SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H

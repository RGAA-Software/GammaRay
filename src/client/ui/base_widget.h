//
// Created by RGAA on 3/07/2024.
//

#ifndef GAMMARAYPC_BASE_WIDGET_H
#define GAMMARAYPC_BASE_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <memory>
#include <functional>
#include "widget_helper.h"

namespace tc
{
    class ClientContext;
    class MessageListener;

    using OnClickListener = std::function<void(QWidget*)>;

    class BaseWidget : public QWidget {
    public:
        explicit BaseWidget(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void SetOnClickListener(OnClickListener&& l);
        void CreateMsgListener();

        virtual void Hide();
        virtual void Show();

    protected:
        std::shared_ptr<ClientContext> context_ = nullptr;
        OnClickListener click_listener_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };
}

#endif //GAMMARAYPC_BASE_WIDGET_H

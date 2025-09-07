#pragma once
#include <qevent.h>
#include <qpainter.h>
#include <qwidget.h>
#include <memory>

namespace tc
{

    class ClientContext;
    class MessageListener;

    class MediaRecordSignLab : public QWidget {
    public:
        explicit MediaRecordSignLab(const std::shared_ptr<ClientContext>& context, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent* event) override;

    private:
        int toggle_ = 0;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<MessageListener> listener_ = nullptr;
    };
}
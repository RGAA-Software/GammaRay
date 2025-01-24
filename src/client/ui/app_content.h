//
// Created by RGAA on 2023/8/16.
//

#ifndef SAILFISH_SERVER_APPCONTENT_H
#define SAILFISH_SERVER_APPCONTENT_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include "round_rect_widget.h"

namespace tc
{

    class ClientContext;
    class Settings;

    class AppContent : public RoundRectWidget {
    public:

        explicit AppContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        ~AppContent();

        virtual void OnContentShow();
        virtual void OnContentHide();

        void paintEvent(QPaintEvent *event) override;

    protected:

        std::shared_ptr<ClientContext> context_ = nullptr;

        Settings* settings_ = nullptr;

    };

}

#endif //SAILFISH_SERVER_APPCONTENT_H

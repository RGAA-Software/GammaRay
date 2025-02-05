//
// Created by RGAA on 2023/8/16.
//

#include "app_content.h"

#include "client/ct_settings.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

namespace tc
{

    AppContent::AppContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : RoundRectWidget(0xffffff, 0, parent) {
        context_ = ctx;
        settings_ = Settings::Instance();
    }

    AppContent::~AppContent() = default;

    void AppContent::OnContentShow() {
    }

    void AppContent::OnContentHide() {
    }

}
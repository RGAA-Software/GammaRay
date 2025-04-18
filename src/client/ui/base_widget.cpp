//
// Created by RGAA on 3/07/2024.
//

#include "base_widget.h"
#include "client/ct_client_context.h"
#include "ct_app_message.h"
namespace tc
{

    BaseWidget::BaseWidget(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : QWidget(parent), context_(ctx) {
        CreateMsgListener();
    }

    void BaseWidget::CreateMsgListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<FloatControllerPanelUpdateMessage>([=, this](const FloatControllerPanelUpdateMessage& msg) {
            UpdateStatus(msg);
        });
    }

    void BaseWidget::SetOnClickListener(OnClickListener&& l) {
        click_listener_ = l;
    }

    void BaseWidget::Hide() {
        this->hide();
    }

    void BaseWidget::Show() {
        this->show();
    }

}
//
// Created by RGAA on 3/07/2024.
//

#include "base_widget.h"
#include "client_context.h"

namespace tc
{

    BaseWidget::BaseWidget(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : QWidget(parent), context_(ctx) {

    }

    void BaseWidget::CreateMsgListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
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
//
// Created by RGAA on 3/07/2024.
//

#include "float_controller.h"
#include "app_color_theme.h"
#include "tc_common_new/log.h"
#include "client_context.h"
#include <QTimer>

const static std::string kPosX = "float_button_pos_x";
const static std::string kPosY = "float_button_pos_y";

namespace tc
{
    FloatController::FloatController(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent){
        auto image = new QImage(":resources/image/tc_icon.png");
        pixmap_ = QPixmap::fromImage(*image);
        pixmap_ = pixmap_.scaled(30, 30);
        setMouseTracking(true);
        this->setStyleSheet("background:#00000000;");
        QTimer::singleShot(200, [=]() {
            QRect parent_rect = parentWidget()->geometry();
            float xpos = parent_rect.width() * std::atof(context_->GetValueByKey(kPosX).c_str());
            float ypos = parent_rect.height() * std::atof(context_->GetValueByKey(kPosY).c_str());
            move(xpos, ypos);
        });

    }

    void FloatController::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setPen(Qt::NoPen);
        if (pressed_) {
            painter.setBrush(QBrush(QColor(0xdfdfdf)));
        } else if (enter_) {
            painter.setBrush(QBrush(QColor(0xf5f5f5)));
        } else {
            painter.setBrush(QBrush(QColor(0xffffff)));
        }
        painter.drawRoundedRect(this->rect(), this->width()/2, this->height()/2);

        painter.drawPixmap((this->width()-pixmap_.width())/2, (this->height()-pixmap_.height())/2, pixmap_);
        BaseWidget::paintEvent(event);
    }

    void FloatController::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        drag_position_ = event->globalPos() - frameGeometry().topLeft();
        event->accept();
        repaint();
    }

    void FloatController::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        event->accept();
        repaint();

        QRect parent_rect = parentWidget()->geometry();
        float x_pos_ratio = this->pos().x() * 1.0f / parent_rect.width();
        float y_pos_ratio = this->pos().y() * 1.0f / parent_rect.height();
        context_->SaveKeyValue(kPosX, std::to_string(x_pos_ratio));
        context_->SaveKeyValue(kPosY, std::to_string(y_pos_ratio));

        if (click_listener_) {
            click_listener_();
        }

        has_moved_ = false;
    }

    void FloatController::mouseMoveEvent(QMouseEvent *event) {
        if (pressed_) {
            has_moved_ = true;
            QPoint new_top_left = event->globalPos() - drag_position_;
            QRect parent_rect = parentWidget()->geometry();
            new_top_left.setX(new_top_left.x() < 0 ? 0 : new_top_left.x() > (parent_rect.width()-this->width()) ? parent_rect.width()-this->width() : new_top_left.x());
            new_top_left.setY(new_top_left.y() < 0 ? 0 : new_top_left.y() > (parent_rect.height()-this->height()) ? parent_rect.height()-this->height() : new_top_left.y());
            move(new_top_left);
        }
        if (move_listener_ && pressed_) {
            move_listener_();
        }
    }

    void FloatController::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void FloatController::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

    bool FloatController::HasMoved() const {
        return has_moved_;
    }
}
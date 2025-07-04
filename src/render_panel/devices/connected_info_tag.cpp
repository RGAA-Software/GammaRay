#include "connected_info_tag.h"
#include <QPainterPath>
#include <qapplication.h>

namespace tc {

    ConnectedInfoTag::ConnectedInfoTag(QWidget* parent) {
	    setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(28, 32);
    }

    void ConnectedInfoTag::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), Qt::transparent);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0x29, 0x79, 0xff));

        QPainterPath path;

        int width = this->width();
        int height = this->height();
        const int radius = height / 2;

        // 添加0.5像素偏移以获得完美抗锯齿
        const qreal offset = 0.5;

        // 起点：左侧半圆顶部端点
        path.moveTo(offset, offset);

        // 绘制左侧半圆 (考虑抗锯齿偏移)
        path.arcTo(QRectF(offset, offset,2 * radius - 1, height - 1), 90, 180);

        // 绘制右侧矩形部分
        path.lineTo(width - offset, height - offset);  // 右下角
        path.lineTo(width - offset, offset);      // 右上角
        path.lineTo(radius + offset, offset); // 回到半圆顶部端点

        path.closeSubpath();
        painter.drawPath(path);

        QPen pen(Qt::white, 2);  
        painter.setPen(pen);
        QPolygonF shape;
        if (expanded_) {
            // 计算大于号的坐标
            QPointF point1(width * 0.5, height * 0.3);  // 左上点
            QPointF point2(width * 0.7, height * 0.5);  // 中心点
            QPointF point3(width * 0.5, height * 0.7);  // 左下点
            shape << point1 << point2 << point3;
        }
        else {
            // 计算小于号的坐标
            QPointF point1(width * 0.7, height * 0.3);  // 左上点
            QPointF point2(width * 0.5, height * 0.5);  // 中心点
            QPointF point3(width * 0.7, height * 0.7);  // 左下点
            shape << point1 << point2 << point3;
        }
        painter.drawPolyline(shape);

        if (parentWidget()) {
            auto screen_rect = QApplication::primaryScreen()->availableGeometry();
            int screen_height = screen_rect.height();
            auto curent_pos = parentWidget()->pos();
            if (curent_pos.y() + 70 + 130 > screen_height) {
                curent_pos.setY(screen_height - 70 - 130);
            }
            else if (curent_pos.y() < 6) {
                curent_pos.setY(6);
            }
            parentWidget()->move(curent_pos);
        }
    }

    bool ConnectedInfoTag::GetExpanded() const {
        return expanded_;
    }

    void ConnectedInfoTag::SetExpanded(bool expanded) {
        expanded_ = expanded;
    }

    void ConnectedInfoTag::mousePressEvent(QMouseEvent* event) {
        if (event->button() == Qt::LeftButton) {
            m_dragStartPos = event->globalPos();
            m_dragging = true;
            generate_movement_ = false;
        }
        QWidget::mousePressEvent(event);
    }

    void ConnectedInfoTag::mouseMoveEvent(QMouseEvent* event) {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            QPoint delta = event->globalPos() - m_dragStartPos;
            delta.setX(0);
            auto primary_screen =  QApplication::primaryScreen();
            if (parentWidget() && primary_screen) {
                auto screen_rect = primary_screen->availableGeometry();
                int screen_height = screen_rect.height();
                if (parentWidget()->pos().y() + 70 + 130 > screen_height && delta.y() > 0) {
                    delta.setY(0);
                }
                if (parentWidget()->pos().y() < 6 && delta.y() < 0) {
                    delta.setY(0);
                }
                parentWidget()->move(parentWidget()->pos() + delta);
                generate_movement_ = true;
            }
            m_dragStartPos = event->globalPos();
        }
        QWidget::mouseMoveEvent(event);
    }

    void ConnectedInfoTag::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::LeftButton) {
            m_dragging = false;
        }
        QWidget::mouseReleaseEvent(event);
    }
}
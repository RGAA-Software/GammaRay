#include "connected_info_tag.h"
#include <QPainterPath>

namespace tc {

ConnectedInfoTag::ConnectedInfoTag(QWidget* parent) {
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
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
    if (expanded) {
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
}

void ConnectedInfoTag::mousePressEvent(QMouseEvent* event) {

    QWidget::mousePressEvent(event);
}

}
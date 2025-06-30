#include "connected_info_tag.h"
#include <QPainterPath>

namespace tc {

ConnectedInfoTag::ConnectedInfoTag(QWidget* parent) {
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(48, 48);
}

void ConnectedInfoTag::paintEvent(QPaintEvent* event)
{
    //Q_UNUSED(event);
    //QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿

    //// 1. 清除背景（透明）
    //painter.fillRect(rect(), Qt::transparent);

    // 2. 绘制半圆（上半圆示例）
    //QRect circleRect = rect();
    //painter.setPen(Qt::NoPen);
    //painter.setBrush(QColor(52, 152, 219));  // 蓝色填充

    //// 角度单位：1/16度（90° * 16 = 1440, 180° * 16 = 2880）
    //painter.drawPie(circleRect, 90 * 16, 180 * 16);  // 从90°开始画180°（上半圆）

    /* 其他方向半圆：
      下半圆：painter.drawPie(rect(), 270 * 16, 180 * 16);
      左半圆：painter.drawPie(rect(), 180 * 16, 180 * 16);
      右半圆：painter.drawPie(rect(), 0 * 16, 180 * 16);
    */

    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    // 设置形状颜色
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(52, 152, 219));

    QPainterPath path;/*
    CreateLeftPath(path);*/

    const int w = width();
    const int h = height();
    const int radius = h / 2;

    // 添加0.5像素偏移以获得完美抗锯齿
    const qreal offset = 0.5;

    // 起点：左侧半圆顶部端点
    path.moveTo(offset, offset);

    // 绘制左侧半圆 (考虑抗锯齿偏移)
    path.arcTo(QRectF(offset, offset,
        2 * radius - 1, h - 1),
        90, 180);

    // 绘制右侧矩形部分
    path.lineTo(w - offset, h - offset);  // 右下角
    path.lineTo(w - offset, offset);      // 右上角
    path.lineTo(radius + offset, offset); // 回到半圆顶部端点

    path.closeSubpath();


    painter.drawPath(path);
}



void ConnectedInfoTag::CreateLeftPath(QPainterPath& path) {
    const int w = width();
    const int h = height();
    const int radius = qMin(w, h) / 2;

    // 左半圆部分
    path.moveTo(0, h / 2);
    path.arcTo(QRectF(0, h / 2 - radius, 2 * radius, 2 * radius), 90, 180);

    // 延伸的矩形部分
    path.lineTo(w, h);
    path.lineTo(w, 0);
    path.closeSubpath();
}
}
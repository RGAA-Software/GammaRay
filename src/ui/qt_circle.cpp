﻿#include "qt_circle.h"

#include <QPainter>
#include <QPen>
#include <QColor>
#include <QBrush>
#include <QTransform>
#include <QDebug>
#include <QLinearGradient>
#include <QGradient>
#include <QEvent>
#include <cmath>
#include <sstream>

namespace tc
{

    QtCircle::QtCircle(QWidget *parent) : EffectWidget(parent) {

    }

    void QtCircle::paintEvent(QPaintEvent *event) {
        int circle_bonding_box_size = 230;
        int circle_bonding_box_half = circle_bonding_box_size / 2;
        int circle_vert_center = height() / 2;

        int left_circle_x = width() / 2 - circle_bonding_box_size / 2;
        int right_circle_x = width() / 2 + circle_bonding_box_size / 2;

        std::lock_guard<std::mutex> guard(data_mtx_);

        if (left_new_bars_.empty()) {
            return;
        }
        if (left_bars.size() != left_new_bars_.size()) {
            left_bars.resize(left_new_bars_.size());
        }
        if (right_bars.size() != right_new_bars_.size()) {
            right_bars.resize(right_new_bars_.size());
        }

        FallDownBars(left_bars, left_new_bars_);
        FallDownBars(right_bars, right_new_bars_);

        auto filter_left_data = left_bars;
        auto filter_right_data = right_bars;

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing);
        painter.setBrush(QBrush(0xffffff));

        QLinearGradient bar_gradient(0, 0, 60, 0);
        bar_gradient.setColorAt(0.0, QColor(0xff, 0x66, 0x66));
        bar_gradient.setColorAt(1.0, QColor(0xcc, 0xcc, 0xff));

        QLinearGradient circle_bar_gradient(0, 0, 60, 0);
        circle_bar_gradient.setColorAt(0.0, QColor(0x00, 0xd3, 0x66));
        circle_bar_gradient.setColorAt(1.0, QColor(0xfc, 0xcc, 0x12));

        int circle_bar_size = 120;

        int first_circle_bar_size = circle_bar_size * 2 / 3;

        auto radius = 30;
        auto bar_scale = 0.3f;
        auto basic_length = 3;
        float first_item_angel = 360.0f / (first_circle_bar_size*1.0f);

        QFont font;
        font.setPointSize(17);
        font.setBold(true);
        painter.setPen(QPen(QColor(0x00, 0x00, 0x33)));
        painter.setFont(font);
        painter.drawText(left_circle_x - circle_bonding_box_half,
                         circle_vert_center - circle_bonding_box_half,
                         circle_bonding_box_size,
                         circle_bonding_box_size,
                         Qt::AlignCenter, "L");
        painter.drawText(right_circle_x - circle_bonding_box_half,
                         circle_vert_center - circle_bonding_box_half,
                         circle_bonding_box_size,
                         circle_bonding_box_size,
                         Qt::AlignCenter, "R");

        // Left
        for (int i = 0; i < first_circle_bar_size; i++) {
            auto angel = -(i + 1.f) * (first_item_angel) + rotate_;
            painter.save();
            painter.translate(left_circle_x, circle_vert_center);
            painter.rotate(angel);
            painter.fillRect(radius, 0, filter_left_data[i]*bar_scale+basic_length, 3, QBrush(circle_bar_gradient));
            painter.translate(-left_circle_x, -circle_vert_center);
            painter.restore();
        }

        // Right
        for (int i = 0; i < first_circle_bar_size; i++) {
            auto angel = (i + 1.0f) * (first_item_angel) + rotate_ + 180;
            painter.save();
            painter.translate(right_circle_x, circle_vert_center);
            painter.rotate(angel);
            painter.fillRect(radius, 0, filter_right_data[i]*bar_scale+basic_length, 3, QBrush(bar_gradient));
            painter.translate(-right_circle_x, -circle_vert_center);
            painter.restore();
        }
        color_count_++;
    }


}

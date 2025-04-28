#pragma execution_character_set("utf-8")

#include "qt_vertical.h"

#include <QPainter>

namespace tc
{

QtVertical::QtVertical(QWidget* parent) : EffectWidget(parent) {

}

void QtVertical::paintEvent(QPaintEvent* event) {
    if (left_new_bars_.empty()) {
        return;
    }
    if (left_bars.size() != left_new_bars_.size()) {
        left_bars.resize(left_new_bars_.size());
    }

    FallDownBars(left_bars, left_new_bars_);

    auto filter_left_data = left_bars;

    QPainter painter(this);
    QLinearGradient bar_gradient(0, 0, this->width(), 0);
    bar_gradient.setColorAt(0.0, QColor(0xff, 0x66, 0x66));
    bar_gradient.setColorAt(0.5, QColor(0xcc, 0xcc, 0xff));
    bar_gradient.setColorAt(1.0, QColor(0x22, 0x22, 0xff));
    painter.setBrush(QBrush(bar_gradient));
    painter.setPen(Qt::NoPen);

    auto height = this->height();

    int item_width = 3;
    int gap = 2;
    int x_offset = 0;

    int bar_size = std::min(70, (int)filter_left_data.size());
    int half_bar_size = bar_size / 2;
    for (int i = bar_size; i > half_bar_size; i--) {
        filter_left_data[i] = filter_left_data[bar_size - i];
    }

    for (int i = 0; i < bar_size; i++) {
        auto bar_value = 0.35 * filter_left_data[i];
        painter.drawRect(x_offset + (item_width + gap) * i, height/2 - bar_value, item_width, bar_value);
    }
}

}

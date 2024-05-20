#pragma execution_character_set("utf-8")

#include "qt_vertical.h"

#include <QPainter>

namespace tc
{

QtVertical::QtVertical(QWidget* parent) : EffectWidget(parent) {
    pixmap.load("./bg1.jpg");
}

void QtVertical::paintEvent(QPaintEvent* event) {
    std::lock_guard<std::mutex> guard(data_mtx_);

    if (left_new_bars_.empty()) {
        return;
    }
    if (left_bars.size() != left_new_bars_.size()) {
        left_bars.resize(left_new_bars_.size());
    }

    FallDownBars(left_bars, left_new_bars_);

    auto filter_left_data = left_bars;

    QPainter painter(this);
    QLinearGradient bar_gradient(0, 0, 512, 0);
    bar_gradient.setColorAt(0.0, QColor(0xff, 0x66, 0x66));
    bar_gradient.setColorAt(1.0, QColor(0xcc, 0xcc, 0xff));
    painter.setBrush(QBrush(bar_gradient));
    painter.setPen(Qt::NoPen);

    if (!pixmap.isNull()) {
        painter.drawPixmap(0, 0, pixmap);
    }

    auto height = this->height();

    int item_width = 2;
    int gap = 1;
    int x_offset = 5;

    for (int i = 0; i < filter_left_data.size(); i++) {
        painter.drawRect(x_offset + (item_width + gap) * i, height/2 - filter_left_data[i], item_width, filter_left_data[i]);
    }

    painter.setPen(QPen(0xff00ff));
    QString msg = QString::fromStdWString(L"如果屏幕太小可能显示不全，共480个，傅里叶变换后，左右对称。在vector中取自己想要的部分使用。");
    painter.drawText(0, 50, msg);
}

}

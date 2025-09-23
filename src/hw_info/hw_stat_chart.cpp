//
// Created by RGAA on 23/09/2025.
//

#include "hw_stat_chart.h"
#include "tc_common_new/random.h"
#include <QPainterPath>

namespace tc
{

    HWStatChart::HWStatChart(QWidget* parent) : QWidget(parent) {

    }

    HWStatChart* HWStatChart::SetRow(int row) {
        row_ = row;
        return this;
    }

    HWStatChart* HWStatChart::SetColumn(int column) {
        column_ = column;
        return this;
    }

    HWStatChart* HWStatChart::SetMaxValues(int count) {
        max_values_ = count;
        return this;
    }

    HWStatChart* HWStatChart::SetTitle(const QString& title) {
        title_ = title;
        return this;
    }

    HWStatChart* HWStatChart::SetYAxisDesc(const QString& desc) {
        y_axis_desc_ = desc;
        return this;
    }

    HWStatChart* HWStatChart::SetChartType(const HWStatChartType& type) {
        type_ = type;
        return this;
    }

    void HWStatChart::UpdateValues(const std::vector<float>& v) {
        values_.clear();
        values_.insert(values_.begin(), v.begin(), v.end());
        QMetaObject::invokeMethod(this, [this]() {
            update();
        });
    }

    void HWStatChart::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        {
            QPen pen(0xefefef);
            painter.setPen(pen);
            painter.setBrush(QBrush(0xfafafa));
            auto radius = 3.0f;
            painter.drawRoundedRect(QRectF(1, 1, this->width()-2, this->height()-2), radius, radius);
        }
        float row_step = this->height() * 1.0f / row_;
        float column_step = this->width() * 1.0f / column_;
        {
            QPen pen(0xf1f1f1);
            painter.setPen(pen);
        }
        for (int i = 1; i < row_; i++) {
            painter.drawLine(0, i * row_step, this->width(), i * row_step);
        }
        for (int i = 1; i < column_; i++) {
            painter.drawLine(i*column_step, 0, i*column_step, this->height());
        }


        auto x_step = this->width() * 1.0f / max_values_;
        QPainterPath path;
        path.moveTo(this->width(), this->height());
        int x_index = 0;
        float last_x = 0;
        for (int i = values_.size() - 1; i >= 0; i--) {
            auto v = values_[i];
            auto y = this->height() - v * this->height();
            auto x = this->width() - x_index * x_step;
            last_x = x;
            path.lineTo(x, y);
            x_index++;
        }
        path.lineTo(last_x, this->height());

        if (type_ == HWStatChartType::kRegion) {
            path.lineTo(this->width(), this->height());
            QLinearGradient linearGrad(0, 0, 0, this->height());
            linearGrad.setColorAt(0, QColor(0x2979ff));
            linearGrad.setColorAt(1, QColor(0x99d9ff));
            painter.fillPath(path, linearGrad);
        }
        else {
            QPen pen(0x2979ff);
            painter.setPen(pen);
            painter.drawPath(path);
        }

        //
        {
            QPen pen(0x555555);
            painter.setPen(pen);
            auto font = painter.font();
            font.setPixelSize(12);
            painter.setFont(font);
            painter.drawText(QRectF(0, 0, this->width(), 30), Qt::AlignCenter, title_);
        }

        {
            QPen pen(0x777777);
            painter.setPen(pen);
            auto font = painter.font();
            font.setPixelSize(11);
            painter.setFont(font);
            painter.drawText(QRectF(0, 0, 55, 30), Qt::AlignCenter, y_axis_desc_);
        }
    }

}
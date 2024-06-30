#include "custom_tab_btn.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QRadialGradient>

#include "app_skin.h"

namespace tc
{
    CustomTabBtn::CustomTabBtn(QWidget *parent) : QPushButton(parent) {
        //active_bg.load(":/images/resources/btn_bg.png");

        inactive_color = AppSkin::kTabBtnInActiveColor;
        hover_color = AppSkin::kTabBtnHoverColor;
    }

    CustomTabBtn::~CustomTabBtn() {

    }

    void CustomTabBtn::SetSelectedFontColor(const std::string &color) {
        auto style = std::format(R"(
            QPushButton {{
                background:none;
                border: none;
                color: {};
            }}

            QPushButton:hover {{

            }}
            QPushButton:pressed{{

            }}
        )", color);
        this->setStyleSheet(style.c_str());
    }

    void CustomTabBtn::mousePressEvent(QMouseEvent *event) {
        QPushButton::mousePressEvent(event);
    }

    void CustomTabBtn::mouseReleaseEvent(QMouseEvent *event) {
        QPushButton::mouseReleaseEvent(event);
    }

    void CustomTabBtn::SetText(const QString &text) {
        this->text = text;
        repaint();
    }

    void CustomTabBtn::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        auto font = painter.font();
        //font.setBold(true);
        font.setPixelSize(14);
        painter.setFont(font);

        if (!active) {
            QRadialGradient gradient(width() / 2, height() / 2, width() * 1.2, width() / 3, height() / 2);
            gradient.setColorAt(0, AppSkin::kTabBtnHoverColor);
            gradient.setColorAt(1, 0xffffff);

            int font_color = 0;
            QBrush brush(Qt::BrushStyle::SolidPattern);
            if (enter) {
                brush = QBrush(gradient);
                font_color = 0xffffff;
            } else {
                brush.setColor(enter ? hover_color : inactive_color);
                font_color = 0x333333;
            }

            painter.setBrush(brush);
            painter.setPen(Qt::NoPen);
            int offset = 0;
            painter.drawRoundedRect(0 + offset, 0 + offset, width() - 2 * offset, height() - 2 * offset, border_radius_, border_radius_);

            QFontMetrics fm = painter.fontMetrics();
            QSize s = fm.size(0, text);
            int width = s.width();//fm.width(text);
            int height = fm.descent() + fm.ascent();
            painter.setPen(QPen(QColor(font_color)));
            painter.drawText((this->width() - width) / 2, (this->height() - height) / 2, width, height, 0, text);
        } else {
            //QRect rect(0, 0, width(), height());
            //painter.drawPixmap(rect, active_bg);
            QRadialGradient gradient(width() / 2, height() / 2, width() * 1.2, width() / 4, height() / 2);
            gradient.setColorAt(0, AppSkin::kTabBtnHoverColor);
            gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
            auto brush = QBrush(gradient);
            painter.setBrush(brush);
            painter.setPen(Qt::NoPen);
            int offset = 0;
            painter.drawRoundedRect(0 + offset, 0 + offset, width() - 2 * offset, height() - 2 * offset, border_radius_, border_radius_);

            QFontMetrics fm = painter.fontMetrics();
            QSize s = fm.size(0, text);
            int width = s.width();//fm.width(text);
            int height = fm.descent() + fm.ascent();
            painter.setPen(QPen(QColor(0xffffff)));
            painter.drawText((this->width() - width) / 2, (this->height() - height) / 2, width, height, 0, text);

        }

        //QPushButton::paintEvent(event);
    }

    void CustomTabBtn::enterEvent(QEnterEvent *event) {
        QPushButton::enterEvent(event);
        enter = true;
        repaint();
    }

    void CustomTabBtn::leaveEvent(QEvent *event) {
        QPushButton::leaveEvent(event);
        enter = false;
        repaint();
    }

    void CustomTabBtn::ToActiveStatus() {
        active = true;
        repaint();
    }

    void CustomTabBtn::ToInActiveStatus() {
        active = false;
        repaint();
    }

}
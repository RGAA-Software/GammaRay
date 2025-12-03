#include "gd_custom_progress_bar.h"

GDCustomProgressBar::GDCustomProgressBar(QWidget* parent) : QWidget(parent) {

	setMinimumHeight(30);
}

void GDCustomProgressBar::setValue(int value) {
    m_value = qBound(0, value, 100);
    update();
}

void GDCustomProgressBar::setError(bool error) {
    m_error = error;
    update();
}

void GDCustomProgressBar::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();
    int barHeight = 6;
    QRect barRect(rect.left(), rect.bottom() - barHeight, rect.width(), barHeight);

    painter.setBrush(QColor("#ebebeb"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(barRect, 3, 3);

    QColor progressColor = m_error ? QColor("#fe3539") : QColor("#3094ff");
    int progressWidth = rect.width() * m_value / 100;
    QRect progressRect(barRect.left(), barRect.top(), progressWidth, barHeight);
    painter.setBrush(progressColor);
    painter.drawRoundedRect(progressRect, 3, 3);

    QString text = QString("%1%").arg(m_value);
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(14);
    font.setItalic(true);
    painter.setFont(font);

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text) + 10;
    int textX = rect.left() + progressWidth - textWidth / 2;
    textX = qBound(rect.left(), textX, rect.right() - textWidth);
    int textY = barRect.top() - 18; 

    painter.setPen(Qt::black);
    painter.drawText(textX, textY, textWidth, fm.height(),Qt::AlignCenter, text);
}

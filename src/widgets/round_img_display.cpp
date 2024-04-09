#include "round_img_display.h"

#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QDebug>
#include <QImage>
#include <QPainterPath>

namespace tc
{
    RoundImageDisplay::RoundImageDisplay(const QString &path, int width, int height, int radius, QWidget *parent)
            : QLabel(parent) {
        setFixedSize(width, height);
        this->radius = radius;
        placeholder = QPixmap(":/images/resources/ic_cover.jpg");
    }

    RoundImageDisplay::RoundImageDisplay(const QPixmap &pixmap, int radius, QWidget *parent) : QLabel(parent) {
        this->radius = radius;
        this->pixmap = pixmap;
    }

    RoundImageDisplay::~RoundImageDisplay() {

    }

    void RoundImageDisplay::resizeEvent(QResizeEvent *event) {
        auto size = event->size();
        mask = QBitmap(size);
        //qDebug() << "size :" << size;
    }

    void RoundImageDisplay::UpdatePixmap(const QPixmap& px) {
        std::lock_guard<std::mutex> guard(pixmap_mutex_);
        this->pixmap = px;
        this->repaint();
    }

    void RoundImageDisplay::paintEvent(QPaintEvent *event) {
        QLabel::paintEvent(event);
        QPainter painter(this);

        std::lock_guard<std::mutex> guard(pixmap_mutex_);

        if (!pixmap.isNull() && painter.isActive()) {
            painter.setRenderHints(QPainter::Antialiasing, true);
            painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
            QPainterPath path;
            path.addRoundedRect(mask.rect(), radius, radius);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, pixmap);
        } else {
            painter.setBrush(QBrush(QColor(0x33, 0x33, 0x33)));
            painter.drawRoundedRect(0, 0, mask.width(), mask.height(), radius, radius);
        }

    }
}
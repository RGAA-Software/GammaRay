#pragma once

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QBitmap>
#include <QPixmap>

#include <memory>
#include <mutex>

namespace tc
{
    class RoundImageDisplay : public QLabel {

    public:
        RoundImageDisplay(const QString &path, int width, int height, int radius, QWidget *parent = nullptr);
        RoundImageDisplay(const QPixmap &pixmap, int radius, QWidget *parent = nullptr);
        ~RoundImageDisplay() override;
        void paintEvent(QPaintEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

        void UpdatePixmap(const QPixmap& pixmap);

    private:
        QBitmap mask;
        QPixmap pixmap;
        int radius;
        std::mutex pixmap_mutex_;

    };

}
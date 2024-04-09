#pragma once

#include <QFont>
#include <QEvent>
#include <QColor>
#include <QPushButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>

namespace tc
{

    class CustomTabBtn : public QPushButton {
    Q_OBJECT

    public:

        explicit CustomTabBtn(QWidget *parent = 0);
        ~CustomTabBtn() override;

        void SetSelectedFontColor(const std::string &color);
        void SetBorderRadius(int r) { border_radius_ = r; }

        void ToActiveStatus();
        void ToInActiveStatus();

        void SetText(const QString &text);

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;

    private:

        QColor hover_color = QColor(255, 160, 90);
        QColor inactive_color = QColor(160, 160, 160);

        bool active = false;
        //QPixmap   active_bg;

        bool enter = false;
        QString text;
        int border_radius_ = 0;
    };

}
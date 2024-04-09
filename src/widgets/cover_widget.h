#pragma once

#include <QWidget>
#include <QEvent>
#include <QPaintEvent>
#include <QFont>
#include <memory>

namespace tc
{
    class CoverWidget : public QWidget {
    Q_OBJECT

    public:
        CoverWidget(QWidget *parent = nullptr, int offset = 0);
        ~CoverWidget();

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void SetTagEnableStatus(bool enable);
        void UpdateTagText(const QString &t);

    private:

        bool cursor_enter = false;
        int widget_alpha = 0;
        int target_alpha = 120;
        int offset = 0;

        QFont title_font;

        QString tag_text = "1920x1080";
        bool enable_tab_display = true;
    };

}
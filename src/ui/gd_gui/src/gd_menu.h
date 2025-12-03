#pragma once

#include <QMenu>
#include <qevent.h>

class GDMenu : public QMenu {
    Q_OBJECT
public:
    explicit GDMenu(QWidget* parent = nullptr);
    void init();
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;
signals:
    void SigHide();
};

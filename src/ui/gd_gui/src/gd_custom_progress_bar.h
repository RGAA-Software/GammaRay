#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>

class GDCustomProgressBar : public QWidget {
    Q_OBJECT
public:
    explicit GDCustomProgressBar(QWidget* parent = nullptr);
    void setValue(int value);
    void setError(bool error);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    int m_value = 0;
    bool m_error = false;
};

#pragma once

#include <qwidget.h>
#include <qpainter.h>
#include <qslider.h>
#include <qevent.h>

class GDProgressBar : public QSlider {
	Q_OBJECT
public:
	GDProgressBar(QWidget* parent = nullptr);
	~GDProgressBar();
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	void Init();

signals:
	void SigPosChanged();
};
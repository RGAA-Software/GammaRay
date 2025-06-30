#pragma once
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>

namespace tc {

class ConnectedInfoTag : public QWidget {
public:
	ConnectedInfoTag(QWidget* parent = nullptr);
	void paintEvent(QPaintEvent* event) override;
	void CreateLeftPath(QPainterPath& path);
};

}
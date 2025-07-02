#pragma once
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>


namespace tc {

class ConnectedInfoTag : public QWidget {
	Q_OBJECT
public:
	ConnectedInfoTag(QWidget* parent = nullptr);
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	bool GetExpanded() const;
	void SetExpanded(bool expanded);
private:
	bool expanded_ = true;
};

}
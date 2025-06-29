#pragma once

#include <qwidget.h>
#include <qpushbutton.h>
#include <QPaintEvent>
#include <QPen>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <qevent.h>
#include <QMouseEvent>
#include <functional>

namespace tc {

using OnClickedCallback = std::function<void()>;

class FileTransLogSwitchButton : public QPushButton {
	Q_OBJECT
public:
	FileTransLogSwitchButton(QWidget* parent);
	~FileTransLogSwitchButton() = default;

	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e);
	void setOnClickedCallback(OnClickedCallback func);
	void setActive(bool state);
	bool getActive();
private:
	bool mIsActive = false;
	OnClickedCallback mOnClickedCallback = nullptr;
};

}
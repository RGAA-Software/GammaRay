#include "file_trans_log_switch_button.h"
#include <qbrush.h>

namespace tc {
FileTransLogSwitchButton::FileTransLogSwitchButton(QWidget* parent) : QPushButton(parent) {

}

void FileTransLogSwitchButton::paintEvent(QPaintEvent* e) {
	QPushButton::paintEvent(e);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(Qt::NoPen);
	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	if (mIsActive) {
		brush.setColor(QColor(0x4e, 0x99, 0xec));
	}
	else {
		brush.setColor(QColor(0xff, 0xff, 0xff));
	}
	painter.setBrush(brush);
	painter.drawRoundedRect((width() - 24) / 2, height() - 2, 24, 2, 0, 0);
}

void FileTransLogSwitchButton::mousePressEvent(QMouseEvent* e) {
	if (mOnClickedCallback) {
		mOnClickedCallback();
	}
	QPushButton::mousePressEvent(e);
}

void FileTransLogSwitchButton::setOnClickedCallback(std::function<void()> func) {
	mOnClickedCallback = func;
}

void FileTransLogSwitchButton::setActive(bool b) {
	mIsActive = b;
	repaint();
	update();
}

bool FileTransLogSwitchButton::getActive() {
	return mIsActive;
}
}
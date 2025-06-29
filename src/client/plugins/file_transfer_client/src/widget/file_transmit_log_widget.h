#pragma once
#include <qwidget.h>
#include <qpainter.h>
class QTextEdit;
class QVBoxLayout;

namespace tc {
class FileTransmitLogWidget : public QWidget {
	Q_OBJECT
public:
	FileTransmitLogWidget(QWidget* parent = nullptr);
	~FileTransmitLogWidget();
	virtual void paintEvent(QPaintEvent* qpevent) override;
	void Init();
	QVBoxLayout* main_vbox_layout_ = nullptr;
	QTextEdit* text_edit_ = nullptr;

};
}
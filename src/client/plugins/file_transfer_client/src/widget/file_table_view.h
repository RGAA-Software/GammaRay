#pragma once
#include <qtableview.h>
#include <qevent.h>
#include <qmenu.h>
#include <qdebug.h>

namespace tc {

class FileTableView : public QTableView {
	Q_OBJECT
public:

	FileTableView(QWidget* parent = nullptr);
	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

signals:
	void SigHoverIndexChanged(QModelIndex);
	void SigLeaveEvent();
	void SigContextClicked(QModelIndex, QPoint);
	void SigFocusOut();
	void SigMousePressed();
private:
	QModelIndex context_index_;
};

}
#include "file_table_view.h"
#include <iostream>>
#include <qdebug.h>
#include <qwidgetaction.h>
#include <qicon.h>

namespace tc {

FileTableView::FileTableView(QWidget* parent) : QTableView(parent) {}

void FileTableView::mouseMoveEvent(QMouseEvent* event) {
	QModelIndex index = indexAt(event->pos());
	emit SigHoverIndexChanged(index);
	QTableView::mouseMoveEvent(event);
}

void FileTableView::leaveEvent(QEvent* event) {
	emit SigLeaveEvent();
	QTableView::leaveEvent(event);
}

void FileTableView::contextMenuEvent(QContextMenuEvent* event)
{
	// 限定于 文件名那一列
	auto pos = event->pos();
	pos.setX(1);
	context_index_ = indexAt(pos);
	// 这里的坐标 是 减去 表头高度的
	//std::cout << "event->pos() x = " << event->pos().x() << " y = " << event->pos().y() << std::endl;
	emit SigContextClicked(context_index_, event->globalPos());
}

void FileTableView::focusOutEvent(QFocusEvent* event) {
	emit SigFocusOut();
	QTableView::focusOutEvent(event);
	//std::cout << "FileTableView::focusOutEvent" << std::endl;
}

void FileTableView::mousePressEvent(QMouseEvent* event) {
	//std::cout << "FileTableView::mousePressEvent" << std::endl;
	emit SigMousePressed();
	QTableView::mousePressEvent(event);
}

}
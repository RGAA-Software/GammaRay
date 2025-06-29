#pragma once
#include <QAbstractTableModel>
#include <qstyleditemdelegate.h>
#include <iostream>
#include <qlineedit.h>
#include <qevent.h>
#include "file_detail_info.h"

namespace tc {

class FileInfoTableViewBtnDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit FileInfoTableViewBtnDelegate(QObject* parent = nullptr);
	QLineEdit* editor_ = nullptr;
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
		editor->installEventFilter(const_cast<FileInfoTableViewBtnDelegate*>(this));
		return editor;
	}

	bool eventFilter(QObject* obj, QEvent* event) override
	{
		if (event->type() == QEvent::FocusIn) {
			//std::cout << " FileInfoTableViewBtnDelegate FocusIn" << std::endl;
			if (QLineEdit* editor = qobject_cast<QLineEdit*>(obj))
			{
				editor_ = editor;
				editor_->grabKeyboard();
				editor_->setFocus();
			}
		}
		else if (event->type() == QEvent::FocusOut)
		{
			if (QLineEdit* editor = qobject_cast<QLineEdit*>(obj))
			{
				editor_->releaseKeyboard();
				// 编辑器失去焦点
				//std::cout << " FileInfoTableViewBtnDelegate FocusOut" << std::endl;
				emit SigFocusOut();
			}
		}
		else if (event->type() == QEvent::KeyPress) {
			//std::cout << " FileInfoTableViewBtnDelegate KeyPress 0 " << std::endl;
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
				//std::cout << " FileInfoTableViewBtnDelegate KeyPress " << std::endl;
				emit SigActivated();
			}
		}
		return QStyledItemDelegate::eventFilter(obj, event);
	}
#if 0
	bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override
	{
		std::cout << " FileInfoTableViewBtnDelegate editorEvent----------------------------------------------------" << std::endl;
		if (event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::RightButton)
			{
				// 处理右键单击事件
				std::cout << " FileInfoTableViewBtnDelegate editorEvent RightButton" << std::endl;
				//return true; // 停止事件传播，防止编辑器失去焦点
			}
		}
		else if (event->type() == QEvent::KeyPress) {
			std::cout << " FileInfoTableViewBtnDelegate editorEvent KeyPress 0 " << std::endl;
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
				std::cout << " FileInfoTableViewBtnDelegate editorEvent KeyPress " << std::endl;
				emit SigActivated();
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
#endif

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	bool mouse_in_flag_ = false;
signals:
	void SigFocusOut();
	void SigActivated();
private:
	QModelIndex edit_index_;
	int hover_row_;
	QBrush hover_item_background_color_;
public slots:
	void OnHoverIndexChanged(const QModelIndex& index);
	void OnMouseLeaveChanged();

};

enum class FileInfoTableColumnType {
	kColumnName,           //文件名字
	kColumnSize,           //文件大小
	kColumnType,           //文件类型
	kColumnDateChanaged,   //修改时间
};

class FileInfoTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	FileInfoTableModel(const FileContainer& file_container);
	QStringList m_headers;
public:
	Q_INVOKABLE virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	Q_INVOKABLE virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	void beginSetData() { beginResetModel(); }
	void endSetData() { endResetModel(); }

	void refresh()
	{
		beginResetModel();
		endResetModel();
	}

	// 更新要展示的文件列表
	void Updatefilecontainer(const FileContainer& file_container);

private:
	bool is_select_all = false;

	FileContainer file_container_;
};

}

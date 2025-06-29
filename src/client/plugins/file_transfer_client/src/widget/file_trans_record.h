#pragma once
#include <map>
#include <tuple>
#include <qtableview.h>
#include <qevent.h>
#include <qstyleditemdelegate.h>
#include "file_transmit_task_state.h"
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QMenu;
class QPushButton;

namespace tc {

class FileTransRecordDetailInfo {
public:
	FileTransRecordDetailInfo() = default;
	~FileTransRecordDetailInfo() = default;

	QIcon file_icon_;
	QString file_name_;
	uint64_t file_size_ = 0;
	QString speed_ = "--";
	uint8_t schedule_ = 0;

	QString origin_path_;
	QString target_path_;

	EFileTransmitTaskState state_ = EFileTransmitTaskState::kWait;
	EFileTransmitTaskType type_ = EFileTransmitTaskType::kUnKnown;
	EFileTransmitTaskErrorCause cause_ = EFileTransmitTaskErrorCause::kPlaceholder;
};

class  FileTransRecordContainer {
public:
	static FileTransRecordContainer* Instance()
	{
		static FileTransRecordContainer self{};
		return &self;
	}
	FileTransRecordContainer() = default;
	~FileTransRecordContainer() = default;
	int Size();
	void Calculate();
	int CompletedSize();
	int FailedSize();
	std::map<QString, FileTransRecordDetailInfo> files_trans_record_info_;
private:
	int completed_size_ = 0;
	int failed_size_ = 0;
};

class FileTransRecordTableViewBtnDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit FileTransRecordTableViewBtnDelegate(QObject* parent = nullptr);

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	bool editorEvent(QEvent* event, QAbstractItemModel* model,
		const QStyleOptionViewItem& option, const QModelIndex& index) override;

	bool mouse_in_flag_ = false;

	QMap<QModelIndex, QStyleOptionButton*> m_btns;
signals:
	void deleteData(const QModelIndex& index);
private:
	enum class ButtonState {
		kUnKnow,
		kMouseOver,
		kMousePress,
	};
	int hover_row_;
	QBrush hover_item_background_color_;

	QPoint mouse_point_;  // 鼠标位置
	ButtonState btn_state_ = ButtonState::kUnKnow;
	int row_ = -1;

	QIcon del_icon_;
	QIcon del_icon_hover_;

public slots:
	void OnHoverIndexChanged(const QModelIndex& index);
};

enum class FileTransRecordTableColumnType {
	kColumnName,
	kColumnSchedule,
	kColumnSize,
	kColumnSpeed,
	kColumnOriginPath,
	kColumnTargetPath,
	kColumnDirection,
	kColumnOperation,
};

class FileTransRecordTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	static FileTransRecordTableModel* Instance()
	{
		static FileTransRecordTableModel self{};
		return &self;
	}
	FileTransRecordTableModel();
	QStringList m_headers;
public:
	Q_INVOKABLE virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	void beginSetData() { beginResetModel(); }
	void endSetData() { endResetModel(); }

	void refresh()
	{
		beginResetModel();
		endResetModel();
	}
	void Updatefilecontainer();

private:
	bool is_select_all_ = false;
};



class FileTransRecordTableView : public QTableView {
	Q_OBJECT
public:
	FileTransRecordTableView(QWidget* parent = nullptr);
	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
signals:
	void SigHoverIndexChanged(QModelIndex);
	void SigLeaveEvent();
	void SigContextClicked(QModelIndex, QPoint);
};

// 文件传输日志
class FileTransRecord : public QWidget {
	Q_OBJECT
public:
	FileTransRecord(QWidget* parent = nullptr);
	~FileTransRecord();
	void Init();
	void InitContextMenu();
	void ClearCompletedTasksRecord();
	std::tuple<int, int, int> GetStatisticsInfo();
private:
	void OnContextMenuEvent(QModelIndex index, QPoint pos);
private:
	QVBoxLayout* main_vbox_layout_ = nullptr;
	FileTransRecordTableView* file_trans_record_table_view_ = nullptr;

	QMenu* context_menu_ = nullptr;
	QPushButton* open_file_explorer_btn_ = nullptr;
	QString to_be_opened_file_path_;
	
};

}
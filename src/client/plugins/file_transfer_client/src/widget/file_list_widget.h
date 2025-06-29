#pragma once 
#include <stack>
#include <memory>
#include <mutex>
#include <qwidget.h>
#include <qboxlayout.h>
#include <qdir.h>
#include "file_const_def.h"
#include "file_table_model.h"
#include "file_detail_info.h"
#include "file_table_view.h"

class QPushButton;
class QMenu;

namespace tc {

class BaseFileUtil;
class FileInfoTableViewBtnDelegate;


// 实际用来展示文件列表的类
class FileListWidget : public QWidget {
	Q_OBJECT
public:
	FileListWidget(FileAffiliationType affiliation_type, QWidget* parent = nullptr, QString path = "");
	~FileListWidget();
	void Init(QString path = "");
	//返回
	void GoBack();
	void Refresh();
	void GoParentDir();
	//返回首页
	void GoIndex();
	void GoTargetPath(const QString& path);
	//获取选中的文件、文件夹
	FileContainer GetSelectFiles();

	QDir GetCurrentDir();
	void SearchByFileName(const QString& file_name);
	bool HasDisk() {
		return current_file_container_has_disk_;
	}
	bool HasHistory() {
		return !history_dirs_.empty();
	}
	bool IsRootPath() {
		return current_dir_ == tc::kRealRootPath;
	}
	void ExitPersistentEditor();
private:
	FileAffiliationType affiliation_type_ = FileAffiliationType::kLocal;
	std::shared_ptr<BaseFileUtil> file_util_ = nullptr;
	FileTableView* file_table_view_ = nullptr;
	FileInfoTableModel* file_info_table_model_ = nullptr;
	FileContainer current_file_container_;
	FileContainer current_file_container_bak_; // 备份，搜索用
	bool current_file_container_has_disk_ = false; // 当前列表页 是否含有磁盘, 因为在windows系统中，包含磁盘列表那一页(一般为根目录)，就没必要显示 相关右键操作了(重命名、删除、新建文件夹等)

	// 表头类型与排序规则(升序 降序) 
	std::map<EFileTableHeaderViewItemType, bool> header_item_type_map_sort_;
	void Sort(EFileTableHeaderViewItemType header_item_type);
	std::pair<EFileTableHeaderViewItemType, bool> current_header_item_type_sort_;

	//文件路径浏览的目录，用于返回
	std::stack<QDir> history_dirs_;

	//上次浏览的目录
	QDir last_dir_;

	//当前展示的目录
	QDir current_dir_;

	FileInfoTableViewBtnDelegate* item_delegate_ = nullptr;

	void InitSigChannel();
	void InitHeaderViewSigChannel();
	void NotifyCurrentDirPathChange();

	// 鼠标右键操作相关 ---start---
	void OnContextMenuEvent(QModelIndex index, QPoint pos);
	void InitContextMenu();
	QMenu* menu_index_valid_ = nullptr;
	QPushButton* btn_rename_ = nullptr;
	QPushButton* btn_del_ = nullptr;
	QPushButton* btn_new_folder_ = nullptr;
	QPushButton* btn_select_all_ = nullptr;

	QMenu* menu_index_novalid_ = nullptr;
	QPushButton* btn_new_folder2_ = nullptr;
	QPushButton* btn_select_all2_ = nullptr;

	// 表示当前编辑的索引
	QModelIndex context_menu_index_;
	// 鼠标右键操作相关 ---end---

	// 表示当前编辑的索引是否打开编辑模式了
	bool current_index_opened_ = false;

	void ReName(const QString& abs_path, const QString& name);

	void HandleNewFolder();
	QString create_new_folder_path_;
	bool create_new_folder_res_ = false;
signals:
	void SigUpdateCurrentDirPath(QString);
	void SigCurrentFileContainterHasDisk(bool);
	void SigTableViewMousePressed();
public slots:
	void OnRowDoubleClicked(const QModelIndex&);
};
}
#include "file_list_widget.h"
#include <set>
#include <qtableview.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstandardpaths.h>
#include <qfileiconprovider.h>
#include <qstorageinfo.h>
#include <qheaderview.h>
#include <qpushbutton.h>
#include <qtablewidget.h>
#include <qscrollbar.h>
#include <qsizepolicy.h>
#include <qwidgetaction.h>
#include <qpushbutton.h>
#include <qmenu.h>
#include <QRegularExpression>
#include <vector>
#include <qsettings.h>
#include "file_table_view_style.h"
#include "local_file_util.h"
#include "remote_file_util.h"
#include "file_table_view.h"
#include "file_const_def.h"
#include "file_log_manager.h"
#include "tc_label.h"

#define TC_TABLE_HEADER_HEIGHT 25
#define TC_TABLE_FILE_NAME_COLUMN 0

namespace tc {

static int s_gam_real_table_header_height = 25;

FileListWidget::FileListWidget(FileAffiliationType affiliation_type, QWidget* parent, QString path) : affiliation_type_(affiliation_type), QWidget(parent)
{
	Init(path);
}

FileListWidget::~FileListWidget() {

}

void FileListWidget::Init(QString last_path) {
	if (FileAffiliationType::kLocal == affiliation_type_) {
		file_util_ = std::make_shared<LocalFileUtil>();
	}
	else if (FileAffiliationType::kRemote == affiliation_type_) {
		file_util_ = std::make_shared<RemoteFileUtil>();
	}
	QHBoxLayout* main_hboxlayout = new QHBoxLayout();
	main_hboxlayout->setContentsMargins(0,0,0,0);
	main_hboxlayout->setSpacing(0);
	setLayout(main_hboxlayout);
	file_info_table_model_ = new FileInfoTableModel(current_file_container_);
	/*file_info_table_model_->m_headers
		<< QStringLiteral("名称")
		<< QStringLiteral("大小")
		<< QStringLiteral("类型")
		<< QStringLiteral("修改时间");*/
	file_info_table_model_->m_headers
		<< tcTr("id_file_trans_name")
		<< tcTr("id_file_trans_size")
		<< tcTr("id_file_trans_type")
		<< tcTr("id_file_trans_modify_time");
	file_table_view_ = new FileTableView(this);
	main_hboxlayout->addWidget(file_table_view_);
	file_table_view_->setAttribute(Qt::WA_StyledBackground);
	file_table_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	file_table_view_->setObjectName("tableView");
	file_table_view_->verticalHeader()->hide();
	//file_table_view_->verticalHeader()->setDefaultSectionSize(TC_TABLE_HEADER_HEIGHT); //设置高度
	file_table_view_->horizontalHeader()->setFixedHeight(s_gam_real_table_header_height);
	file_table_view_->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度

	file_table_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	file_table_view_->verticalScrollBar()->setStyleSheet(QString::fromStdString(kScrollBarStyle));

	file_table_view_->setShowGrid(false);
	file_table_view_->setAlternatingRowColors(true); // 奇数 偶数 颜色差异

	file_table_view_->horizontalHeader()->setHighlightSections(false);
	//file_table_view_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft); 无效  要在tablemodel里面做
	file_table_view_->setSelectionBehavior(QAbstractItemView::SelectRows); //设置选择行为时每次选择一行
	file_table_view_->setStyleSheet(QString::fromStdString(KFileTableStyle));
	QFont font = file_table_view_->horizontalHeader()->font(); //设置表头字体加粗
	//font.setBold(true);
	file_table_view_->horizontalHeader()->setFont(font);
	file_table_view_->setModel(file_info_table_model_);
	file_table_view_->horizontalHeader()->resizeSection(static_cast<int>(EFileTableHeaderViewItemType::KName), 330); //设置表头第一列的宽度
	file_table_view_->horizontalHeader()->resizeSection(static_cast<int>(EFileTableHeaderViewItemType::KSize), 80);
	file_table_view_->horizontalHeader()->resizeSection(static_cast<int>(EFileTableHeaderViewItemType::KType), 60);
	file_table_view_->horizontalHeader()->resizeSection(static_cast<int>(EFileTableHeaderViewItemType::KUpdateTime), 140);
	file_table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
	file_table_view_->horizontalHeader()->setSectionsMovable(QHeaderView::ResizeToContents);
	file_table_view_->setDragDropMode(QAbstractItemView::NoDragDrop);

	item_delegate_ = new FileInfoTableViewBtnDelegate(this);
	file_table_view_->setItemDelegate(item_delegate_);
	file_table_view_->setMouseTracking(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	file_table_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	file_table_view_->setFocusPolicy(Qt::NoFocus);
	adjustSize();
	InitContextMenu();
	InitSigChannel();
	if (last_path.isEmpty()) {
		current_dir_ = tc::kRealRootPath;
		file_util_->GetThisPCFiles();
	}
	else {
		if (tc::kRealRootPath == last_path) {
			file_util_->GetThisPCFiles();
		}
		else {
			file_util_->GetFiles(last_path);
		}
		current_dir_ = last_path;
	}

	header_item_type_map_sort_[EFileTableHeaderViewItemType::KName] = false;
	header_item_type_map_sort_[EFileTableHeaderViewItemType::KSize] = true;
	header_item_type_map_sort_[EFileTableHeaderViewItemType::KType] = true;
	header_item_type_map_sort_[EFileTableHeaderViewItemType::KUpdateTime] = true;

	current_header_item_type_sort_.first = EFileTableHeaderViewItemType::KName;
	current_header_item_type_sort_.second = true;
}

void FileListWidget::InitContextMenu() {
	QString btn_css = "QPushButton{text-align: left; padding-left: %1px; border:none; background-color: #ffffff; font-family:Microsoft YaHei; font-size:%2px; color:#000000;}\
					QPushButton:hover{border:none; background-color: #F5F5F5; font-family:Microsoft YaHei; color:#000000;}";
	QString menu_css = R"(QMenu { background-color: #f9f9fd; border-radius: %1px;
		border: 0px solid #e0e0e0;	
		padding: %2px;
		box-shadow: 4px 4px 8px rgba(0, 0, 0, 0.4); 
	})";
	btn_css = btn_css.arg(4).arg(12);
	menu_css = menu_css.arg(4).arg(4);
	QIcon* icon_rename = new QIcon(":/resource/rename.svg");
	QIcon* icon_del = new QIcon(":/resource/del.svg");
	QIcon* icon_new_folder = new QIcon(":/resource/new_folder.svg");
	QIcon* icon_select_all = new QIcon(":/resource/select_all.svg");

	menu_index_valid_ = new QMenu();
	menu_index_valid_->setStyleSheet(menu_css);
	menu_index_valid_->setFixedSize(116, 140);

	btn_rename_ = new QPushButton(this);
	btn_rename_->setStyleSheet(btn_css);
	btn_rename_->setFixedSize(100, 30);
	btn_rename_->setIcon(*icon_rename);
	//btn_rename_->setText(QStringLiteral("重命名"));
	btn_rename_->setText(tcTr("id_file_trans_rename"));
	QWidgetAction* action_rename = new QWidgetAction(menu_index_valid_);
	action_rename->setDefaultWidget(btn_rename_);
	menu_index_valid_->addAction(action_rename);

	btn_del_ = new QPushButton(this);
	btn_del_->setStyleSheet(btn_css);
	btn_del_->setFixedSize(110, 30);
	//btn_del_->setText(QStringLiteral("删除"));
	btn_del_->setText(tcTr("id_file_trans_del"));
	btn_del_->setIcon(*icon_del);
	QWidgetAction* action_del = new QWidgetAction(menu_index_valid_);
	action_del->setDefaultWidget(btn_del_);
	menu_index_valid_->addAction(action_del);

	btn_new_folder_ = new QPushButton(this);
	btn_new_folder_->setStyleSheet(btn_css);
	btn_new_folder_->setFixedSize(110, 30);
	//btn_new_folder_->setText(QStringLiteral("新建文件夹"));
	btn_new_folder_->setText(tcTr("id_file_trans_new_folder"));
	btn_new_folder_->setIcon(*icon_new_folder);
	QWidgetAction* action_new_folder = new QWidgetAction(menu_index_valid_);
	action_new_folder->setDefaultWidget(btn_new_folder_);
	menu_index_valid_->addAction(action_new_folder);

	btn_select_all_ = new QPushButton(this);
	btn_select_all_->setStyleSheet(btn_css);
	btn_select_all_->setFixedSize(110, 30);
	//btn_select_all_->setText(QStringLiteral("全选"));
	btn_select_all_->setText(tcTr("id_file_trans_select_all"));
	btn_select_all_->setIcon(*icon_select_all);
	QWidgetAction* action_select_all = new QWidgetAction(menu_index_valid_);
	action_select_all->setDefaultWidget(btn_select_all_);
	menu_index_valid_->addAction(action_select_all);


	menu_index_novalid_ = new QMenu();
	menu_index_novalid_->setStyleSheet(menu_css);
	menu_index_novalid_->setFixedSize(110, 60);

	btn_new_folder2_ = new QPushButton(this);
	btn_new_folder2_->setStyleSheet(btn_css);
	btn_new_folder2_->setFixedSize(110, 30);
	//btn_new_folder2_->setText(QStringLiteral("新建文件夹"));
	btn_new_folder2_->setText(tcTr("id_file_trans_new_folder"));
	btn_new_folder2_->setIcon(*icon_new_folder);
	QWidgetAction* action_new_folder2 = new QWidgetAction(menu_index_novalid_);
	action_new_folder2->setDefaultWidget(btn_new_folder2_);
	menu_index_novalid_->addAction(action_new_folder2);

	btn_select_all2_ = new QPushButton(this);
	btn_select_all2_->setStyleSheet(btn_css);
	btn_select_all2_->setFixedSize(110, 30);
	btn_select_all2_->setIcon(*icon_select_all);
	//btn_select_all2_->setText(QStringLiteral("全选"));
	btn_select_all2_->setText(tcTr("id_file_trans_select_all"));

	QWidgetAction* action_select_all2 = new QWidgetAction(menu_index_novalid_);
	action_select_all2->setDefaultWidget(btn_select_all2_);
	menu_index_novalid_->addAction(action_select_all2);
}

void FileListWidget::InitSigChannel() {
	file_table_view_->setMouseTracking(true);
	connect(file_table_view_, &FileTableView::SigHoverIndexChanged, item_delegate_, &FileInfoTableViewBtnDelegate::OnHoverIndexChanged);

	connect(file_table_view_, &FileTableView::SigLeaveEvent, this, [=]() {
		item_delegate_->mouse_in_flag_ = false;
		file_table_view_->update(); // 需要立即刷新，不然hover效果会一直在
		});
	connect(file_table_view_, &QTableView::doubleClicked, this, &FileListWidget::OnRowDoubleClicked);

	connect(file_util_.get(), &BaseFileUtil::SigGetFiles, this, [=](FileContainer file_container) {
		//每次获取到文件列表，按照当前排序规则进行排序
		file_container.Sort(current_header_item_type_sort_.first, current_header_item_type_sort_.second);
		current_file_container_has_disk_ = file_container.HasDisk();
		emit SigCurrentFileContainterHasDisk(current_file_container_has_disk_);
		current_file_container_bak_ = current_file_container_ = file_container;
		file_info_table_model_->Updatefilecontainer(file_container);
		HandleNewFolder();
	});

	connect(file_util_.get(), &BaseFileUtil::SigRemove, this, [=]() {
		Refresh();
		});

	connect(file_util_.get(), &BaseFileUtil::SigReName, this, [=]() {
		Refresh();
		});

	connect(file_util_.get(), &BaseFileUtil::SigCreateNewFolder, this, [=](QString folder_path) {
		create_new_folder_res_ = true;
		create_new_folder_path_ = folder_path;
		Refresh();
		});

	connect(item_delegate_, &FileInfoTableViewBtnDelegate::SigFocusOut, this, [=]() {
		//std::cout << "FileInfoTableViewBtnDelegate::SigFocusOut context_menu_index_ " << context_menu_index_.row() << " " << context_menu_index_.column() << std::endl;
		QPoint mousePos = QCursor::pos();
		QPoint widgetPos = file_table_view_->mapFromGlobal(mousePos);
		//std::cout << "mousePos.x = " << mousePos.x() << " y = " << mousePos.y() << std::endl;
		//std::cout << "widgetPos.x = " << widgetPos.x() << " y = " << widgetPos.y() << std::endl;
		widgetPos.setY(widgetPos.y() - s_gam_real_table_header_height);
		QRect cellRect = file_table_view_->visualRect(context_menu_index_); // 获取单元格的几何边界
		//std::cout << "1 cellRect.x = " << cellRect.x() << " y = " << cellRect.y() << " w = " << cellRect.width() << " h  = " << cellRect.height() << std::endl;
		// !cellRect.contains ,在此单元格重命名，会再次触发 SigFocusOut，
		if (!cellRect.contains(widgetPos) && current_index_opened_) {
			ExitPersistentEditor();
		}
	});

	connect(item_delegate_, &FileInfoTableViewBtnDelegate::SigActivated, this, [=]() {
		//std::cout << "FileInfoTableViewBtnDelegate::SigActivated context_menu_index_ " << context_menu_index_.row() << " " << context_menu_index_.column() << std::endl;
		ExitPersistentEditor();
	});

	connect(btn_rename_, &QPushButton::clicked, this, [=]() {
		menu_index_valid_->hide();
		menu_index_novalid_->hide();
		file_table_view_->setFocus();
		file_table_view_->openPersistentEditor(context_menu_index_);
		file_table_view_->edit(context_menu_index_);   // 使光标闪烁
		file_table_view_->viewport()->update();
		if (item_delegate_->editor_) {
			file_table_view_->setFocusProxy(item_delegate_->editor_);
		}
		current_index_opened_ = true;
	});

	connect(btn_del_, &QPushButton::clicked, this, [=]() {
		menu_index_valid_->hide();
		menu_index_novalid_->hide();
		// 如果是win32 根目录 就不要有删除功能
		if (tc::kRealRootPath == current_dir_.absolutePath()) {
			return;
		}
		std::vector<QString> del_paths;
		auto files_select = GetSelectFiles();
		for (auto file_info : files_select.files_detail_info_) {
			del_paths.push_back(file_info.file_path_);
		}
		file_util_->Remove(del_paths);
	});

	connect(btn_new_folder_, &QPushButton::clicked, this, [=]() {
		menu_index_valid_->hide();
		menu_index_novalid_->hide();
		auto path = current_dir_.absolutePath();
		file_util_->CreateNewFolder(path);
	});

	connect(btn_new_folder2_, &QPushButton::clicked, this, [=]() {
		menu_index_valid_->hide();
		menu_index_novalid_->hide();
		auto path = current_dir_.absolutePath();
		file_util_->CreateNewFolder(path);
	});

	connect(btn_select_all_, &QPushButton::clicked, this, [=]() {
		menu_index_valid_->hide();
		menu_index_novalid_->hide();
		QItemSelectionModel* selectionModel = file_table_view_->selectionModel();
		QModelIndex topLeft = file_info_table_model_->index(0, 0);
		QModelIndex bottomRight = file_info_table_model_->index(file_info_table_model_->rowCount() - 1, file_info_table_model_->columnCount() - 1);
		QItemSelection selection(topLeft, bottomRight);
		selectionModel->select(selection, QItemSelectionModel::Select);
		//selectionModel->clearSelection();
	});

	connect(btn_select_all2_, &QPushButton::clicked, this, [=]() {
		menu_index_valid_->hide();
		menu_index_novalid_->hide();
		QItemSelectionModel* selectionModel = file_table_view_->selectionModel();
		QModelIndex topLeft = file_info_table_model_->index(0, 0);
		QModelIndex bottomRight = file_info_table_model_->index(file_info_table_model_->rowCount() - 1, file_info_table_model_->columnCount() - 1);
		QItemSelection selection(topLeft, bottomRight);
		selectionModel->select(selection, QItemSelectionModel::Select);
	});

	connect(file_table_view_, &FileTableView::SigContextClicked, this, [=](QModelIndex index, QPoint pos) {
		OnContextMenuEvent(index, pos);
	});

	connect(file_table_view_, &FileTableView::SigMousePressed, this, [=]() {
		//ExitPersistentEditor();
		emit SigTableViewMousePressed();
	});

	connect(file_table_view_, &FileTableView::clicked, this, [=](const QModelIndex& index) {
		//std::cout << "FileTableView::clicked index " << index.row() << " " << index.column() << std::endl;
		ExitPersistentEditor();
	});

	if (FileAffiliationType::kRemote == affiliation_type_) {
		connect(dynamic_cast<RemoteFileUtil*>(file_util_.get()), &RemoteFileUtil::SigRemoteFilePermissionPath, this, [=]() {
			if ("/" == g_remote_file_permission_path) { // 说明拥有最高权限
				return;
			}
			QString current_dir_path = current_dir_.absolutePath();
			if (!current_dir_path.isEmpty()) {
				if ("/" != current_dir_path) {
					if (current_dir_path.endsWith('/')) {
						current_dir_path.chop(1);
					}
				}
			}
			if ("/" == current_dir_path) {
				current_dir_ = g_remote_file_permission_path;
				NotifyCurrentDirPathChange();
				return;
			}
			//判断当前路径是否是允许范围内的
			if (current_dir_path == g_remote_file_permission_path) {
				return;
			}
			if (g_remote_file_permission_path.length() > current_dir_path.length()) {
				current_dir_ = g_remote_file_permission_path;
				NotifyCurrentDirPathChange();
				return;
			}
			if (current_dir_path.startsWith(g_remote_file_permission_path)) {
				return;
			}
			current_dir_ = g_remote_file_permission_path;
			NotifyCurrentDirPathChange();
			});
	}
#if 0  // QItemSelectionModel::currentChanged 效果与FileTableView::clicked 类似，暂时关闭这段代码
	connect(file_table_view_->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex& current, const QModelIndex& previous) {
		std::cout << "current row = " << current.row() << " column = " << current.column() << std::endl;
		std::cout << "currentChanged previous row = " << previous.row() << " column = " << previous.column() << std::endl;
		//file_table_view_->closePersistentEditor(previous);
		//if (last_close_edit_index_ != previous) { // 不能连续 对同一个 QModelIndex closePersistentEditor 不然会崩溃
		//	last_close_edit_index_ = previous;
		//	file_table_view_->closePersistentEditor(previous);
		//}
		});
#endif
#if 0 //有了 Dele
	connect(file_table_view_, &FileTableView::activated, this, [=](const QModelIndex& index) {
		// 关闭持久编辑器
		std::cout << "FileTableView activated " << std::endl;
		/*file_table_view_->closePersistentEditor(index);*/
		//if (last_close_edit_index_ != index) { // 不能连续 对同一个 QModelIndex closePersistentEditor 不然会崩溃
		//	last_close_edit_index_ = index;
		//	file_table_view_->closePersistentEditor(index);
		//}
		if (current_index_opened_) {
			std::cout << "0 FileTableView::activated closePersistentEditor context_menu_index_ " << context_menu_index_.row() << " " << context_menu_index_.column() << std::endl;
			current_index_opened_ = false;
			file_table_view_->closePersistentEditor(context_menu_index_);

		}
		});

	// 主动 closePersistentEditor 并没不会触发 closeEditor
	connect(item_delegate_, &FileInfoTableViewBtnDelegate::closeEditor, this, [=]() {
		std::cout << " FileInfoTableViewBtnDelegate::closeEditor " << std::endl;
		});
#endif 

	InitHeaderViewSigChannel();
}

void FileListWidget::OnContextMenuEvent(QModelIndex index, QPoint pos) {
	if (current_file_container_has_disk_) {
		return;
	}
	ExitPersistentEditor();
	context_menu_index_ = index;
	if (index.isValid()) {
		menu_index_valid_->exec(pos);
	}
	else {
		menu_index_novalid_->exec(pos);
	}
}

void FileListWidget::OnRowDoubleClicked(const QModelIndex& index) {
	auto row = index.row();
	auto record = current_file_container_.files_detail_info_[row];
	if (record.file_type_ == EFileType::kFile) {
		return;
	}
	last_dir_ = current_dir_;
	current_dir_ = QDir(record.file_path_);
	history_dirs_.emplace(last_dir_);
	NotifyCurrentDirPathChange();
	file_util_->GetFiles(record.file_path_);
}

void FileListWidget::GoBack() {
	if (history_dirs_.empty()) {
		return;
	}
	QDir dir = history_dirs_.top();
	history_dirs_.pop();
	if (dir.path() == tc::kRealRootPath) {
		current_dir_ = tc::kRealRootPath;
		NotifyCurrentDirPathChange();
		file_util_->GetThisPCFiles();
	}
	else {
		QString temp_path = dir.absolutePath();
		current_dir_ = dir;
		NotifyCurrentDirPathChange();
		file_util_->GetFiles(temp_path);
	}
}

void FileListWidget::Refresh() {
	ExitPersistentEditor();
	QString temp_path = current_dir_.absolutePath();
	if (tc::kRealRootPath == temp_path) {
		file_util_->GetThisPCFiles();
	}
	else {
		file_util_->GetFiles(temp_path);
	}
}

void FileListWidget::GoParentDir() {
	if (FileAffiliationType::kRemote == affiliation_type_) {
		if (!g_remote_file_permission_path.isEmpty()) {
			QString current_dir_path = current_dir_.absolutePath();
			if (!current_dir_path.isEmpty()) {
				if ("/" != current_dir_path) {
					if (current_dir_path.endsWith('/')) {
						current_dir_path.chop(1);
					}
				}
			}
			if (current_dir_path == g_remote_file_permission_path) {
				return;
			}
		}
	}
	if (current_dir_.isRoot()) {  // 'C:/' 与 '/'  都会返回true， 要区分下
		if (tc::kRealRootPath != current_dir_.path()) {
			GoIndex();
		}
	}
	else {
		last_dir_ = current_dir_;
		QFileInfo file_info{ current_dir_.absolutePath() };
		QString temp_path = file_info.dir().path();
		current_dir_ = temp_path;
		history_dirs_.emplace(last_dir_);
		NotifyCurrentDirPathChange();
		file_util_->GetFiles(temp_path);
	}
}

//返回首页
void FileListWidget::GoIndex() {
	last_dir_ = current_dir_;
	file_util_->GetThisPCFiles();
	current_dir_ = tc::kRealRootPath;
	history_dirs_.emplace(last_dir_);
	NotifyCurrentDirPathChange();
}

void FileListWidget::GoTargetPath(const QString& path) {
	if (path != current_dir_.absolutePath()) {
		last_dir_ = current_dir_;
		current_dir_ = path;
		history_dirs_.emplace(last_dir_);
		NotifyCurrentDirPathChange();
	}
	file_util_->GetFiles(path);
}

FileContainer FileListWidget::GetSelectFiles() {
	FileContainer file_container;
	file_container.path_ = current_dir_.absolutePath();
	if (0 == current_file_container_.Size()) {
		return file_container;
	}
	std::set<int> row_set;
	auto rows = file_table_view_->selectionModel()->selectedRows();
	for (const QModelIndex& index : rows) {
		row_set.insert(index.row());
	}
	for (auto row : row_set) {
		auto res_and_file_info = current_file_container_[row];
		if (!res_and_file_info.first) {
			continue;
		}
		auto& file_info = res_and_file_info.second;
		file_container.AddFileInfo(file_info);
	}
	return file_container;
}

QDir FileListWidget::GetCurrentDir() {
	return current_dir_;
}

void FileListWidget::NotifyCurrentDirPathChange() {
	emit SigUpdateCurrentDirPath(current_dir_.absolutePath());
}

void FileListWidget::ReName(const QString& abs_path, const QString& name) {
	file_util_->ReName(abs_path, name);
}

void FileListWidget::ExitPersistentEditor() {
	if (current_index_opened_) { //进入编辑模式后  indexWidget 返回true ， 不能连续 对同一个 QModelIndex closePersistentEditor 不然会崩溃
		current_index_opened_ = false;
		//std::cout << "FileTableView::ExitPersistentEditor closePersistentEditor context_menu_index_ " << context_menu_index_.row() << " " << context_menu_index_.column() << std::endl;
		auto row = context_menu_index_.row();
		auto record = current_file_container_.files_detail_info_[row];
		if (item_delegate_->editor_) {
			item_delegate_->editor_->releaseKeyboard();
			//std::cout << "cellText = " << item_delegate_->editor_->text().toStdString() << std::endl;
			QString new_file_name = item_delegate_->editor_->text();
			if (!new_file_name.isEmpty() && record.file_name_ != new_file_name) {
				file_util_->ReName(record.file_path_, new_file_name);
			}
		}
		file_table_view_->closePersistentEditor(context_menu_index_);
		item_delegate_->editor_ = nullptr;
	}
}

void FileListWidget::HandleNewFolder() {
	if (create_new_folder_res_ && !create_new_folder_path_.isEmpty()) {
		QDir new_dir{ create_new_folder_path_ };
		QString new_folder_name = new_dir.dirName();
		QString cur_path = current_dir_.absolutePath();
		QFileInfo file_info{ create_new_folder_path_ };
		QString new_folder_parent_path = file_info.dir().path();
		if (new_folder_parent_path == cur_path) {
			int rowCount = file_info_table_model_->rowCount();
			int columnCount = file_info_table_model_->columnCount();
			for (int row = 0; row < rowCount; ++row)
			{
				QModelIndex index = file_info_table_model_->index(row, 0); // 第一列的索引为 (row, 0)
				if (index.isValid() && index.column() == 0) // 确保索引有效且在第一列
				{
					QVariant value = file_info_table_model_->data(index);
					//std::cout << "Value at row" << row << ": " << value.toString().toStdString() << std::endl;
					if (value.toString() == new_folder_name) {
						// to do 先退出直接的编辑模式
						ExitPersistentEditor();
						context_menu_index_ = index;
						file_table_view_->setFocus();
						file_table_view_->openPersistentEditor(context_menu_index_);
						file_table_view_->edit(context_menu_index_);   // 使光标闪烁
						file_table_view_->viewport()->update();
						if (item_delegate_->editor_) {
							file_table_view_->setFocusProxy(item_delegate_->editor_);
						}
						current_index_opened_ = true;
						break;
					}
				}
			}
		}
		create_new_folder_res_ = false;
		create_new_folder_path_ = "";
	}
}

void FileListWidget::SearchByFileName(const QString& file_name) {
	if (file_name.isEmpty()) {
		Refresh();
		return;
	}
	QString pattern = ".*" + file_name + ".*";
	FileContainer pattern_file_container;
	QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
	for (auto& file_info : current_file_container_bak_.files_detail_info_) {
		if (regex.match(file_info.file_name_).hasMatch()) {
			pattern_file_container.AddFileInfo(file_info);
		}
	}
	current_file_container_ = pattern_file_container;
	file_info_table_model_->Updatefilecontainer(current_file_container_);
}

void FileListWidget::InitHeaderViewSigChannel() {
	if (!file_table_view_) {
		return;
	}
	QHeaderView* header = file_table_view_->horizontalHeader();
	if (!header) {
		return;
	}
	QObject::connect(header, &QHeaderView::sectionClicked, [=](int logical_index) {
		auto header_item_type = static_cast<EFileTableHeaderViewItemType>(logical_index);
		Sort(header_item_type);
	});
}

void FileListWidget::Sort(EFileTableHeaderViewItemType item_type) {
	if (!header_item_type_map_sort_.count(item_type)) {
		return;
	}
	current_file_container_.Sort(item_type, header_item_type_map_sort_[item_type]);
	FileLogManager::Instance()->AppendLog("QHeaderView::sectionClicked");
	current_header_item_type_sort_.first = item_type;
	current_header_item_type_sort_.second = header_item_type_map_sort_[item_type];
	header_item_type_map_sort_[item_type] = !header_item_type_map_sort_[item_type];
	file_info_table_model_->Updatefilecontainer(current_file_container_); // to do 再检查下这里
	update();
}

}
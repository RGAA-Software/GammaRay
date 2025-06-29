#include "file_overview_widget.h"
#include <qlistview.h>
#include <qaction.h>
#include <qpalette.h>
#include <qfileiconprovider.h>
#include <qscrollbar.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include "file_send_btn.h"
#include "file_operation_btn.h"
#include "file_list_widget.h"
#include "local_file_util.h"
#include "remote_file_util.h"
#include "file_table_view_style.h"
#include "file_transmit_single_task_manager.h"
#include "file_log_manager.h"
#include "tc_label.h"

namespace tc {

//margin padding 的顺序是上右下左
static QString s_combobox_style = "QComboBox {border:0px;border-radius: 4px; padding: 0px 0px 0px 10px; font-family:Microsoft YaHei;font-size:%1px; color:#333333;background-color:#ffffff;}"
	"QComboBox::drop-down { padding-right: %6px;  padding-top: %6px; width:%2px; height:%2px; border:none;}"
	"QComboBox::down-arrow {image: url(:/resource/arrow.svg); width:%2px; height:%2px;}"
	"QComboBox QAbstractItemView {background-color:#ffffff; color:#333333; border:0px; border-radius:8px;font-size:%3px;outline: 0px; margin-top: 6px;}"
	"QComboBox QAbstractItemView::item {border-radius:4px; background-color:#ffffff; color:#333333; font-size:%4px;height:%5px;font-family:Microsoft YaHei; padding-left:10px;}"
	"QComboBox QAbstractItemView::item::hover {border-radius:4px;background-color:#F5F5F5;color:#333333;}"
	"QComboBox QAbstractItemView::item::selected {border-radius:4px;background-color:#F5F5F5;color:#333333;}";

static QString s_search_line_edit_style = R"(
QLineEdit {border: 0px;border-radius: 4px; padding: 8px; font-family:Microsoft YaHei; color:#191919; background-color:#fefefe;}
QLineEdit:focus{color:#000000; background-color:#ffffff;}
)";

static QString s_account_tag_style = R"(
QLabel {font-size: %1px; font-family: Microsoft YaHei; font-weight: bold; color: #191919;line-height: %2px;}
)";

static QString s_aff_type_style = R"(
QLabel {font-size: %1px; font-family: Microsoft YaHei; color: #333333;line-height: %2px;}
)";

AccountInfoWidget::AccountInfoWidget(FileAffiliationType aff_type, QWidget* parent) : aff_type_(aff_type), QWidget(parent) {
	setAttribute(Qt::WA_StyledBackground);
	setStyleSheet("QWidget {background-color:#F5F5F5;}");
	setFixedHeight(42);
	s_account_tag_style = s_account_tag_style.arg(16).arg(24);
	s_aff_type_style = s_aff_type_style.arg(12).arg(18);

	main_hbox_layout_ = new QHBoxLayout(this);
	main_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	main_hbox_layout_->setSpacing(0);
	main_hbox_layout_->setAlignment(Qt::AlignLeft);

	profile_hbox_layout_ = new QHBoxLayout();
	profile_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	profile_hbox_layout_->setSpacing(0);
	profile_hbox_layout_->setAlignment(Qt::AlignLeft);

	btn_profile_picture_ = new QPushButton(this);
	btn_profile_picture_->setFixedSize(42, 42);

	// to do 这里以后应该换成实际的用户头像
	/*btn_profile_picture_->SetPixmap1(":/resource/profile_picture.svg", 42, 42);*/
	QIcon icon;
	icon.addFile(":/resource/profile_picture.svg");
	btn_profile_picture_->setIcon(icon);
	btn_profile_picture_->setIconSize(QSize(42,42));

	profile_hbox_layout_->addSpacing(6);
	profile_hbox_layout_->addWidget(btn_profile_picture_);
	main_hbox_layout_->addLayout(profile_hbox_layout_);

	account_name_vbox_layout_ = new QVBoxLayout();
	account_name_vbox_layout_->setAlignment(Qt::AlignLeft);
	lab_name_ = new QLabel(this);

	lab_name_->setStyleSheet(s_account_tag_style);
	lab_pos_ = new QLabel(this);
	lab_pos_->setStyleSheet(s_aff_type_style);
	
	switch (aff_type_)
	{
	case FileAffiliationType::kLocal:
		//lab_pos_->setText(QStringLiteral("本地设备"));
		lab_pos_->setText(tcTr("id_file_trans_local_device"));
		break;
	case FileAffiliationType::kRemote:
		//lab_pos_->setText(QStringLiteral("远程设备"));
		lab_pos_->setText(tcTr("id_file_trans_remote_device"));
		break;
	default:
		break;
	}
	
	account_name_vbox_layout_->addWidget(lab_name_);
	account_name_vbox_layout_->addWidget(lab_pos_);

	main_hbox_layout_->addSpacing(8);
	main_hbox_layout_->addLayout(account_name_vbox_layout_);
}

void AccountInfoWidget::SetDeviceId(const QString& device_id) {
	lab_name_->setText(device_id);
}

AccountInfoWidget::~AccountInfoWidget() {

}

FileOperationWidget::FileOperationWidget(FileAffiliationType type, QWidget* parent) : affiliation_type_(type), QWidget(parent) {
	Init();
}

FileOperationWidget::~FileOperationWidget() {

}

void FileOperationWidget::Init() {
	setAttribute(Qt::WA_StyledBackground);
	main_hbox_layout_ = new QHBoxLayout();
	main_hbox_layout_->setContentsMargins(0,0,0,0);
	main_hbox_layout_->setSpacing(0);
	setLayout(main_hbox_layout_);

	btn_trans_ = new FileSendBtn(affiliation_type_);

	btn_go_back_ = new FileOperationBtn(":/resource/go_back_normal.svg", ":/resource/go_back_hover.svg",
		":/resource/go_back_press.svg", ":/resource/go_back_disable.svg", FileOperationType::kGoBack, this);
	//btn_go_back_->setToolTip(QStringLiteral("返回"));
	btn_go_back_->setToolTip(tcTr("id_file_trans_go_back"));

	btn_go_parent_directory_ = new FileOperationBtn(":/resource/parent_dir_normal.svg", ":/resource/parent_dir_hover.svg",
		":/resource/parent_dir_press.svg", ":/resource/parent_dir_disable.svg", FileOperationType::kGoParentDir, this);
	//btn_go_parent_directory_->setToolTip(QStringLiteral("上级目录"));
	btn_go_parent_directory_->setToolTip(tcTr("id_file_trans_parent_directory"));

	btn_go_index_ = new FileOperationBtn(":/resource/go_index_normal.svg", ":/resource/go_index_hover.svg",
		":/resource/go_index_press.svg", ":/resource/go_index_disable.svg", FileOperationType::kGoIndex, this);
	//btn_go_index_->setToolTip(QStringLiteral("首页"));
	btn_go_index_->setToolTip(tcTr("id_file_trans_index"));

	btn_go_refresh_ = new FileOperationBtn(":/resource/refresh_normal.svg", ":/resource/refresh_hover.svg",
		":/resource/refresh_press.svg", ":/resource/refresh_disable.svg", FileOperationType::kRefresh, this);
	//btn_go_refresh_->setToolTip(QStringLiteral("刷新"));
	btn_go_refresh_->setToolTip(tcTr("id_file_trans_refresh"));

	btn_go_back_->setEnabled(false);
	btn_go_parent_directory_->setEnabled(false);

	search_line_edit_ = new QLineEdit(this);
	search_line_edit_->setFixedHeight(30);
	search_line_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	search_line_edit_->setStyleSheet(s_search_line_edit_style);
	//search_line_edit_->setPlaceholderText(QStringLiteral("搜索"));
	search_line_edit_->setToolTip(tcTr("id_file_trans_search"));

	QAction* searchAction = new QAction(search_line_edit_);
	searchAction->setIcon(QIcon(":/resource/search.svg"));
	search_line_edit_->addAction(searchAction, QLineEdit::LeadingPosition);//表示action所在方位（左侧）。

	if (FileAffiliationType::kLocal == affiliation_type_) {
		AddOperationButton();
		main_hbox_layout_->addSpacing(6);
		main_hbox_layout_->addWidget(btn_trans_);
	}
	else if (FileAffiliationType::kRemote == affiliation_type_) {
		main_hbox_layout_->addWidget(btn_trans_);
		main_hbox_layout_->addSpacing(6);
		AddOperationButton();
	}
	InitSigChannel();
}

void FileOperationWidget::AddOperationButton() {
	main_hbox_layout_->addWidget(btn_go_back_);
	main_hbox_layout_->addSpacing(6);
	main_hbox_layout_->addWidget(btn_go_parent_directory_);
	main_hbox_layout_->addSpacing(6);
	main_hbox_layout_->addWidget(btn_go_index_);
	main_hbox_layout_->addSpacing(6);
	main_hbox_layout_->addWidget(btn_go_refresh_);
	main_hbox_layout_->addSpacing(6);
	main_hbox_layout_->addWidget(search_line_edit_);
}

void FileOperationWidget::InitSigChannel() {
	connect(btn_go_back_, &QPushButton::clicked, this, [=]() {
		emit SigGoBack();
		});

	connect(btn_go_parent_directory_, &QPushButton::clicked, this, [=]() {
		emit SigParentDirectory();
		});

	connect(btn_go_index_, &QPushButton::clicked, this, [=]() {
		emit SigGoIndex();
		});

	connect(btn_go_refresh_, &QPushButton::clicked, this, [=]() {
		emit SigGoRefresh();
		});

	connect(btn_trans_, &QPushButton::clicked, this, [=]() {
		emit SigSend();
		});

	connect(search_line_edit_, &QLineEdit::returnPressed, this, [=]() {
		emit SigSearchFileByName(search_line_edit_->text());
		});
}

FileOverviewWidget::FileOverviewWidget(FileAffiliationType type, QWidget* parent, QString path) : affiliation_type_(type), QWidget(parent) {
	Init(path);
}

FileOverviewWidget::~FileOverviewWidget()
{
}

void FileOverviewWidget::SetDeviceId(const QString& device_id) {
	account_widget_->SetDeviceId(device_id);
}

void FileOverviewWidget::Init(QString path) {
	main_vbox_layout_ = new QVBoxLayout();
	// 左 上 右 下
	main_vbox_layout_->setContentsMargins(0, 0, 0, 6);
	main_vbox_layout_->setSpacing(0);
	main_vbox_layout_->setAlignment(Qt::AlignTop);
	setLayout(main_vbox_layout_);
	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet(QString("QWidget {background-color: #F5F5F5; border-radius: %1px;}").arg(6));

	//账号信息
	account_hbox_layout_ = new QHBoxLayout();
	account_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	account_hbox_layout_->setAlignment(Qt::AlignLeft);

	account_widget_ = new AccountInfoWidget(affiliation_type_, this);
	account_hbox_layout_->addWidget(account_widget_);
	main_vbox_layout_->addSpacing(6);
	main_vbox_layout_->addLayout(account_hbox_layout_);

	// 操作按钮，返回、上级目录等 
	operation_hbox_layout_ = new QHBoxLayout();
	operation_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	operation_hbox_layout_->setAlignment(Qt::AlignLeft);
	operation_widget_ = new FileOperationWidget(affiliation_type_, this);
	operation_hbox_layout_->addSpacing(6);
	operation_hbox_layout_->addWidget(operation_widget_);
	operation_hbox_layout_->addSpacing(6);
	main_vbox_layout_->addSpacing(6);
	main_vbox_layout_->addLayout(operation_hbox_layout_);

	// 搜索框 
	search_hbox_layout_ = new QHBoxLayout();
	search_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	search_hbox_layout_->setAlignment(Qt::AlignLeft);


	//可选项为此电脑下的磁盘与文件夹
	combobox_select_dir_ = new QComboBox();
	combobox_select_dir_->setFixedHeight(30);
	combobox_select_dir_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	//combobox_select_dir_->setEditable(false);
	combobox_select_dir_->setEditable(true);// 可以进行编辑文本框

	static std::once_flag flag;
	std::call_once(flag, []() {
		s_combobox_style = s_combobox_style
			.arg(12).arg(8).arg(12).arg(12).arg(30).arg(9);
		});

	combobox_select_dir_->setStyleSheet(s_combobox_style);
	combobox_select_dir_->setFixedHeight(30);
	combobox_select_dir_->setView(new QListView());
	combobox_select_dir_->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	combobox_select_dir_->view()->verticalScrollBar()->setStyleSheet(QString::fromStdString(kScrollBarStyle));


	search_hbox_layout_->addSpacing(6);
	search_hbox_layout_->addWidget(combobox_select_dir_);
	search_hbox_layout_->addSpacing(6);
	main_vbox_layout_->addSpacing(6);
	main_vbox_layout_->addLayout(search_hbox_layout_);

	// 文件列表 
	list_view_hbox_layout_ = new QHBoxLayout();
	list_view_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	list_view_hbox_layout_->setSpacing(0);
	list_view_hbox_layout_->setAlignment(Qt::AlignLeft);
	file_list_widget_ = new FileListWidget(affiliation_type_, this, path);

	list_view_hbox_layout_->addSpacing(6);
	list_view_hbox_layout_->addWidget(file_list_widget_);
	list_view_hbox_layout_->addSpacing(6);
	main_vbox_layout_->addSpacing(6);
	main_vbox_layout_->addLayout(list_view_hbox_layout_);

	if (path != tc::kRealRootPath && !path.isEmpty()) {
		operation_widget_->btn_go_parent_directory_->setEnabled(true);
	}


	if (FileAffiliationType::kLocal == affiliation_type_) {
		file_util_ = std::make_shared<LocalFileUtil>();
	}
	else if (FileAffiliationType::kRemote == affiliation_type_) {
		file_util_ = std::make_shared<RemoteFileUtil>();
	}

	InitSigChannel();

	//用于展示在下拉框
	file_util_->GetThisPCFiles();

	combobox_select_dir_->setEditText(path);

	current_path_ = path;
}

void FileOverviewWidget::InitSigChannel() {
	connect(operation_widget_, &FileOperationWidget::SigGoBack, [=]() {
		file_list_widget_->GoBack();
		});

	connect(operation_widget_, &FileOperationWidget::SigGoIndex, [=]() {
		file_list_widget_->GoIndex();
		});

	connect(operation_widget_, &FileOperationWidget::SigParentDirectory, [=]() {
		file_list_widget_->GoParentDir();
		});

	connect(operation_widget_, &FileOperationWidget::SigGoRefresh, [=]() {
		file_list_widget_->Refresh();
		});

	connect(operation_widget_, &FileOperationWidget::SigGoTargetDir, this, [=](const QString& path) {
		file_list_widget_->GoTargetPath(path);
		});

	connect(operation_widget_, &FileOperationWidget::SigSearchFileByName, this, [=](const QString& name) {
		file_list_widget_->SearchByFileName(name);
		});

	connect(operation_widget_, &FileOperationWidget::SigSend, this, [=]() {
		auto select_file_container = file_list_widget_->GetSelectFiles();
		emit SigTransmitFileContainer(select_file_container);
		});

	connect(file_list_widget_, &FileListWidget::SigUpdateCurrentDirPath, this, [=](QString path) {
		current_path_ = path;
		combobox_select_dir_->setEditText(path);

		emit SigUpdateCurrentDirPath(path);

		if (file_list_widget_->HasHistory()) {
			operation_widget_->btn_go_back_->setEnabled(true);
		}
		else {
			operation_widget_->btn_go_back_->setEnabled(false);
		}

		if (file_list_widget_->IsRootPath()) {
			operation_widget_->btn_go_parent_directory_->setEnabled(false);
		}
		else {
			operation_widget_->btn_go_parent_directory_->setEnabled(true);
		}
		});

	connect(file_list_widget_, &FileListWidget::SigCurrentFileContainterHasDisk, this, [=](bool has) {
		emit SigCurrentFileContainterHasDisk(has);
		});



	connect(FileTransmitSingleTaskManager::Instance(), &FileTransmitSingleTaskManager::SigTransmitSuccess, this, [=](EFileTransmitTaskType type) {
		switch (type)
		{
		case EFileTransmitTaskType::kUpload:
			if (FileAffiliationType::kRemote == affiliation_type_) {
				file_list_widget_->Refresh();
			}
			break;
		case EFileTransmitTaskType::kDownload:
			if (FileAffiliationType::kLocal == affiliation_type_) {
				file_list_widget_->Refresh();
			}
			break;
		}
		});

	connect(file_list_widget_, &FileListWidget::SigTableViewMousePressed, this, [=]() {
		emit SigTableViewMousePressed();
		});

	connect(file_util_.get(), &BaseFileUtil::SigGetFiles, this, [=](FileContainer file_container) {
		combobox_select_dir_->clear();
		for (auto file_info : file_container.files_detail_info_) {
			QIcon icon;
			if (EFileType::kFolder == file_info.file_type_) {
				icon.addFile(":/resource/folder.svg");
			}
			else if (EFileType::kDesktopFolder == file_info.file_type_) {
				icon.addFile(":/resource/desktop.svg");
			}
			else if (EFileType::kFile == file_info.file_type_) { // 文件类型不要显示在下拉框
				continue;
			}
			else {
				QFileInfo fileInfo(file_info.file_name_);
				QFileIconProvider iconProvider;
				icon = iconProvider.icon(fileInfo);
			}
			combobox_select_dir_->addItem(icon, file_info.file_name_);
			combobox_file_container_.AddFileInfo(file_info);
		}
		combobox_select_dir_->setEditText(current_path_);
		});


	connect(combobox_select_dir_, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=](int index) {
		auto res = combobox_file_container_[index];
		if (!res.first) {
			return;
		}
		file_list_widget_->GoTargetPath(res.second.file_path_);
	});

	// connect(combobox_select_dir_, &QComboBox::editTextChanged, this, [=](const QString& path) { file_list_widget_->GoTargetPath(path); }); 会实时触发
	connect(combobox_select_dir_, &QComboBox::textActivated, this, [=](const QString& path) {  //回车触发
		file_list_widget_->GoTargetPath(path);
	});
}

void FileOverviewWidget::RefreshByComboboxSelectDirContent() {
	QString cur_dir = combobox_select_dir_->currentText();
	if (cur_dir.isEmpty()) {
		return;
	}
	file_list_widget_->GoTargetPath(cur_dir);
}

QDir FileOverviewWidget::GetCurrentDir() {
	return file_list_widget_->GetCurrentDir();
}

bool FileOverviewWidget::HasDisk() {
	if (!file_list_widget_) {
		return false;
	}
	return file_list_widget_->HasDisk();
}

void FileOverviewWidget::SetSendBtnEnabled(bool enabled) {
	if (!operation_widget_) {
		return;
	}
	operation_widget_->btn_trans_->setEnabled(enabled);
}

void FileOverviewWidget::ExitPersistentEditor() {
	file_list_widget_->ExitPersistentEditor();
}

}
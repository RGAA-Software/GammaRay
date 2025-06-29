#pragma once
#include <qdir.h>
#include <qwidget.h>
#include "file_const_def.h"
#include "file_detail_info.h"
class QAction;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;


namespace tc {
class BaseFileUtil;
class FileSendBtn;
class FileOperationBtn;
class FileListWidget;


class AccountInfoWidget : public QWidget {
	Q_OBJECT
public:
	AccountInfoWidget(FileAffiliationType aff_type, QWidget* parent = nullptr);
	~AccountInfoWidget();
	void SetDeviceId(const QString& device_id);
private:
	QHBoxLayout* main_hbox_layout_ = nullptr;
	QHBoxLayout* profile_hbox_layout_ = nullptr;
	QPushButton* btn_profile_picture_ = nullptr;

	QVBoxLayout* account_name_vbox_layout_ = nullptr;
	QLabel* lab_name_ = nullptr;
	QLabel* lab_pos_ = nullptr;

	FileAffiliationType aff_type_ = FileAffiliationType::kLocal;
};


// 菜单栏 文件操作：比如 上级、返回、刷新等
class FileOperationWidget : public QWidget {
	Q_OBJECT
public:
	FileOperationWidget(FileAffiliationType type, QWidget* parent = nullptr);
	~FileOperationWidget();
	FileAffiliationType affiliation_type_ = FileAffiliationType::kLocal;
	void Init();
	FileSendBtn* btn_trans_ = nullptr;
	FileOperationBtn* btn_go_back_ = nullptr;
	FileOperationBtn* btn_go_parent_directory_ = nullptr;
private:
	QHBoxLayout* main_hbox_layout_ = nullptr;
	FileOperationBtn* btn_go_index_ = nullptr;
	FileOperationBtn* btn_go_refresh_ = nullptr;

	QLineEdit* search_line_edit_ = nullptr;

	void AddOperationButton();
	void InitSigChannel();
signals:
	void SigGoBack();
	void SigParentDirectory();
	void SigGoIndex();
	void SigGoRefresh();
	void SigGoTargetDir(const QString& path);

	void SigSend();
	void SigSearchFileByName(const QString& name);
};

// 用来显示文件操作、列表
class FileOverviewWidget : public QWidget {
	Q_OBJECT
public:
	FileOverviewWidget(FileAffiliationType type, QWidget* parent = nullptr, QString path = "");
	~FileOverviewWidget();
	void Init(QString path = "");
	QDir GetCurrentDir();
	bool HasDisk();
	void SetSendBtnEnabled(bool enabled);
	void ExitPersistentEditor();
	void RefreshByComboboxSelectDirContent();
	void SetDeviceId(const QString& device_id);
signals:
	void SigTransmitFileContainer(FileContainer);
	void SigUpdateCurrentDirPath(QString);
	void SigCurrentFileContainterHasDisk(bool);
	void SigTableViewMousePressed();
private:

	QVBoxLayout* main_vbox_layout_ = nullptr;

	//账号信息
	QHBoxLayout* account_hbox_layout_ = nullptr;
	AccountInfoWidget* account_widget_ = nullptr;

	// 操作按钮 
	QHBoxLayout* operation_hbox_layout_ = nullptr;
	FileOperationWidget* operation_widget_ = nullptr;

	// 搜索框
	QHBoxLayout* search_hbox_layout_ = nullptr;
	QComboBox* combobox_select_dir_ = nullptr;
	FileContainer combobox_file_container_;

	// 文件列表
	QHBoxLayout* list_view_hbox_layout_ = nullptr;
	FileListWidget* file_list_widget_ = nullptr;

	std::shared_ptr<BaseFileUtil> file_util_ = nullptr;

	FileAffiliationType affiliation_type_ = FileAffiliationType::kLocal;

	void InitSigChannel();

	QString current_path_ = "";
};
}
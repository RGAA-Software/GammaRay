#include "file_show_widget.h"
#include <qdir.h>
#include <iostream>
#include <qboxlayout.h>
#include "file_overview_widget.h"
#include "file_transmit_task.h"
#include "file_transmit_task_manager.h"


namespace tc {

int FileShowWidget::file_transfer_direction_control_ = 3;

FileShowWidget::FileShowWidget(QWidget* parent, QString local_path, QString remote_path) : QWidget(parent) {
	Init(local_path, remote_path);
}

FileShowWidget::~FileShowWidget() {
}

void FileShowWidget::Init(QString local_path, QString remote_path) {
	main_hbox_layout_ = new QHBoxLayout();
	main_hbox_layout_->setContentsMargins(0,0,0,0);
	main_hbox_layout_->setSpacing(0);
	setLayout(main_hbox_layout_);
	local_file_overview_widget_ = new FileOverviewWidget(FileAffiliationType::kLocal, this, local_path);
	main_hbox_layout_->addWidget(local_file_overview_widget_);
	remote_file_overview_widget_ = new FileOverviewWidget(FileAffiliationType::kRemote, this, remote_path);
	main_hbox_layout_->addSpacing(6);
	main_hbox_layout_->addWidget(remote_file_overview_widget_);
	InitSigChannel();
}

void FileShowWidget::SetDevicesId(const QString& local_device_id, const QString& remove_device_id) {
	local_file_overview_widget_->SetDeviceId(local_device_id);
    remote_file_overview_widget_->SetDeviceId(remove_device_id);
}

void FileShowWidget::InitSigChannel() {
	//上传
	connect(local_file_overview_widget_, &FileOverviewWidget::SigTransmitFileContainer, this, [=](FileContainer file_container)
		{
			// to do 权限控制
			/*if (!(Tc_FILE_TRANSMIT_DIRECTION_CONTROL_SITE_TO_CONTROLLED & file_transfer_direction_control_)) {
				emit SigNoFileTransmitPermission(TcCA_FILE_TRANSMIT_DIRECTION_CONTROL_SITE_TO_CONTROLLED);
				return;
			}*/
			std::cout << "FileShowWidget upload " << std::endl;
			auto remote_dir = remote_file_overview_widget_->GetCurrentDir();
			auto local_dir = local_file_overview_widget_->GetCurrentDir();
			// windows系统中 有磁盘那一页，不允许文件传输了
			if (local_file_overview_widget_->HasDisk() || remote_file_overview_widget_->HasDisk()) {
				std::cout << "FileShowWidget  HasDisk" << std::endl;
				return;
			}
			auto task_ptr = FileTransmitTask::CreateFileTransmitTask(std::move(file_container));
			task_ptr->target_dir_path_ = remote_dir.absolutePath();
			task_ptr->current_dir_path_ = local_dir.absolutePath();
			task_ptr->task_type_ = EFileTransmitTaskType::kUpload;
			std::cout << "current_dir_path_ = " << task_ptr->current_dir_path_.toStdString() << std::endl;
			std::cout << "target_dir_path_ = " << task_ptr->target_dir_path_.toStdString() << std::endl;
			FileTransmitTaskManager::Instance()->AddFileTransmitTask(task_ptr);
		});

	//下载
	connect(remote_file_overview_widget_, &FileOverviewWidget::SigTransmitFileContainer, this, [=](FileContainer file_container)
		{
			// to do 权限控制
			/*if (!(Tc_FILE_TRANSMIT_DIRECTION_CONTROLLED_SITE_TO_CONTROL & file_transfer_direction_control_)) {
				emit SigNoFileTransmitPermission(TcCA_FILE_TRANSMIT_DIRECTION_CONTROLLED_SITE_TO_CONTROL);
				return;
			}*/
			std::cout << "FileShowWidget download " << std::endl;
			auto remote_dir = remote_file_overview_widget_->GetCurrentDir();
			auto local_dir = local_file_overview_widget_->GetCurrentDir();
			// windows系统中 有磁盘那一页，不允许文件传输了
			if (local_file_overview_widget_->HasDisk() || remote_file_overview_widget_->HasDisk()) {
				std::cout << "FileShowWidget  HasDisk" << std::endl;
				return;
			}
			auto task_ptr = FileTransmitTask::CreateFileTransmitTask(std::move(file_container));
			task_ptr->current_dir_path_ = remote_dir.absolutePath();
			task_ptr->target_dir_path_ = local_dir.absolutePath();
			task_ptr->task_type_ = EFileTransmitTaskType::kDownload;
			std::cout << "current_dir_path_ = " << task_ptr->current_dir_path_.toStdString() << std::endl;
			std::cout << "target_dir_path_ = " << task_ptr->target_dir_path_.toStdString() << std::endl;
			FileTransmitTaskManager::Instance()->AddFileTransmitTask(task_ptr);
		});

	connect(local_file_overview_widget_, &FileOverviewWidget::SigUpdateCurrentDirPath, this, [=](QString path) {
		emit SigLocalCurrentDirChangePath(path);
		});
	connect(remote_file_overview_widget_, &FileOverviewWidget::SigUpdateCurrentDirPath, this, [=](QString path) {
		emit SigRemoteCurrentDirChangePath(path);
		});

	connect(local_file_overview_widget_, &FileOverviewWidget::SigCurrentFileContainterHasDisk, this, [=](bool has) {
		local_file_list_has_disk_ = has;
		SetSendBtnState();
		});
	connect(remote_file_overview_widget_, &FileOverviewWidget::SigCurrentFileContainterHasDisk, this, [=](bool has) {
		remote_file_list_has_disk_ = has;
		SetSendBtnState();
		});

	// 当鼠标点击在表格的空白处，就退出单元格编辑模式
	connect(local_file_overview_widget_, &FileOverviewWidget::SigTableViewMousePressed, this, [=]() {
		local_file_overview_widget_->ExitPersistentEditor();
		remote_file_overview_widget_->ExitPersistentEditor();
		});

	connect(remote_file_overview_widget_, &FileOverviewWidget::SigTableViewMousePressed, this, [=]() {
		local_file_overview_widget_->ExitPersistentEditor();
		remote_file_overview_widget_->ExitPersistentEditor();
		});
}

void FileShowWidget::SetSendBtnState() {
	if (local_file_list_has_disk_ || remote_file_list_has_disk_) {
		local_file_overview_widget_->SetSendBtnEnabled(false);
		remote_file_overview_widget_->SetSendBtnEnabled(false);
	}
	else {
		local_file_overview_widget_->SetSendBtnEnabled(true);
		remote_file_overview_widget_->SetSendBtnEnabled(true);
	}
}

void FileShowWidget::RefreshByComboboxSelectDirContent() {
	local_file_overview_widget_->RefreshByComboboxSelectDirContent();
	remote_file_overview_widget_->RefreshByComboboxSelectDirContent();
}

}
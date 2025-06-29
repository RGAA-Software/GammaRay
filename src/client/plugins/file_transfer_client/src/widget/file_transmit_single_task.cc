#include "file_transmit_single_task.h"
#include <iostream>
#include <qfileinfo.h>
#include <qtimer.h>
#include "tc_common_new/time_util.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/log.h"
#include "file_transmit_single_task_manager.h"
#include "local_file_util.h"
#include "remote_file_util.h"
#include "file_popup_widgets.h"
#include "file_transmit_task_manager.h"
#include "file_transmit_task.h"
#include "core/file_sdk_interface.h"
#include "file_log_manager.h"

namespace tc {

FileTransmitSingleTask::FileTransmitSingleTask() {
	remote_file_util_ = std::make_shared<RemoteFileUtil>();
	local_file_util_ = std::make_shared<LocalFileUtil>();
	timer_ = new QTimer(this);
	timer_->setInterval(1000);
	InitSigChannel();
}

void FileTransmitSingleTask::InitSigChannel() {
	// 上传
	connect(remote_file_util_.get(), &BaseFileUtil::SigExists, this, [=](bool exists, QString file_size_str, QString file_date_str) {
		if (exists) {
			auto parent_task_ptr = FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_];
			if (parent_task_ptr->skip_) {
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
				return;
			}
			QFileInfo local_finfo{ current_file_path_ };
			uint64_t local_file_size = local_finfo.size();
			QString local_file_size_str = QString::fromStdString(tc::StringUtil::FormatSize(local_file_size));
			QString local_file_date_str = local_finfo.metadataChangeTime().toString("yyyy-MM-dd hh:mm:ss");
			FileCoverDialog file_cover_dialog{};
			file_cover_dialog.SetData(EFileTransmitTaskType::kDownload == task_type_, local_finfo.fileName(), local_file_size_str, local_file_date_str,
				file_size_str, file_date_str);
			if (FileSDKInterface::Instance()->file_trans_widget_) {
				auto pos =  FileSDKInterface::Instance()->file_trans_widget_->pos();
				int offset_x = (FileSDKInterface::Instance()->file_trans_widget_->width() - file_cover_dialog.width()) / 2;
				int offset_y = (FileSDKInterface::Instance()->file_trans_widget_->height() - file_cover_dialog.height()) / 2;
				file_cover_dialog.move(pos.x() + offset_x, pos.y() + offset_y);
			}

			file_cover_dialog.exec();

			if (FileCoverDialog::EOperationType::kAllCover == file_cover_dialog.operation_type_) {
				FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_]->cover_ = true;
				DoUpload();
			}
			else if (FileCoverDialog::EOperationType::kAllSkip == file_cover_dialog.operation_type_) {
				FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_]->skip_ = true;
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
			}
			else if (FileCoverDialog::EOperationType::kCancel == file_cover_dialog.operation_type_) {
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
			}
			else if (FileCoverDialog::EOperationType::kCover == file_cover_dialog.operation_type_) {
				DoUpload();
			}
			else if (FileCoverDialog::EOperationType::kSkip == file_cover_dialog.operation_type_) {
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
			}
		}
		else {
			DoUpload();
		}
		});
	// 下载
	connect(local_file_util_.get(), &BaseFileUtil::SigExists, this, [=](bool exists) {
		if (exists) {
			auto parent_task_ptr = FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_];
			if (parent_task_ptr->skip_) {
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
				return;
			}
			QFileInfo local_finfo{ target_file_path_ };
			uint64_t local_file_size = local_finfo.size();
			QString local_file_size_str = QString::fromStdString(tc::StringUtil::FormatSize(local_file_size));
			QString local_file_date_str = local_finfo.metadataChangeTime().toString("yyyy-MM-dd hh:mm:ss");
			FileCoverDialog file_cover_dialog{};
			file_cover_dialog.SetData(EFileTransmitTaskType::kDownload == task_type_, local_finfo.fileName(), local_file_size_str, local_file_date_str,
				remote_file_size_str_, remote_file_date_str_);
			if (FileSDKInterface::Instance()->file_trans_widget_) {
				auto pos = FileSDKInterface::Instance()->file_trans_widget_->pos();
				int offset_x = (FileSDKInterface::Instance()->file_trans_widget_->width() - file_cover_dialog.width()) / 2;
				int offset_y = (FileSDKInterface::Instance()->file_trans_widget_->height() - file_cover_dialog.height()) / 2;
				file_cover_dialog.move(pos.x() + offset_x, pos.y() + offset_y);
			}
			file_cover_dialog.exec();
			if (FileCoverDialog::EOperationType::kAllCover == file_cover_dialog.operation_type_) {
				FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_]->cover_ = true;
				DoDownload();
			}
			else if (FileCoverDialog::EOperationType::kAllSkip == file_cover_dialog.operation_type_) {
				FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_]->skip_ = true;
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
			}
			else if (FileCoverDialog::EOperationType::kCancel == file_cover_dialog.operation_type_) {
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
			}
			else if (FileCoverDialog::EOperationType::kCover == file_cover_dialog.operation_type_) {
				DoDownload();
			}
			else if (FileCoverDialog::EOperationType::kSkip == file_cover_dialog.operation_type_) {
				emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kSkip);
			}
		}
		else {
			DoDownload();
		}
	});

	connect(FileSDKInterface::Instance(), &FileSDKInterface::SigTransmitTaskEndRes, this, [=](QString task_id, EFileTransmitTaskState state, EFileTransmitTaskErrorCause cause) {
		if (task_id == task_id_) {
			if (is_ended_) {
				return;
			}
			last_update_time_ = tc::TimeUtil::GetCurrentTimestamp();
			emit SigTransmitTaskRes(task_type_, state, cause);
		}
		});

	connect(FileSDKInterface::Instance(), &FileSDKInterface::SigDownloadTransmitTaskProgress, this, [=](QString task_id, int progress) {
		if (task_id == task_id_) {
			if (is_ended_) {
				return;
			}
			last_update_time_ = tc::TimeUtil::GetCurrentTimestamp();
			emit SigTransmitTaskProgress(progress, false);
		}
		});

	connect(timer_, &QTimer::timeout, this, [=]() {
		if (is_ended_) {
			if (timer_->isActive()) {
				timer_->stop();
			}
			return;
		}
		
		auto curr = tc::TimeUtil::GetCurrentTimestamp();
		//速度计算
		uint64_t temp_size = already_transmit_file_size_ - last_calculate_size_;
		if (temp_size > 0) {
			uint64_t temp_time = curr - last_calculate_time_;
			auto speed = double(temp_size) / double(temp_time);
			last_calculate_time_ = curr;

            // test beg //
            auto mB_size =  (uint32_t)speed / 1024;
            if (mB_size > 50) {
                LOGI("SPEED is so fast: {}MB/s, diff size: {}, diff time: {}, already_transmit_file_size: {}, last_calculate_size: {}",
                     mB_size, temp_size, temp_time, already_transmit_file_size_, last_calculate_size_);
            }
            // test end //

            last_calculate_size_ = already_transmit_file_size_;

			emit SigSpeed(speed);
		}
		if (curr <= last_update_time_) {
			return;
		}
		if (curr - last_update_time_ > 60 * 1000) { // timeout 1 min
			std::cout << task_id_.toStdString() << "task time out" << std::endl;
			emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kTimeOut);
		}
	});
}

void FileTransmitSingleTask::DoUpload() {
	QFileInfo finfo{ current_file_path_ };
	std::string cur_path = current_file_path_.toStdString();
	std::string tar_path = target_file_path_.toStdString();
	this->task_state_ = EFileTransmitTaskState::KTransmitting;
	last_calculate_time_ = last_update_time_ = tc::TimeUtil::GetCurrentTimestamp();
	timer_->start();
	std::string task_id_str = task_id_.toStdString();
	//FileLogManager::Instance()->AppendLog(QStringLiteral("开始上传任务:将") + current_file_path_  + QStringLiteral(",上传至:") + target_file_path_);
	LOGI("DoUpload file: {} to file: {}", cur_path, tar_path);
	FileSDKInterface::Instance()->UploadFile(cur_path, tar_path, task_id_str, std::move([=](ETcFileTransmitState state, uint64_t upload_size, uint64_t file_size) {
		this->already_transmit_file_size_ = upload_size;
		this->last_update_time_ = tc::TimeUtil::GetCurrentTimestamp();

		if (this->is_delete_) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kDelete);
			return;
		}
		if (this->is_ended_) {
			return;
		}
		if (ETcFileTransmitState::kFileNoneExist == state) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileNotExists);
		}
		else if (ETcFileTransmitState::kUploadProcess == state) {
			static uint64_t last_time = tc::TimeUtil::GetCurrentTimestamp();
			auto current_time = tc::TimeUtil::GetCurrentTimestamp();
			if (current_time - last_time <= 1000) {
				return;
			}
			last_time = current_time;
			if (file_size == 0) {
				this->progress_ = 100;
			}
			else {
				this->progress_ = (int8_t)((upload_size * 1.0) / (file_size * 1.0) * 100);
			}
			if (this->progress_ > 100) {
				this->progress_ = 100;
			}
			emit this->SigTransmitTaskProgress(this->progress_, false);
		}
		else if (ETcFileTransmitState::kFileOpenFailed == state) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileFailedOpen);
		}
		else if (ETcFileTransmitState::kFileReadFailed == state) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileFailedRead);
		}
		else if (ETcFileTransmitState::kUploadReadEnd == state) {
			emit this->SigTransmitTaskProgress(100, true);
		}
		else if (ETcFileTransmitState::kTImeOut == state) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kTimeOut);
		}
		else if (ETcFileTransmitState::kPacketLoss == state) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kPacketLoss);
		}
		else if (ETcFileTransmitState::kUnknowError == state) {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kUnKnown);
		}
		else {
			emit this->SigTransmitTaskRes(this->task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kUnKnown);
		}
	}));
}

void FileTransmitSingleTask::DoDownload() {
	std::string cur_path = current_file_path_.toStdString();
	std::string tar_path = target_file_path_.toStdString();
	this->task_state_ = EFileTransmitTaskState::KTransmitting;
	last_calculate_time_ = last_update_time_ = tc::TimeUtil::GetCurrentTimestamp();
	timer_->start();
	std::string task_id_str = task_id_.toStdString();
	//FileLogManager::Instance()->AppendLog(QStringLiteral("开始下载任务:将") + current_file_path_ + QStringLiteral(",下载至:") + target_file_path_);
	FileSDKInterface::Instance()->DownloadFile(tar_path, cur_path, task_id_str);
}

FileTransmitSingleTask::~FileTransmitSingleTask() {

}

void FileTransmitSingleTask::Run() {
	if (is_delete_) {
		emit SigTransmitTaskRes(task_type_, EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kDelete);
		return;
	}
	if (EFileTransmitTaskType::kUpload == task_type_) {
		auto parent_task_ptr = FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_];
		if (parent_task_ptr->cover_) { // 如果全覆盖
			DoUpload();
			return;
		}
		remote_file_util_->Exists(target_file_path_);
	}
	else if (EFileTransmitTaskType::kDownload == task_type_) {
		auto parent_task_ptr = FileTransmitTaskManager::Instance()->tasks_[this->parent_task_id_];
		if (parent_task_ptr->cover_) { // 如果全覆盖
			DoDownload();
			return;
		}
		local_file_util_->Exists(target_file_path_);
	}
}

void FileTransmitSingleTask::Delete() {
	is_delete_ = true;
}

}
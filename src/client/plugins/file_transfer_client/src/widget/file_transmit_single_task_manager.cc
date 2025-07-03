#include "file_transmit_single_task_manager.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include <qfileinfo.h>
#include <qfileiconprovider.h>
#include <qtimer.h>
#include <quuid.h>
#include "tc_common_new/string_util.h"
#include "tc_common_new/log.h"
#include "file_transmit_single_task.h"
#include "file_trans_record.h"
#include "core/file_sdk_interface.h"
#include "file_log_manager.h"
#include "core/file_transmit_sdk.h"
#include "tc_label.h"

namespace tc {

FileTransmitSingleTaskManager::FileTransmitSingleTaskManager() {

}

FileTransmitSingleTaskManager::~FileTransmitSingleTaskManager() {

}

void FileTransmitSingleTaskManager::AddFileTransmitSingleTask(const std::shared_ptr<FileTransmitSingleTask>& task_ptr) {
	std::cout << (int)task_ptr->task_type_ << " AddFileTransmitSingleTask current_file_path_ = " << task_ptr->current_file_path_.toStdString() << std::endl;
	std::cout << (int)task_ptr->task_type_ << " AddFileTransmitSingleTask target_file_path_ = " << task_ptr->target_file_path_.toStdString() << std::endl;
	{
		std::lock_guard<std::mutex> lck{ mutex_ };
        task_ptr->task_id_ = GenerateTaskId();
		all_tasks_[task_ptr->task_id_] = task_ptr;
	}
	AddSigChannel(task_ptr);
	AddToFileTransRecord(task_ptr);
	if (EFileTransmitTaskType::kDownload == task_ptr->task_type_) {
		if (download_activated_) {
			task_ptr->Run();
			download_activated_ = false;
		}
		else {
			download_tasks_.push(task_ptr);
		}
	}
	else if (EFileTransmitTaskType::kUpload == task_ptr->task_type_) {
		if (upload_activated_) {
			task_ptr->Run();
			upload_activated_ = false;
		}
		else {
			upload_tasks_.push(task_ptr);
		}
	}
}

QString FileTransmitSingleTaskManager::GenerateTaskId() {
	QUuid uuid = QUuid::createUuid();
	QString uuidString = uuid.toString();
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(8) << ++counter;
	return QString::fromStdString(oss.str()) + "_" + QString::fromStdString(FileTransmitSDK::s_stream_id_) + "<gr>" + uuidString;
}


bool FileTransmitSingleTaskManager::HasTask() {
	std::lock_guard<std::mutex> lck{ mutex_ };
	for (auto it = all_tasks_.begin(); it != all_tasks_.end(); ++it) {
		if (!it->second->is_ended_) {
			return true;
		}
	}
	return false;
}

void FileTransmitSingleTaskManager::AddSigChannel(const std::shared_ptr<FileTransmitSingleTask>& task_ptr) {
	connect(task_ptr.get(), &FileTransmitSingleTask::SigTransmitTaskRes, this, [=](EFileTransmitTaskType type, EFileTransmitTaskState state, EFileTransmitTaskErrorCause cause) {
#if 0
		{
			std::lock_guard<std::mutex> lck{ mutex_ };
			std::cout << "---------------------------------------all_tasks_ size = " << all_tasks_.size() << std::endl;
			std::cout << "FileTransRecordContainer size = " << FileTransRecordContainer::Instance()->files_trans_record_info_.size() << std::endl;
			std::string s = std::to_string(FileTransRecordContainer::Instance()->files_trans_record_info_.size());
			static FILE* pf = fopen(".\\FileTransmitSingleTaskManager.txt", "wb");
			fwrite(s.c_str(), 1, s.size(), pf);
			const char* newline = "\r\n"; // 换行符
			fwrite(newline, sizeof(char), 2, pf); // 写入换行符
			fflush(pf);
		}
#endif

		emit SigSpeed(task_ptr->task_type_, "0");

		if (EFileTransmitTaskErrorCause::kDelete == cause || EFileTransmitTaskErrorCause::kTimeOut == cause) {
			if (EFileTransmitTaskErrorCause::kTimeOut == cause) {
				LOGE("EFileTransmitTaskErrorCause::kTimeOut, task_id_: {},  current_file_path_: {}", task_ptr->task_id_.toStdString(), task_ptr->current_file_path_.toStdString());
			}
			FileSDKInterface::Instance()->AbortTransmitTask(task_ptr->task_id_);
		}
		if (task_ptr->is_ended_) {
			std::lock_guard<std::mutex> lck{ mutex_ };
			all_tasks_.erase(task_ptr->task_id_);
			return;
		}
		task_ptr->is_ended_ = true;
		if (task_ptr->timer_->isActive()) {
			task_ptr->timer_->stop();
		}

		GenerateLog(task_ptr, type, state, cause);

		if (EFileTransmitTaskType::kDownload == type) {
			if (download_tasks_.empty()) {
				download_activated_ = true;
			}
			else {
				auto task = download_tasks_.front();
				download_tasks_.pop();
				std::cout << "next download task" << std::endl;
				task->Run();
			}
		}
		else if (EFileTransmitTaskType::kUpload == type) {
			if (upload_tasks_.empty()) {
				upload_activated_ = true;
			}
			else {
				auto task = upload_tasks_.front();
				upload_tasks_.pop();
				std::cout << "next upload task" << std::endl;
				task->Run();
			}
		}
		if (EFileTransmitTaskState::kSuccess == state) {
			emit SigTransmitSuccess(type);
		}
		task_ptr->task_state_ = state;
		task_ptr->task_error_cause_ = cause;
		std::cout << "FileTransmitSingleTaskManager SigTransmitTaskRes state = " << (int)state << std::endl;
		// 同步到列表记录中显示
		if (FileTransRecordContainer::Instance()->files_trans_record_info_.count(task_ptr->task_id_) > 0) {
			FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_].state_ = task_ptr->task_state_;
			FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_].cause_ = task_ptr->task_error_cause_;
			FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_].speed_ = "--";
			FileTransRecordTableModel::Instance()->Updatefilecontainer();
		}
		std::lock_guard<std::mutex> lck{ mutex_ };
		all_tasks_.erase(task_ptr->task_id_);
	});

	connect(task_ptr.get(), &FileTransmitSingleTask::SigTransmitTaskProgress, this, [=](int progress, bool verifying) {
		if (task_ptr->is_ended_) {
			return;
		}
		task_ptr->task_state_ = EFileTransmitTaskState::KTransmitting;
		if (verifying) {
			task_ptr->task_state_ = EFileTransmitTaskState::kVerifying;
			std::cout << "SigTransmitTaskProgress kVerifying" << std::endl;
		}
		if (FileTransRecordContainer::Instance()->files_trans_record_info_.count(task_ptr->task_id_) > 0) {
			FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_].state_ = task_ptr->task_state_;
			FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_].schedule_ = progress;
			FileTransRecordTableModel::Instance()->Updatefilecontainer();
		}
		});

	connect(task_ptr.get(), &FileTransmitSingleTask::SigSpeed, this, [=](double speed) {
		if (task_ptr->is_ended_) {
			return;
		}
		auto bytes = speed * 1000;
		auto format_speed = QString::fromStdString(tc::StringUtil::FormatSize(bytes) + "/s");
		if (FileTransRecordContainer::Instance()->files_trans_record_info_.count(task_ptr->task_id_) > 0) {
			FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_].speed_ = QString::fromStdString(tc::StringUtil::FormatSize(bytes) + "/s");
			FileTransRecordTableModel::Instance()->Updatefilecontainer();
		}
		emit SigSpeed(task_ptr->task_type_, format_speed);
	});
}


void FileTransmitSingleTaskManager::AddToFileTransRecord(const std::shared_ptr<FileTransmitSingleTask>& task_ptr) {
	// 在于record widget 关联起来
	FileTransRecordDetailInfo record_info;
	QFileInfo finfo{ task_ptr->current_file_path_ };
	record_info.file_name_ = finfo.fileName();
	record_info.origin_path_ = task_ptr->current_file_path_;
	record_info.target_path_ = task_ptr->target_file_path_;
	record_info.state_ = task_ptr->task_state_;
	record_info.type_ = task_ptr->task_type_;
	record_info.file_size_ = task_ptr->file_size_;
	QFileIconProvider iconProvider;
	record_info.file_icon_ = iconProvider.icon(finfo);
	FileTransRecordContainer::Instance()->files_trans_record_info_[task_ptr->task_id_] = record_info;
	FileTransRecordTableModel::Instance()->Updatefilecontainer();
}


void FileTransmitSingleTaskManager::DeleteFileTransmitSingleTask(const QString& task_id) {
	std::lock_guard<std::mutex> lck{ mutex_ };
	if (all_tasks_.count(task_id) > 0) {
		all_tasks_[task_id]->Delete();
	}
}

std::optional<bool> FileTransmitSingleTaskManager::IsDelete(const QString& task_id) {
	std::lock_guard<std::mutex> lck{ mutex_ };
	if (all_tasks_.count(task_id) > 0) {
		return all_tasks_[task_id]->is_delete_;
	}
	return std::nullopt;
}

std::optional<bool> FileTransmitSingleTaskManager::IsEnded(const QString& task_id) {
	std::lock_guard<std::mutex> lck{ mutex_ };
	if (all_tasks_.count(task_id) > 0) {
		return all_tasks_[task_id]->is_ended_;
	}
	return std::nullopt;
}

std::shared_ptr<FileTransmitSingleTask> FileTransmitSingleTaskManager::GetTaskById(const QString& task_id) {
	std::lock_guard<std::mutex> lck{ mutex_ };
	if (all_tasks_.count(task_id) > 0) {
		return all_tasks_[task_id];
	}
	return nullptr;
}

void FileTransmitSingleTaskManager::GenerateLog(std::shared_ptr<FileTransmitSingleTask> task_ptr, EFileTransmitTaskType type, EFileTransmitTaskState state, EFileTransmitTaskErrorCause cause) {
	QString log;
	std::shared_ptr<void> auto_log(nullptr, [=, &log](void* temp) {
		FileLogManager::Instance()->AppendLog(log);
		});
	switch (type)
	{
	case EFileTransmitTaskType::kUpload:
		//log += QStringLiteral("上传任务: ");
		log += tcTr("id_file_trans_upload_task");
		break;
	case EFileTransmitTaskType::kDownload:
		//log += QStringLiteral("下载任务: ");
		log += tcTr("id_file_trans_down_task");
		break;
	}

	//log += QStringLiteral("源路径:") + task_ptr->current_file_path_ + QStringLiteral(",目标路径:") + task_ptr->target_file_path_ + QStringLiteral(";");

	log += tcTr("id_file_trans_origin_path") + task_ptr->current_file_path_ + tcTr("id_file_trans_target_path") + task_ptr->target_file_path_ + QStringLiteral(";");

	switch (state)
	{
	case EFileTransmitTaskState::kSuccess:
		//log += QStringLiteral("成功; ");
		log += tcTr("id_file_trans_state_success");
		return;
		break;
	case EFileTransmitTaskState::kError:
		//log += QStringLiteral("失败; ");
		log += tcTr("id_file_trans_state_failed");
		break;
	case EFileTransmitTaskState::kDelete:
		//log += QStringLiteral("取消; ");
		log += tcTr("id_file_trans_state_cancel");
		return;
		break;
	default:
		break;
	}

	//log += QStringLiteral("失败原因:");

	log += tcTr("id_file_trans_failed_reason");

	switch (cause)
	{
	case EFileTransmitTaskErrorCause::kUnKnown:
		//log += QStringLiteral("未知");
		log += tcTr("id_file_trans_failed_cause_unknow");
		break;
	case EFileTransmitTaskErrorCause::kCancel:
		//log += QStringLiteral("用户取消");
		log += tcTr("id_file_trans_failed_cause_cancel");
		break;
	case EFileTransmitTaskErrorCause::kSkip:
		//log += QStringLiteral("用户跳过");
		log += tcTr("id_file_trans_failed_cause_skip");
		break;
	case EFileTransmitTaskErrorCause::kDelete:
		//log += QStringLiteral("用户删除");
		log += tcTr("id_file_trans_failed_cause_del");
		break;
	case EFileTransmitTaskErrorCause::kMd5VerifyError:
		//log += QStringLiteral("md5校验失败");
		log += tcTr("id_file_trans_failed_cause_md5_verify");
		break;
	case EFileTransmitTaskErrorCause::kSignVerificationFails:
		//log += QStringLiteral("签名校验失败");
		log += tcTr("id_file_trans_failed_cause_sign_verify");
		break;
	case EFileTransmitTaskErrorCause::kFileNotExists:
		//log += QStringLiteral("文件不存在");
		log += tcTr("id_file_trans_failed_cause_file_no_exists");
		break;
	case EFileTransmitTaskErrorCause::kFileFailedOpen:
		//log += QStringLiteral("文件打开失败");
		log += tcTr("id_file_trans_failed_cause_file_open");
		break;
	case EFileTransmitTaskErrorCause::kFileFailedRead:
		//log += QStringLiteral("文件读取异常");
		log += tcTr("id_file_trans_failed_cause_file_read");
		break;
	case EFileTransmitTaskErrorCause::kDirFailedCreate:
		//log += QStringLiteral("文件夹创建失败");
		log += tcTr("id_file_trans_failed_cause_folder_create");
		break;
	case EFileTransmitTaskErrorCause::kRemoteFileFailedOpen:
		//log += QStringLiteral("远端文件打开失败");
		log += tcTr("id_file_trans_failed_cause_remote_file_open");
		break;
	case EFileTransmitTaskErrorCause::kFileFailedWrite:
		//log += QStringLiteral("写文件异常");
		log += tcTr("id_file_trans_failed_cause_file_write");
		break;
	case EFileTransmitTaskErrorCause::kVerifyError:
		//log += QStringLiteral("校验失败");
		log += tcTr("id_file_trans_failed_cause_verify");
		break;
	case EFileTransmitTaskErrorCause::kTimeOut:
		//log += QStringLiteral("网络超时");
		log += tcTr("id_file_trans_failed_cause_time_out");
		break;
	case EFileTransmitTaskErrorCause::kTCcaException:
		//log += QStringLiteral("流路异常");
		log += tcTr("id_file_trans_failed_cause_stream_exception");
		break;
	case EFileTransmitTaskErrorCause::kPacketLoss:
		//log += QStringLiteral("网络丢包");
		log += tcTr("id_file_trans_failed_cause_packet_loss");
		break;
	default:
		//log += QStringLiteral("未知");
		log += tcTr("id_file_trans_failed_cause_unknow");
		break;
	}
}


}
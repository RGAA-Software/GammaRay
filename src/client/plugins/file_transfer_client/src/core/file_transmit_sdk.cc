#include "file_transmit_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include "file_msg_answer_cbk.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/log.h"
#include "../widget/file_log_manager.h"

namespace tc {

class FixedSizeDeque {
private:
	std::deque<uint64_t> dq_;
	const size_t max_size_ = 20;

public:
	void Push(int value) {
		if (dq_.size() >= max_size_) {
			dq_.pop_front();
		}
		dq_.push_back(value);
	}
	void Pint() const {
		LOGI("g_file_index_deque start print");
		for (uint64_t index : dq_) {
			LOGI("g_file_index_deque index: {}", index);
		}
		LOGI("g_file_index_deque end print");
	}
	void Clear() {
		dq_.clear();
	}
};

FixedSizeDeque g_file_index_deque;

std::mutex FileTransmitSDK::file_transmit_mutex_;

std::string FileTransmitSDK::s_device_id_;
std::string FileTransmitSDK::s_stream_id_;

std::map<std::string, FileTransmitSDK::EFileTransmitTaskSimpleState> FileTransmitSDK::file_transmit_task_with_simple_state_;

std::shared_ptr<FileTransmitSDK> FileTransmitSDK::Make(const std::string& device_id, const std::string& stream_id) {
	return std::make_shared<FileTransmitSDK>(device_id, stream_id);
}

FileTransmitSDK::FileTransmitSDK(const std::string& device_id, const std::string& stream_id) : io_ctx_(1), asio_timer_(io_ctx_) {
    s_device_id_ = device_id;
    s_stream_id_ = stream_id;
	io_ctx_.start();
	AddTimer(std::chrono::milliseconds(6000), std::bind(&FileTransmitSDK::On6000msTimer, this));
	msg_answer_cst_ = std::make_shared<FileMsgAnswerCbkStructure>();
}

FileTransmitSDK::~FileTransmitSDK() {
	file_upload_thread_.stop();
	file_upload_thread_.join();
	file_download_thread_.stop();
	file_download_thread_.join();
	asio_timer_.stop();
	io_ctx_.stop();
}

uint64_t FileTransmitSDK::GetCurrentTimestamp() {
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp
		= std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	std::time_t timestamp = tp.time_since_epoch().count();
	return timestamp;
}

void FileTransmitSDK::On6000msTimer() {
	std::vector<std::string> task_ids;
	std::lock_guard<std::mutex> lg(id_with_download_task_mutex_);
	for (auto it = id_with_download_task_.begin(); it != id_with_download_task_.end(); ++it) {
		if (it->second->is_ended_) {
            LOGI("Will clear this task since this task was ended: {}", it->first);
			if (it->second->file_ptr_->IsOpen()) {
				it->second->file_ptr_->Close();
			}
			task_ids.emplace_back(it->first);
			continue;
		}

        auto now = GetCurrentTimestamp();
        auto diff = now - it->second->last_update_time_;
		if (diff >= 14 * 1000) {
            LOGI("Will clear this task since update time error: {}, now: {}, last: {}, diff: {}ms", it->first, now, it->second->last_update_time_, diff);
			it->second->is_ended_ = true;
			if (it->second->file_ptr_->IsOpen()) {
				it->second->file_ptr_->Close();
			}
			task_ids.emplace_back(it->first);
		}
	}
	for (auto& id : task_ids) {
        LOGI("Clear task: {}, path: {}", id, id_with_download_task_[id]->target_file_path_);
		id_with_download_task_.erase(id);
	}
}

void FileTransmitSDK::GetFilesList(const std::string& path, OnFileOperateCallbackFuncType&& resp_callback) {
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_path_of_filelist(path);
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kGetFilesList);
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::RecursiveGetFilesList(const std::string& path, OnFileOperateCallbackFuncType&& resp_callback) {
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_path_of_filelist(path);
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kRecursiveGetFilesList);
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::BatchCreateFolders(const std::string& paths, OnFileOperateCallbackFuncType&& resp_callback) {
	if (paths.empty()) {
		return;
	}
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kBatchCreateFolders);
	std::vector<std::string> paths_vec;
	//boost::algorithm::split_regex(paths_vec, paths, boost::regex(path_split_));
	// to do 测试
	tc::StringUtil::Split(paths, paths_vec, path_split_);
	for (auto path : paths_vec) {
		if (path.empty()) {
			continue;
		}
		file_operate_msg->add_paths_of_create_folder(path);
	}
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::Exists(const std::string& path, OnFileOperateCallbackFuncType&& resp_callback) {
	if (path.empty()) {
		return;
	}
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_path_of_judge_exists(path);
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kIsExists);
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::CreateNewFolder(const std::string& parent_path, OnFileOperateCallbackFuncType&& resp_callback) {
	if (parent_path.empty()) {
		return;
	}
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_path_of_create_new_folder(parent_path);
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kCreateNewFolder);
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::Rename(std::string old_path, std::string new_name, OnFileOperateCallbackFuncType&& resp_callback) {
	if (old_path.empty() || new_name.empty()) {
		return;
	}
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_path_of_rename(old_path);
	file_operate_msg->set_name_of_rename(new_name);
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kRename);
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::Remove(std::string paths, OnFileOperateCallbackFuncType&& resp_callback) {
	if (paths.empty()) {
		return;
	}
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kDel);

	std::vector<std::string> paths_vec;
	tc::StringUtil::Split(paths, paths_vec, path_split_);
	for (auto path : paths_vec) {
		if (path.empty()) {
			continue;
		}
		file_operate_msg->add_paths_of_del(path);
	}
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		msg_answer_cst_->Add(msg, resp_callback);
		send_message_func_(msg);
	}
}

void FileTransmitSDK::HandleFileOperateRespMessage(const std::shared_ptr<tc::Message>& msg) {
	msg_answer_cst_->HandleRespAnswerMessage(msg);
}

void FileTransmitSDK::HandleFileUploadMessage(const tc::FileTransRespUpload& resp_upload) {
	bool upload_res = resp_upload.res();
	std::string task_id = resp_upload.task_id();
	auto error_cause = resp_upload.error_cause();
	std::string src_file_path = resp_upload.src_file_path();
	std::string target_file_path = resp_upload.target_file_path();
	if (upload_res) {//上传成功
        LOGI("{} upload to {} SUCCESS.", src_file_path, target_file_path);
		file_upload_end_callback_(ETcFileTransmitState::kSuccess, task_id);
	}
	else { // 上传失败
		LOGE("{} upload to {} error", src_file_path, target_file_path);
		file_transmit_task_with_simple_state_[task_id] = EFileTransmitTaskSimpleState::kOppositeEndError;
		switch (error_cause)
		{
		case tc::FileTransRespUpload_UploadErrorCause_kFailedOpen:
			file_upload_end_callback_(ETcFileTransmitState::kRemoteFileOpenFailed, task_id);
			LOGE("ETcFileTransmitState::kRemoteFileOpenFailed");
			break;
		case tc::FileTransRespUpload_UploadErrorCause_kFailedVerify:
			file_upload_end_callback_(ETcFileTransmitState::kVerifyError, task_id);
			LOGE("ETcFileTransmitState::kVerifyError");
			break;
		case tc::FileTransRespUpload_UploadErrorCause_kFailedWrite:
			file_upload_end_callback_(ETcFileTransmitState::kFileWriteFailed, task_id);
			LOGE("ETcFileTransmitState::kFileWriteFailed");
			break;
		case tc::FileTransRespUpload_UploadErrorCause_kUnknow:
			file_upload_end_callback_(ETcFileTransmitState::kUnknowError, task_id);
			LOGE("ETcFileTransmitState::kUnknowError");
			break;
		case tc::FileTransRespUpload_UploadErrorCause_kPacketLoss:
			file_upload_end_callback_(ETcFileTransmitState::kPacketLoss, task_id);
			LOGE("ETcFileTransmitState::kPacketLoss");
			break;
		case tc::FileTransRespUpload_UploadErrorCause_kDirFailedCreate:
			file_upload_end_callback_(ETcFileTransmitState::kCreateFolderFailed, task_id);
			LOGE("ETcFileTransmitState::kCreateFolderFailed");
			break;
		default:
			file_upload_end_callback_(ETcFileTransmitState::kUnknowError, task_id);
			LOGE("ETcFileTransmitState::kUnknowError");
			break;
		}
	}
}

// 当对端有异常时候会接收到 FileTransmitRespDownload 消息
void FileTransmitSDK::HandleFileDownloadMessage(const tc::FileTransRespDownload& resp_download) {
	auto error_cause = resp_download.error_cause();
	auto task_id = resp_download.task_id();
	if (file_download_callback_) {
		if (tc::FileTransRespDownload_DownloadErrorCause_kNoExists == error_cause) {
			file_download_callback_(ETcFileTransmitState::kFileNoneExist, task_id, 0, 0);
		}
		else if (tc::FileTransRespDownload_DownloadErrorCause_kFailedOpen == error_cause) {
			file_download_callback_(ETcFileTransmitState::kRemoteFileOpenFailed, task_id, 0, 0);
		}
		else if (tc::FileTransRespDownload_DownloadErrorCause_kUnknow == error_cause) {
			file_download_callback_(ETcFileTransmitState::kUnknowError, task_id, 0, 0);
		}
		else if (tc::FileTransRespDownload_DownloadErrorCause_kFailedRead == error_cause) {
			file_download_callback_(ETcFileTransmitState::kFileReadFailed, task_id, 0, 0);
		}
		else {
			file_download_callback_(ETcFileTransmitState::kUnknowError, task_id, 0, 0);
		}
	}
	{
		std::lock_guard<std::mutex> lg(id_with_download_task_mutex_);
		if (id_with_download_task_.count(task_id)) {
			id_with_download_task_[task_id]->is_ended_ = true;
			if (id_with_download_task_[task_id]->file_ptr_->IsOpen()) {
				id_with_download_task_[task_id]->file_ptr_->Close();
			}
		}
	}
}

void FileTransmitSDK::HandleFileTransmitDataPacket(const tc::FileTransDataPacket& file_data_packet) {
    // test beg //
	if (false) {
	    static std::ofstream tst_file("1.test.recv.zip", std::ios::binary);
	    std::string data = file_data_packet.data();
	    tst_file.write(data.data(), data.size());
	    tst_file.flush();
	}
    // test end //

	asio::post(file_download_thread_, [=, this]() {
		std::string task_id;
		uint64_t src_file_size;
		std::string src_file_path;
		std::string target_file_path;
		try {
			task_id = file_data_packet.task_id();
			std::string file_data = file_data_packet.data();
			uint64_t index = file_data_packet.index();
			src_file_path = file_data_packet.src_file_path();
			target_file_path = file_data_packet.target_file_path();
			auto transmit_state = file_data_packet.transmit_state();
			src_file_size = file_data_packet.file_size();
			std::string data = file_data_packet.data();
			std::lock_guard<std::mutex> lg(id_with_download_task_mutex_);
			if (0 == index % 100) {  // per 100 send once
				SendFileTransDataPacketResponseMessage(task_id, index);
			}
			if (!id_with_download_task_.count(task_id)) {
				//新的下载任务
				auto download_task = std::make_shared<tc::FileDownloadTask>();
				id_with_download_task_[task_id] = download_task;
				download_task->task_id_ = task_id;
				download_task->src_file_path_ = src_file_path;
				download_task->target_file_path_ = target_file_path;
				download_task->current_packet_index_ = index;
				QDir temp_path{ QString::fromStdString(target_file_path) };
				QString temp_path_str = temp_path.absoluteFilePath("..");
				QString path_str = QDir(temp_path_str).absolutePath();
				QDir target_dir{ path_str };
				if (!target_dir.exists()) {
					target_dir.mkpath(".");
				}
				if (!target_dir.exists()) {
					LOGE("HandleRespFileTransmitDataPacketMessage error, target_dir = {} , can not be created.", target_dir.path().toStdString());
					SendSaveFileExceptionMessage(src_file_path, target_file_path, task_id, tc::FileTransSaveFileException::kDirFailedCreate);
					id_with_download_task_[task_id]->is_ended_ = true;
					file_download_callback_(ETcFileTransmitState::kCreateFolderFailed, task_id, 0, 0);
					return;
				}

                LOGI("Create file: {}, task_id: {}", target_file_path, task_id);
				download_task->file_ptr_ = tc::File::OpenForWriteB(target_file_path);
				if (!id_with_download_task_[task_id]->file_ptr_->IsOpen()) {
                    LOGE("Create file failed!");
					id_with_download_task_[task_id]->is_ended_ = true;
					SendSaveFileExceptionMessage(src_file_path, target_file_path, task_id, tc::FileTransSaveFileException::kFailedOpen);
					file_download_callback_(ETcFileTransmitState::kFileOpenFailed, task_id, 0, 0);
					return;
				}
			}
			else {
				g_file_index_deque.Push(index);
				if (index - id_with_download_task_[task_id]->current_packet_index_ != 1) {
					// 发生了丢包
					LOGE("kPacketLoss file name : {}", id_with_download_task_[task_id]->target_file_path_);
					LOGE("index : {}", index);
					LOGE("id_with_upload_task_[task_id]->current_packet_index_ : {}", id_with_download_task_[task_id]->current_packet_index_);
					g_file_index_deque.Pint();
					g_file_index_deque.Clear();
					id_with_download_task_[task_id]->file_ptr_->Close();
					id_with_download_task_[task_id]->is_ended_ = true;
					SendSaveFileExceptionMessage(src_file_path, target_file_path, task_id, tc::FileTransSaveFileException::kPacketLoss);
					file_download_callback_(ETcFileTransmitState::kPacketLoss, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
					return;
				}
				id_with_download_task_[task_id]->current_packet_index_ = index;
			}

			{ // 判断任务是否被取消, 如果被取消，就通知对端
				std::lock_guard<std::mutex> lck{ file_transmit_mutex_ };
				if (EFileTransmitTaskSimpleState::kCancel == file_transmit_task_with_simple_state_[task_id]) {
                    LOGW("File was canceled: {}", id_with_download_task_[task_id]->target_file_path_);
					id_with_download_task_[task_id]->file_ptr_->Close();
					id_with_download_task_[task_id]->is_ended_ = true;
					SendSaveFileExceptionMessage(src_file_path, target_file_path, task_id, tc::FileTransSaveFileException::kCancel);
					return;
				}
			}
			if (id_with_download_task_[task_id]->is_ended_) {
                LOGW("Ended file, ignore writing: {}", id_with_download_task_[task_id]->target_file_path_);
				return;
			}
			id_with_download_task_[task_id]->last_update_time_ = GetCurrentTimestamp();
			if (id_with_download_task_[task_id]->file_ptr_->IsOpen()) {
				if (!data.empty()) {
					auto append_size = id_with_download_task_[task_id]->file_ptr_->Append(data.data(), data.size());
					//id_with_download_task_[task_id]->file_ptr_->Flush();
					if (append_size != data.size()) {
						LOGE("HandleRespFileTransmitDataPacketMessage append_size != data.size");
						SendSaveFileExceptionMessage(src_file_path, target_file_path, task_id, tc::FileTransSaveFileException::kFailedWrite);
						id_with_download_task_[task_id]->is_ended_ = true;
						id_with_download_task_[task_id]->file_ptr_->Close();
						file_download_callback_(ETcFileTransmitState::kFileWriteFailed, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
						return;
					}
					id_with_download_task_[task_id]->saved_file_size_ += append_size;
					//LOGI("Append write_index = %d, id_with_download_task_[task_id]->saved_file_size_ = %llu, src_file_size = %llu, msg_index = %llu", write_index++,
						//id_with_download_task_[task_id]->saved_file_size_, src_file_size, index);
					file_download_callback_(ETcFileTransmitState::kDownloadProcess, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
				}
			}
            else {
                LOGW("File was closed, ignore writing: {}", id_with_download_task_[task_id]->target_file_path_);
            }
			if (tc::FileTransDataPacket_TransmitState_kTransmitting != transmit_state) {
				id_with_download_task_[task_id]->file_ptr_->Close();
				id_with_download_task_[task_id]->is_ended_ = true;
                LOGI("File will be closed: {}", id_with_download_task_[task_id]->target_file_path_);
			}
			switch (transmit_state)
			{
			case tc::FileTransDataPacket_TransmitState_kEnd: {
				// to do 先校验下大小，后面再考虑校验md5
				QFileInfo qfile_info{QString::fromStdString(id_with_download_task_[task_id]->target_file_path_)};
				auto target_file_size = qfile_info.size();
				if (src_file_size == target_file_size) {
					file_download_callback_(ETcFileTransmitState::kSuccess, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
				}
				else {
					LOGE("HandleRespFileTransmitDataPacketMessage end, error size: src_file_size = {}, target_file_size = {}", src_file_size, target_file_size);
					file_download_callback_(ETcFileTransmitState::kVerifyError, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
				}
				break;
			}
			case tc::FileTransDataPacket_TransmitState_kError: // 目前是对端读文件异常了，会发送此消息
				file_download_callback_(ETcFileTransmitState::kFileReadFailed, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
				break;
			default:
				break;
			}
		}
		catch (std::exception& e) {
			LOGE("FileTransmitSDK::HandleRespFileTransmitDataPacketMessage error is {}.", std::string(e.what()));
			SendSaveFileExceptionMessage(src_file_path, target_file_path, task_id, tc::FileTransSaveFileException::kUnknow);
			std::lock_guard<std::mutex> lg(id_with_download_task_mutex_);
			if (id_with_download_task_.count(task_id) > 0) {
				id_with_download_task_[task_id]->file_ptr_->Close();
				id_with_download_task_[task_id]->is_ended_ = true;
				file_download_callback_(ETcFileTransmitState::kUnknowError, task_id, id_with_download_task_[task_id]->saved_file_size_, src_file_size);
			}
		}
	});
}

void FileTransmitSDK::DownloadFile(std::string local_file_path, std::string remote_file_path, std::string task_id) {
	{
		std::lock_guard<std::mutex> lck{ file_transmit_mutex_ };
		file_transmit_task_with_simple_state_[task_id] = EFileTransmitTaskSimpleState::kNormal;
	}
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileOperationEvent);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto file_operate_msg = new tc::FileOperateionsEvent();
	file_operate_msg->set_path_of_save(local_file_path);
	file_operate_msg->set_path_of_download(remote_file_path);
	file_operate_msg->set_operate_type(tc::FileOperateionsEvent::kDownload);
	file_operate_msg->set_task_id(task_id);
	msg->set_allocated_file_operateions_event(file_operate_msg);
	if (send_message_func_) {
		send_message_func_(msg);
	}
}

// 下载文件过程中出现异常，告知远端
void FileTransmitSDK::SendSaveFileExceptionMessage(std::string src_file_path, std::string target_file_path, std::string task_id, tc::FileTransSaveFileException::SaveFileExceptionCause cause) {
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileTransSaveFileException);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto save_exception = new tc::FileTransSaveFileException();
	save_exception->set_error_cause(cause);
	save_exception->set_task_id(task_id);
	save_exception->set_src_file_path(src_file_path);
	save_exception->set_target_file_path(target_file_path);
	msg->set_allocated_file_trans_save_file_exception(save_exception);
	if (send_message_func_) {
		send_message_func_(msg);
	}
}

// 发送接收到的数据包序列，让对端酌情调整速率
void FileTransmitSDK::SendFileTransDataPacketResponseMessage(std::string task_id, uint64_t recved_index) {
	auto msg = std::make_shared<tc::Message>();
	msg->set_type(tc::kFileTransDataPacketResponse);
    msg->set_device_id(s_device_id_);
	msg->set_stream_id(s_stream_id_);
	auto resp = new tc::FileTransDataPacketResponse();
	resp->set_task_id(task_id);
	resp->set_index(recved_index);
	msg->set_allocated_file_trans_data_packet_response(resp);
	if (send_message_func_) {
		send_message_func_(msg);
	}
}

void FileTransmitSDK::AbortFileTransmit(std::string task_id) {
	{
		std::lock_guard<std::mutex> lck{ file_transmit_mutex_ };
		if (file_transmit_task_with_simple_state_.count(task_id)) {
			file_transmit_task_with_simple_state_[task_id] = EFileTransmitTaskSimpleState::kCancel;
		}
	}
}

void FileTransmitSDK::UploadFile(std::string src_file_path, std::string target_file_path, std::string task_id, UploadFileCallbackFuncType&& upload_callback) {
	asio::post(file_upload_thread_, [=]() {
		try {
			{
				std::lock_guard<std::mutex> lck{ file_transmit_mutex_ };
				file_transmit_task_with_simple_state_[task_id] = EFileTransmitTaskSimpleState::kNormal;
			}
			//const std::size_t buffer_size = 4096;
			char buffer[kSingleBufferSize] = { 0, };

			ResetTokenBucket(); //初始赋值令牌桶10个令牌,慢启动
			int timer_id = AddTimer(std::chrono::milliseconds(100), std::bind(&FileTransmitSDK::GrantTokenBucket, this));
			std::shared_ptr<void> auto_close_timer{ nullptr, [=](void*) {
				StopTimer(timer_id);
				ResetTokenBucket();
			}};

			QString src_file_path_qstr = QString::fromStdString(src_file_path);
			QFileInfo src_file_info{ src_file_path_qstr };


			if (!src_file_info.exists()) {
				LOGE("File:{} no exists", src_file_path);
				upload_callback(ETcFileTransmitState::kFileNoneExist, 0, 0);
				return;
			}
			uint64_t file_size = src_file_info.size();
			std::wstring src_file_pathw = src_file_path_qstr.toStdWString();
#if WIN32
			FILE* pf = _wfopen(src_file_pathw.c_str(), L"rb");
#else
            FILE* pf = fopen(src_file_path.c_str(), "rb");
#endif
			if (!pf) {
				LOGE("File open {} error", src_file_path);
				upload_callback(ETcFileTransmitState::kFileOpenFailed, 0, file_size);
				return;
			}
			std::shared_ptr<void> auto_close_file{ nullptr, [=](void* buf) {
				fclose(pf);
			} };
			uint64_t statistics_readed_size = 0;
			uint64_t index = 0;
			// 是否已经发送过消息了
			bool sended_msg = false;
			while (true) {
				bool is_send_msg = true;
				auto msg = std::make_shared<tc::Message>();
				msg->set_type(tc::kFileTransDataPacket);
                msg->set_device_id(s_device_id_);
				msg->set_stream_id(s_stream_id_);
				auto file_data_packet = new tc::FileTransDataPacket();
				file_data_packet->set_index(index++);
				file_data_packet->set_transmit_direction(tc::FileTransDataPacket::kUpload);
				file_data_packet->set_task_id(task_id);
				file_data_packet->set_src_file_path(src_file_path);
				file_data_packet->set_target_file_path(target_file_path);
				file_data_packet->set_file_size(file_size);
				msg->set_allocated_file_trans_data_packet(file_data_packet);
				std::shared_ptr<void> auto_send{ nullptr, [=, &is_send_msg, &sended_msg](void* buf) {
					if (0 >= token_bucket_) {
						std::unique_lock<std::mutex> lck{ grant_token_mutex_ };
#if 0					// wait 用法
						grant_token_cv_.wait(lck, [=]() ->bool {
							if (token_bucket_ > 0) {
								return true;
							}
							return false;
						});
						grant_token_cv_.wait_for(lck, std::chrono::milliseconds(3000));
#endif
						// wait_for用法
						grant_token_cv_.wait_for(lck, std::chrono::milliseconds(3000), [=]() ->bool {
							if (task_id_with_recved_index_.count(task_id)) {
								if (index - task_id_with_recved_index_[task_id] < 8000 && token_bucket_ > 0) {
									return true;
								}
								else {
									LOGW("src_file_path: {}, UploadFile index: {}, task_id_with_recved_index_[task_id]: {}, token_bucket_: {}", src_file_path, index, task_id_with_recved_index_[task_id], token_bucket_);
									return false;
								}
							}
							if (token_bucket_ > 0) {
								return true;
							}
							return false;
						});
					}
					if (!is_send_msg || !send_message_func_) {
						return;
					}
					if (!send_message_func_(msg)) {
						LOGE("FileUpload send msg time out.");
						upload_callback(ETcFileTransmitState::kTImeOut, statistics_readed_size, file_size);
						return;
					}
					--token_bucket_;
					sended_msg = true;
					//std::this_thread::sleep_for(std::chrono::milliseconds(1));//限速
				} };
				{ // 判断任务是否被取消 或者 对端发生了异常
					std::lock_guard<std::mutex> lck{ file_transmit_mutex_ };
					if (EFileTransmitTaskSimpleState::kCancel == file_transmit_task_with_simple_state_[task_id]) {
                        LOGE("FileUpload task {} is cancel or timeout. src_file_path:{}  ", task_id, src_file_path);
						if (sended_msg) {
							//通知对端此任务取消
							file_data_packet->set_transmit_state(tc::FileTransDataPacket::kCancel);
						}
						else {
							is_send_msg = false;
						}
						return;
					}
					else if (EFileTransmitTaskSimpleState::kOppositeEndError == file_transmit_task_with_simple_state_[task_id]) {
						//对端错误
						is_send_msg = false;
						return;
					}
				}
				std::size_t readed_size = fread(buffer, 1, kSingleBufferSize, pf);
				if (readed_size > 0) {
					statistics_readed_size += readed_size;
					file_data_packet->set_data(buffer, readed_size);
					upload_callback(ETcFileTransmitState::kUploadProcess, statistics_readed_size, file_size);
					if (feof(pf)) { // 文件结束
                        LOGI("upload file at end: {}", src_file_path_qstr.toStdString());
						file_data_packet->set_transmit_state(tc::FileTransDataPacket::kEnd);
						break;
					}
					else {
						file_data_packet->set_transmit_state(tc::FileTransDataPacket::kTransmitting);
						continue;
					}
				}
				else {
					if (feof(pf)) {
                        LOGI("upload file at end: {}", src_file_path_qstr.toStdString());
						file_data_packet->set_transmit_state(tc::FileTransDataPacket::kEnd);
					}
					else {
						file_data_packet->set_transmit_state(tc::FileTransDataPacket::TransmitState::FileTransDataPacket_TransmitState_kError);
						LOGE("File read {} error", src_file_path);
						upload_callback(ETcFileTransmitState::kFileReadFailed, statistics_readed_size, file_size);
						return;
					}
					break;
				}
			} // end while
			upload_callback(ETcFileTransmitState::kUploadReadEnd, statistics_readed_size, file_size);
		}
		catch (std::exception& e) {
			LOGE("FileTransmitSDK::FileUpload error is {}.", std::string(e.what()));
			upload_callback(ETcFileTransmitState::kUnknowError, 0, 0);
		}
	});
}

void FileTransmitSDK::HandleFileTransDataPacketResponse(tc::FileTransDataPacketResponse data_packet_resp) {
	uint64_t recved_index = data_packet_resp.index();
	std::string task_id = data_packet_resp.task_id();
	//LOGI("HandleFileTransDataPacketResponse task_id: {}, recved_index: {}", task_id, recved_index);
	std::unique_lock<std::mutex> lck{ grant_token_mutex_ };
	task_id_with_recved_index_[task_id] = recved_index;
}

void FileTransmitSDK::GrantTokenBucket() {
	token_bucket_ = token_bucket_ + speed_ / kSingleBufferSize;
	std::unique_lock<std::mutex> lck{ grant_token_mutex_ };
	grant_token_cv_.notify_all();
}

void FileTransmitSDK::ResetTokenBucket() {
	token_bucket_ = 10;
}

} // namespace dl
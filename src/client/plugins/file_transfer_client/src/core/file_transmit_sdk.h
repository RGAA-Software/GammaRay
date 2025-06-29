#pragma once
#include <mutex>
#include <chrono>
#include <iostream>
#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <condition_variable>
#include <asio2/asio2.hpp>
#include "common/file_trans_def.h"

namespace tc {

constexpr uint64_t kSingleBufferSize = 1024 * 4;

class FileMsgAnswerCbkStructure;
class File;
class FileDownloadTask {
public:
	std::string task_id_;
	uint64_t saved_file_size_ = 0;//已经写入文件的大小
	std::shared_ptr<File> file_ptr_;
	std::string src_file_path_;
	std::string target_file_path_;
	uint64_t current_packet_index_ = 0;
	std::atomic<bool> is_ended_ = false;
	std::atomic<uint64_t> last_update_time_ = 0;
};

class FileTransmitSDK {
public:
	static std::shared_ptr<FileTransmitSDK> Make(const std::string& device_id, const std::string& stream_id);
	FileTransmitSDK(const std::string& device_id, const std::string& stream_id);
	~FileTransmitSDK();

	SendMessageFuncType send_message_func_ = nullptr;

	OnFileUploadEndCallbackFunc file_upload_end_callback_ = nullptr;

	OnFileDownloadCallbackFunc file_download_callback_ = nullptr;

	void RegSendMessageCallback(SendMessageFuncType&& callback) {
		send_message_func_ = std::move(callback);
	}

	void RegOnFileUploadEndCallback(OnFileUploadEndCallbackFunc&& callback) {
		file_upload_end_callback_ = std::move(callback);
	}

	void RegOnFileDownloadCallback(OnFileDownloadCallbackFunc&& callback) {
		file_download_callback_ = std::move(callback);
	}

	void UploadFile(std::string local_file_path, std::string remote_file_path, std::string task_id, UploadFileCallbackFuncType&& upload_callback);

	void AbortFileTransmit(std::string task_id);

	void DownloadFile(std::string local_file_path, std::string remote_file_path, std::string task_id);

	void GetFilesList(const std::string& path, OnFileOperateCallbackFuncType&& resp_callback);

	void RecursiveGetFilesList(const std::string& path, OnFileOperateCallbackFuncType&& resp_callback);

	void BatchCreateFolders(const std::string& paths, OnFileOperateCallbackFuncType&& resp_callback);

	void Exists(const std::string& path, OnFileOperateCallbackFuncType&& resp_callback);

	void CreateNewFolder(const std::string& parent_path, OnFileOperateCallbackFuncType&& resp_callback);

	void Rename(std::string old_path, std::string new_name, OnFileOperateCallbackFuncType&& resp_callback);

	void Remove(std::string paths, OnFileOperateCallbackFuncType&& resp_callback);


	void HandleFileOperateRespMessage(const std::shared_ptr<tc::Message>& msg);
	void HandleFileUploadMessage(const tc::FileTransRespUpload& resp_upload);
	// 当对端有异常时候，会调用到HandleFileDownloadMessage
	void HandleFileDownloadMessage(const tc::FileTransRespDownload& resp_download);
	void HandleFileTransmitDataPacket(const tc::FileTransDataPacket& data_packet);
	
	void HandleFileTransDataPacketResponse(tc::FileTransDataPacketResponse data_packet_resp);

	void SendSaveFileExceptionMessage(std::string src_file_path, std::string target_file_path, std::string task_id, tc::FileTransSaveFileException::SaveFileExceptionCause cause);

	void SendFileTransDataPacketResponseMessage(std::string task_id, uint64_t recved_index);

	std::shared_ptr<FileMsgAnswerCbkStructure> msg_answer_cst_ = nullptr;

	const std::string path_split_ = "<path_split>"; 

	asio::thread_pool file_upload_thread_{ 1 };

	asio::thread_pool file_download_thread_{ 1 };

	enum class EFileTransmitTaskSimpleState {
		kNormal,
		kCancel,            //用户取消
		kOppositeEndError,  //对端异常
	};
	// task_id  任务
	static std::mutex file_transmit_mutex_;
	// id_map_state
	static std::map<std::string, EFileTransmitTaskSimpleState> file_transmit_task_with_simple_state_;
	// id_map_down_task
	std::map<std::string, std::shared_ptr<tc::FileDownloadTask>> id_with_download_task_;
	std::mutex id_with_download_task_mutex_;

	// 定时检测
	void On6000msTimer();
	uint64_t GetCurrentTimestamp();
	int AddTimer(const std::chrono::milliseconds& duration, const std::function<void()>& func) {
		asio_timer_.start_timer(++next_timer_id_, duration, func);
		return next_timer_id_.load();
	}
	void StopTimer(int timer_id) {
		asio_timer_.stop_timer(timer_id);
	}
	std::atomic_int next_timer_id_ = -1;
	asio2::iopool io_ctx_;
	asio2::timer asio_timer_;
	static std::string s_device_id_;
	static std::string s_stream_id_;

	// 响应发送机制: 暂未启用
	std::mutex upload_mutex_;
	std::condition_variable upload_cv_;


	// 限速设计:
	// const uint64_t single_buffer_size_ = 1024 * 64;
	// MB / 100ms
	const uint64_t speed_ = 5 * 1 * 1024 * 1024;
	std::atomic<uint64_t> token_bucket_ = {0};
	void GrantTokenBucket();
	void ResetTokenBucket();
	std::mutex grant_token_mutex_;
	std::condition_variable grant_token_cv_;

	std::map<std::string, uint64_t> task_id_with_recved_index_;
	
};
}
#pragma once
#include <qstring.h>
#include <qobject.h>
#include <atomic>
#include <memory>
#include "file_transmit_task_state.h"

class QTimer;

namespace tc {

class RemoteFileUtil;
class LocalFileUtil;

class FileTransmitSingleTask : public QObject {
	Q_OBJECT
public:
	FileTransmitSingleTask();
	~FileTransmitSingleTask();

	void Run();

	void Delete();

	void InitSigChannel();

	// 发往对端路径
	QString target_file_path_;

	// 当前文件路径
	QString current_file_path_;

	// 父任务id
	int32_t parent_task_id_{ 0 };

	QString task_id_;

	uint64_t file_size_ = 0;

	// 下载文件时候，远端文件的 文件大小以及文件时间
	QString remote_file_size_str_;
	QString remote_file_date_str_;

	EFileTransmitTaskState task_state_ = EFileTransmitTaskState::kWait;

	EFileTransmitTaskType task_type_ = EFileTransmitTaskType::kUnKnown;

	EFileTransmitTaskErrorCause task_error_cause_ = EFileTransmitTaskErrorCause::kUnKnown;

	std::atomic<bool> is_delete_{ false };

	std::atomic<bool> is_ended_{ false };

	std::shared_ptr<RemoteFileUtil> remote_file_util_ = nullptr;

	std::shared_ptr<LocalFileUtil> local_file_util_ = nullptr;

	QTimer* timer_ = nullptr;

	std::atomic<uint64_t> last_update_time_ = 0;

	// 计算速度用
	uint64_t last_calculate_time_ = 0;
	uint64_t last_calculate_size_ = 0;
	std::atomic<uint64_t> already_transmit_file_size_ = 0;

	int8_t progress_ = -1;
private:
	void DoUpload();
	void DoDownload();
signals:
	void SigTransmitTaskRes(EFileTransmitTaskType, EFileTransmitTaskState, EFileTransmitTaskErrorCause);
	void SigTransmitTaskProgress(int progress, bool verifying);
	void SigSpeed(double);   //  bytes/ms
};

}
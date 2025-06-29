#pragma once

#include <qobject.h>
#include <qstring.h>
#include <memory>
#include <atomic>
#include "file_detail_info.h"
#include "file_transmit_task_state.h"
#include "common/task_worker_callback.h"

namespace tc {

class RemoteFileUtil;
class LocalFileUtil;

class FileTransmitTask : public QObject {
	Q_OBJECT
public:
	static std::shared_ptr<FileTransmitTask> CreateFileTransmitTask(FileContainer&& file_container);
	FileTransmitTask(FileContainer&& file_container);
	~FileTransmitTask();
	void Run();

	// 发往的对端目录路径
	QString target_dir_path_;

	// 当前所在目录路径
	QString current_dir_path_;

	EFileTransmitTaskType task_type_ = EFileTransmitTaskType::kUnKnown;

	// 全部覆盖
	std::atomic<bool> cover_ = false;

	// 全部跳过
	std::atomic<bool> skip_ = false;

	int32_t task_id_ = -1;
private:
	//待发送的文件集合
	FileContainer file_container_;

	void TraverseDirectory(const QString& path, std::vector<QString>& folders, std::vector<QString>& files);

	QString BuildOppositePath(QString current_path);

	// remote_file_size 与 remote_file_date_str 表示下载任务时候，远端文件的一些信息
	void AddFileTransmitSingleTask(const QString& current_file_path, uint64_t remote_file_size = 0, const QString& remote_file_date_str = "");

	std::shared_ptr<RemoteFileUtil> remote_file_util_ = nullptr;

	std::shared_ptr<LocalFileUtil> local_file_util_ = nullptr;

	void DownloadTaskRun();

	void UploadTaskRun();

	static YKTaskWorker async_download_task_woker_;

	static YKTaskWorker async_upload_task_woker_;
};

}
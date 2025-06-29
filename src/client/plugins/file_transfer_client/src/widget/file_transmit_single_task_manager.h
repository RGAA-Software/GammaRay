#pragma once
#include <qobject.h>
#include <vector>
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include "file_transmit_task_state.h"

namespace tc {

class FileTransmitSingleTask;


class FileTransmitSingleTaskManager : public QObject {
	Q_OBJECT
public:
	static FileTransmitSingleTaskManager* Instance()
	{
		static FileTransmitSingleTaskManager self{};
		return &self;
	}
	FileTransmitSingleTaskManager();
	~FileTransmitSingleTaskManager();

	void AddFileTransmitSingleTask(const std::shared_ptr<FileTransmitSingleTask>& task_ptr);

	QString GenerateTaskId();

	void DeleteFileTransmitSingleTask(const QString& task_id);

	bool IsDelete(const QString& task_id);

	bool IsEnded(const QString& task_id);

	std::shared_ptr<FileTransmitSingleTask> GetTaskById(const QString& task_id);

	bool HasTask();
signals:
	void SigTransmitSuccess(EFileTransmitTaskType);
	void SigSpeed(EFileTransmitTaskType, QString);
private:

	std::atomic<bool> upload_activated_ = true;

	std::queue<std::shared_ptr<FileTransmitSingleTask>> upload_tasks_;

	std::atomic<bool> download_activated_ = true;

	std::queue<std::shared_ptr<FileTransmitSingleTask>> download_tasks_;

	std::atomic<int32_t> counter{ -1 };

	std::mutex mutex_;

	std::map<QString, std::shared_ptr<FileTransmitSingleTask>> all_tasks_;


	void AddSigChannel(const std::shared_ptr<FileTransmitSingleTask>& task_ptr);

	void AddToFileTransRecord(const std::shared_ptr<FileTransmitSingleTask>& task_ptr);

	void GenerateLog(std::shared_ptr<FileTransmitSingleTask> task_ptr, EFileTransmitTaskType type, EFileTransmitTaskState state, EFileTransmitTaskErrorCause cause);
};
}
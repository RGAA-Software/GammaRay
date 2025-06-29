#pragma once

#include <qobject>
#include <memory>
#include <vector>
#include <atomic>
#include <map>

namespace tc {

class FileTransmitTask;

class FileTransmitTaskManager : public QObject {
	Q_OBJECT
public:
	static FileTransmitTaskManager* Instance()
	{
		static FileTransmitTaskManager self{};
		return &self;
	}
	void AddFileTransmitTask(std::shared_ptr<FileTransmitTask> task);

	void InitSigChannel();

	std::atomic<int32_t> GenerateTaskId();

	std::map<int32_t, std::shared_ptr<FileTransmitTask>> tasks_;

private:
	FileTransmitTaskManager();
	~FileTransmitTaskManager();
	std::atomic<int32_t> dispense_task_id_{ -1 };
};


}
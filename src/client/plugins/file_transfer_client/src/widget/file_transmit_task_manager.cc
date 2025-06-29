#include "file_transmit_task_manager.h"
#include <iostream>
#include "file_transmit_task.h"

namespace tc {

FileTransmitTaskManager::FileTransmitTaskManager() {
	InitSigChannel();
}

FileTransmitTaskManager::~FileTransmitTaskManager() {

}

void FileTransmitTaskManager::AddFileTransmitTask(std::shared_ptr<FileTransmitTask> task) {
	task->task_id_ = GenerateTaskId();
	tasks_[task->task_id_] = task;
	task->Run();
}

void FileTransmitTaskManager::InitSigChannel() {

}

std::atomic<int32_t> FileTransmitTaskManager::GenerateTaskId() {
	return ++dispense_task_id_;
}

}
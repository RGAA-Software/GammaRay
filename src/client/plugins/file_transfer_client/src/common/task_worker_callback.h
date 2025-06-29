#ifndef YK_TASK_WORKER_CALLBACK_H_
#define YK_TASK_WORKER_CALLBACK_H_
#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <tuple>
#include <wchar.h>
#include <memory>
#include <thread>
#include <future>
#include <queue>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>

class YKTaskWorker {
public:
	class Task_with_Callback {
	public:
		Task_with_Callback(std::packaged_task<void(void)> exec, std::packaged_task<void(void)> callback) {
			mExec = std::move(exec);
			mCallback = std::move(callback);
		}
		std::packaged_task<void(void)> mExec;
		std::packaged_task<void(void)> mCallback;
	};

	YKTaskWorker() {
		mThread = std::thread(std::bind(&YKTaskWorker::Run, this));
	}
	void Exit() {
		mStopFlag = true;
		std::unique_lock<std::mutex> lck(mMutex);
		mTaskDequeue.clear();
		lck.unlock();
		mCV.notify_all();
		if (mThread.joinable()) {
			mThread.join();
		}
		std::cout << "~YKTaskWorker()" << std::endl;
	}
	~YKTaskWorker() {
		Exit();
	}
	uint64_t TasksCount() {
		std::unique_lock<std::mutex> lck(mMutex);
		return mTaskDequeue.size();
	}
	void AsyncTask(const std::function<void(void)>& func, const std::function<void(void)> callback) {
		if (mStopFlag) {
			std::cout << "YKTaskWorker stoped" << std::endl;
			return;
		}
		std::packaged_task<void(void)> exec(func);
		std::packaged_task<void(void)> cbk(callback);
		Task_with_Callback task(std::move(exec), std::move(cbk));
		std::unique_lock<std::mutex> lck(mMutex);
		mTaskDequeue.push_back(std::move(task));
		mCV.notify_all();
	}
private:
	std::deque<Task_with_Callback> mTaskDequeue;
	std::atomic<bool> mStopFlag = { false };
	std::thread mThread;
	std::mutex mMutex;
	std::condition_variable mCV;

	void Run() {
		while (!mStopFlag) {
			std::unique_lock<std::mutex> lck(mMutex);
			mCV.wait(lck, [=]()->bool { // false,将解锁，并堵塞； true 立马返回
				if (mStopFlag) {
					return true;
				}
				return !mTaskDequeue.empty();
				});
			if (mStopFlag) {
				return;
			}
			/*if (mTaskDequeue.empty()) {
				mCV.wait_for(lck, std::chrono::milliseconds(1));
				if (mTaskDequeue.empty()) {
					continue;
				}
			}*/
			Task_with_Callback task = std::move(mTaskDequeue.front());
			mTaskDequeue.pop_front();
			lck.unlock();
			if (task.mExec.valid()) {
				task.mExec();
			}
			if (task.mCallback.valid()) {
				task.mCallback();
			}
		}
	}
};
#endif
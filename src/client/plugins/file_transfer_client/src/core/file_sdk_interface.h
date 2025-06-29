#pragma once
#include <qobject.h>
#include <qstring.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "tc_message.pb.h"
#include "file_transfer_client/src/widget/file_detail_info.h"
#include "file_transfer_client/src/widget/file_transmit_task_state.h"
#include "file_transfer_client/src/common/file_trans_def.h"

namespace tc {

class FileTransmitSDK;
class FileMsgAnswerCbkStructure;

class FileSDKInterface : public QObject {
	Q_OBJECT
public:
	static FileSDKInterface* Instance() {
		static FileSDKInterface self;
		return &self;
	}
	void InitFileTransSDK(const std::string& device_id, const std::string& stream_id);

	void RegSendMessageFunc(SendMessageFuncType&& func);

	void OnMessage(const std::shared_ptr<Message>& msg);

	void RegisterFileOperateCallback();

	void GetFilesList(const QString& path, OnFileOperateCallbackFuncType&& callback);

	void GetRecursiveFilesList(const QString& path, OnFileOperateCallbackFuncType&& callback);

	void BatchCreateFolders(std::vector<QString> folders, OnFileOperateCallbackFuncType&& callback);

	void ReName(const QString& old_path, const QString& new_name, OnFileOperateCallbackFuncType&& callback);

	void Exists(const QString& file_path, OnFileOperateCallbackFuncType&& callback);

	void CreateNewFolder(const QString& parent_path, OnFileOperateCallbackFuncType&& callback);

	// origin del
	void Remove(std::vector<QString> paths, OnFileOperateCallbackFuncType&& callback);

	void AbortTransmitTask(const QString& task_id);

	// origin FileUpload
	void UploadFile(std::string local_file_path, std::string remote_file_path, std::string task_id, UploadFileCallbackFuncType&& upload_callback);

	// origin FileOperateDownload
	void DownloadFile(std::string local_file_path, std::string remote_file_path, std::string task_id);

    // 设置基本状态回调
    void SetOnFileUploadBeginCallback(OnFileUploadBeginCallback&& cbk);
    void SetOnFileUploadEndCallback(OnFileUploadEndCallback&& cbk);
    void SetOnFileDownloadBeginCallback(OnFileDownloadBeginCallback&& cbk);
    void SetOnFileDownloadEndCallback(OnFileDownloadEndCallback&& cbk);

public:
    QWidget* file_trans_widget_ = nullptr;
	std::shared_ptr<FileTransmitSDK> file_transmit_sdk_ = nullptr;

signals:
	void SigGetFiles(FileContainer);
	void SigExists(bool, QString file_size_str = "", QString file_date_str = "");
	void SigTransmitTaskEndRes(QString task_id, EFileTransmitTaskState, EFileTransmitTaskErrorCause);

	// 下载任务用
	void SigDownloadTransmitTaskProgress(QString task_id, int progress);

	void SigDel(bool ret, QStringList no_del_paths, QString error_msg);

	void SigRename(bool ret, QString old_path, QString new_path, QString error_msg);


	void SigCreateNewFolder(bool ret, QString parent_path, QString new_create_path, QString error_msg);

	void SigTest();

private:
	FileSDKInterface();
	const QString path_split_ = "<path_split>";
	std::shared_ptr<FileMsgAnswerCbkStructure> msg_answer_cbk_t_ = nullptr;
    // 基本状态回调用于记录
    OnFileUploadBeginCallback upload_beg_cbk_;
    OnFileUploadEndCallback upload_end_cbk_;
    OnFileDownloadBeginCallback download_beg_cbk_;
    OnFileDownloadEndCallback download_end_cbk_;
};

}
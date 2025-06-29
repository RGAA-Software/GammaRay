
#ifndef TC_FILE_TRANS_WORKER_CALLBACK_H_
#define TC_FILE_TRANS_WORKER_CALLBACK_H_
#include "base_file_util.h"
#include <qstring.h>
#include <vector>
#include <memory>
#include <qjsonobject.h>


#include "file_transmit_task_state.h"

namespace tc {

class FileNetworkUtil;

class RemoteFileUtil : public BaseFileUtil {
	Q_OBJECT
public:
	RemoteFileUtil();
	//windows下 此电脑下的文件信息
	void GetThisPCFiles() override;

	void GetFiles(const QString& path) override;

	void GetFiles2(const QString& path) override;

	void RecursiveGetFiles(const QString& path) override;

	void BatchCreateFolders(std::vector<QString> folders) override;

	void Remove(std::vector<QString> paths) override;

	void ReName(const QString& old_path, const QString& new_name) override;

	void CreateNewFolder(const QString& parent_path) override;

	void Exists(const QString& path) override;

	static QString current_user_dirs_;
signals:
	void SigRemoteFilePermissionPath();
private:
	std::shared_ptr<FileNetworkUtil> network_util_ = nullptr;


	bool get_file_list_activate_ = false;

	// 用于仅获取文件列表，而不展示在列表上
	std::atomic_int32_t get_file_list_activate2_ = 0;

	bool judge_exists_activate_ = false;

	bool del_activate_ = false;

	bool rename_activate_ = false;

	bool create_new_folder_activate_ = false;
};
}

#endif
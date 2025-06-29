#ifndef TC_FILE_TRANS_WORKER_CALLBACK_H2_
#define TC_FILE_TRANS_WORKER_CALLBACK_H2_

#include "base_file_util.h"
#include <qstring.h>
#include <vector>
#include "common/task_worker_callback.h"

namespace tc {

class LocalFileUtil : public BaseFileUtil {
	Q_OBJECT
public:
	LocalFileUtil();
	//windows下 此电脑下的文件信息
	void GetThisPCFiles() override;

	void GetFiles(const QString& path) override;

	void BatchCreateFolders(std::vector<QString> folders) override;

	void Remove(std::vector<QString> paths) override;

	void ReName(const QString& old_path, const QString& new_name) override;

	void CreateNewFolder(const QString& parent_path) override;

	void Exists(const QString& path) override;

	static QString current_user_dirs_;
private:
	void GetThisPCFilesImpl();
	void GetFilesImpl(const QString& path);
	void BatchCreateFoldersImpl(std::vector<QString> folders);
	void RemoveImpl(std::vector<QString> paths);
	void ReNameImpl(const QString& old_path, const QString& new_name);
	void CreateNewFolderImpl(const QString& parent_path);
	FileContainer current_file_container_;
	static YKTaskWorker async_task_worker_;
	QStringList undel_paths_list_;

	bool rename_res_ = false;

	bool create_new_folder_res_ = false;

	bool get_files_list_ = false;

	QString create_new_folder_path_;
};

}

#endif
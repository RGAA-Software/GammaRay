#ifndef TC_FILE_TRANS_WORKER_CALLBACK_H3_
#define TC_FILE_TRANS_WORKER_CALLBACK_H3_
#include <qstring.h>
#include <vector>
#include <qobject.h>
#include <qjsonobject.h>
#include <qstringlist.h>
#include "file_detail_info.h"

namespace tc {

class BaseFileUtil : public QObject {
	Q_OBJECT
public:
	BaseFileUtil();

	// windows下 此电脑下的文件信息
	virtual void GetThisPCFiles() = 0;

	// 获取path 目录下的文件和文件夹
	virtual void GetFiles(const QString& path) = 0;

	// 获取path 目录下的文件和文件夹 但是不更新列表显示页面
	virtual void GetFiles2(const QString& path) {};

	// 递归获取path 目录下的文件和文件夹 但是不更新列表显示页面
	virtual void RecursiveGetFiles(const QString& path) {};

	virtual void BatchCreateFolders(std::vector<QString> folders) = 0;

	virtual void Remove(std::vector<QString> paths) = 0;

	virtual void ReName(const QString& old_path, const QString& new_name) = 0;

	// 模拟windows系统创建一个文件夹, parent_path 是文件夹的父目录
	virtual void CreateNewFolder(const QString& parent_path) = 0;

	virtual void Exists(const QString& path) = 0;
signals:
	void SigGetFiles(tc::FileContainer file_container);

	void SigGetFiles2(tc::FileContainer file_container);

	void SigRemove();

	void SigReName();

	void SigCreateNewFolder(QString folder_path);

	void SigExists(bool, QString file_size_str = "", QString file_date_str = "");

	//void SigRemoteFilePermissionPath();
};

}

#endif
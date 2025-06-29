#pragma once

#include <QString>
#include <QIcon>
#include <vector>
#include <qmetatype.h>


namespace tc {

enum class EFileType {
	kDisk,
	kFolder,
	kFile,
	kDesktopFolder, //桌面文件夹，
};

// 文件表格中 表头类型
enum class EFileTableHeaderViewItemType {
	KName,
	KSize,
	KType,
	KUpdateTime,
};

class FileDetailInfo {
public:
	FileDetailInfo();
	~FileDetailInfo();

	QString file_name_;
	uint64_t file_size_ = 0;
	QString file_path_;
	QIcon file_icon_;
	QString date_changed_;
	uint64_t data_changed_time_;
	QString suffix_;
	bool local_file_ = true;

	EFileType file_type_ = EFileType::kFolder;

	// 名称
	static bool CompareByNameAsc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.file_name_ < b.file_name_;
	}

	static bool CompareByNameDesc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.file_name_ > b.file_name_;
	}

	// 大小
	static bool CompareBySizeAsc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.file_size_ < b.file_size_;
	}

	static bool CompareBySizeDesc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.file_size_ > b.file_size_;
	}

	// 后缀
	static bool CompareBySuffixAsc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.suffix_ < b.suffix_;
	}

	static bool CompareBySuffixDesc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.suffix_ > b.suffix_;
	}

	// 时间
	static bool CompareByLastModifieAsc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.data_changed_time_ < b.data_changed_time_;
	}

	static bool CompareByLastModifieDesc(const FileDetailInfo& a, const FileDetailInfo& b) {
		return a.data_changed_time_ > b.data_changed_time_;
	}
};

class FileContainer {
public:
	FileContainer();
	~FileContainer();
	void AddFileInfo(FileDetailInfo file_info) {
		files_detail_info_.emplace_back(file_info);
	}
	void Clear() {
		files_detail_info_.clear();
	}
	std::pair<bool, FileDetailInfo> operator[] (int index) {
		if (index >= files_detail_info_.size()) {
			FileDetailInfo temp;
			return std::make_pair(false, temp);
		}
		return std::make_pair(true, files_detail_info_[index]);
	}
	size_t Size() {
		return files_detail_info_.size();
	}
	bool HasDisk();
	std::vector<FileDetailInfo> files_detail_info_;
	QString path_;
	bool home_ = false; //是否是home(比如windows 此电脑目录下就是home)

	// 根据类型排序
	void Sort(EFileTableHeaderViewItemType type, bool asc);
};
Q_DECLARE_METATYPE(tc::FileContainer);

}
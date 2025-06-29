#include "file_detail_info.h"

namespace tc {

FileDetailInfo::FileDetailInfo()
{
}

FileDetailInfo::~FileDetailInfo()
{
}

FileContainer::FileContainer()
{
}

FileContainer::~FileContainer()
{
}

bool FileContainer::HasDisk() {
	for (auto& file_info : files_detail_info_) {
		if (EFileType::kDisk == file_info.file_type_) {
			return true;
		}
	}
	return false;
}

void FileContainer::Sort(EFileTableHeaderViewItemType type, bool asc) {
	//
	//参考windows资源管理器：
	/*
	1. 按照名称排序 
	升序:文件夹在上面，文件在下面
	降序:文件在上面，文件夹在下面

	2. 按照日期排序
	升序:文件夹在上面，文件在下面
	降序:文件在上面，文件夹在下面

	3. 按照类型排序
	不管升序还是降序,文件夹始终在上面, 不过 文件夹的排序 根据 名称 进行升序与降序排列的
	文件按照后缀名进行排序即可

	4. 按照大小排序
	升序: 文件夹在上面，文件在下面
	降序: 文件在上面，文件夹在下面
	其中不管升序还是降序 文件夹按照名称升序排序
	*/

	// 先把文件与文件夹区分开 再排序
	std::vector<FileDetailInfo> files_vector;   // 文件类型
	std::vector<FileDetailInfo> others_vector;  // 文件夹或者其他类型
	
	for (auto file_info : files_detail_info_) {
		if (EFileType::kFile == file_info.file_type_) {
			files_vector.push_back(file_info);
		}
		else {
			others_vector.push_back(file_info);
		}
	}

	std::vector<FileDetailInfo> new_file_info;

	switch (type)
	{
	case tc::EFileTableHeaderViewItemType::KName:
		if (asc) {
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameAsc);
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareByNameAsc);
			others_vector.insert(others_vector.end(), files_vector.begin(), files_vector.end());
			files_detail_info_ = others_vector;
			return;
		}
		else {
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareByNameDesc);
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameDesc);
			files_vector.insert(files_vector.end(), others_vector.begin(), others_vector.end());
			files_detail_info_ = files_vector;
			return;
		}
		break;
	case tc::EFileTableHeaderViewItemType::KSize:
		if (asc) {
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameAsc);
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareBySizeAsc);
			others_vector.insert(others_vector.end(), files_vector.begin(), files_vector.end());
			files_detail_info_ = others_vector;
			return;
		}
		else {
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareBySizeDesc);
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameAsc);
			files_vector.insert(files_vector.end(), others_vector.begin(), others_vector.end());
			files_detail_info_ = files_vector;
			return;
		}
		break;
	case tc::EFileTableHeaderViewItemType::KType:
		if (asc) {
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameAsc);
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareBySuffixAsc);
			others_vector.insert(others_vector.end(), files_vector.begin(), files_vector.end());
			files_detail_info_ = others_vector;
			return;
		}
		else {
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameDesc);
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareBySuffixDesc);
			others_vector.insert(others_vector.end(), files_vector.begin(), files_vector.end());
			files_detail_info_ = others_vector;
			return;
		}
		break;
	case tc::EFileTableHeaderViewItemType::KUpdateTime:
		if (asc) {
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameAsc);
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareByLastModifieAsc);
			others_vector.insert(others_vector.end(), files_vector.begin(), files_vector.end());
			files_detail_info_ = others_vector;
			return;
		}
		else {
			std::sort(files_vector.begin(), files_vector.end(), FileDetailInfo::CompareByLastModifieDesc);
			std::sort(others_vector.begin(), others_vector.end(), FileDetailInfo::CompareByNameAsc);
			files_vector.insert(files_vector.end(), others_vector.begin(), others_vector.end());
			files_detail_info_ = files_vector;
			return;
		}
		break;
	default:
		break;
	}
}

}
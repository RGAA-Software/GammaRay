#include "remote_file_util.h"
#include <iostream>
#include <qstandardpaths.h>
#include <qdebug.h>
#include <qstorageinfo.h>
#include <json/json.hpp>
#include "tc_message.pb.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file_util.h"
#include "tc_label.h"
#include "core/file_sdk_interface.h"
#include "file_log_manager.h"
#include "file_const_def.h"


namespace tc {

FileContainer GetFileContainerFromParseJsonData(const QString& resp_json_data) {
	FileContainer res;
	if (resp_json_data.isEmpty()) {
		return res;
	}
	std::string temp_str = resp_json_data.toStdString();
	nlohmann::json resp_json = nlohmann::json::parse(temp_str);
	if (resp_json.contains("target_path")) {
		res.path_ = QString::fromStdString(resp_json["target_path"]);
	}
	if (resp_json.contains("file_infos")) {
		auto finfos = resp_json["file_infos"];
		for (auto& finfo : finfos) {
			FileDetailInfo file_detail_info;
			file_detail_info.file_size_ = finfo["file_size"];
			file_detail_info.data_changed_time_ = finfo["file_date"];
			file_detail_info.date_changed_ = QDateTime::fromSecsSinceEpoch(finfo["file_date"]).toString("yyyy-MM-dd hh:mm:ss");
			file_detail_info.file_type_ = static_cast<EFileType>(finfo["file_type"]);
			file_detail_info.file_name_ = QString::fromStdString(finfo["file_name"]);
			std::string file_name = file_detail_info.file_name_.toStdString();
			std::string file_suffix = FileUtil::GetFileSuffix(file_name);
			file_detail_info.suffix_ = QString::fromStdString(file_suffix);
			auto temp_path = QString::fromStdString(finfo["file_path"]);
			file_detail_info.file_path_ = temp_path.replace("\\", "/");
			res.AddFileInfo(file_detail_info);
		}	
	}
	return res;
}

QString GetRespDetailErrorMsg(const QString& resp_json_data) {
	QString error_msg;
	if (!resp_json_data.isEmpty()) {
		std::string temp_str = resp_json_data.toStdString();
		nlohmann::json resp_json = nlohmann::json::parse(temp_str);
		if (resp_json.is_object() && resp_json.contains("error_msg")) {
			error_msg = QString::fromStdString(resp_json["error_msg"]);
		}
	}
	return error_msg;
}

QString GetRespFilePermissionPath(const QString& resp_json_data) {
	QString path;
	if (!resp_json_data.isEmpty()) {
		std::string temp_str = resp_json_data.toStdString();
		nlohmann::json resp_json = nlohmann::json::parse(temp_str);
		if (resp_json.is_object() && resp_json.contains("file_permission_path")) {
			path = QString::fromStdString(resp_json["file_permission_path"]);
		}
	}
	return path;
}


QString RemoteFileUtil::current_user_dirs_;

RemoteFileUtil::RemoteFileUtil() : BaseFileUtil() {

}

void RemoteFileUtil::GetThisPCFiles() {
	GetFiles(tc::kRealRootPath); // "/"
}

void RemoteFileUtil::GetFiles(const QString& path) {
	//FileLogManager::Instance()->AppendLog(QStringLiteral("浏览远端文件列表: ") + path);
	FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_browse_remote_file_list") + path);
	FileSDKInterface::Instance()->GetFilesList(path, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		//LOGI("GetFiles path : {}, resp_code : {}, msg_str : {}, resp_json_data_str : {}", path.toStdString(), resp_code, msg_str, resp_json_data_str);
		QString message = QString::fromStdString(msg_str);
		QString resp_json_data = QString::fromStdString(resp_json_data_str);

		FileContainer file_container;
		g_remote_file_permission_path = tc::GetRespFilePermissionPath(resp_json_data);
		if (!g_remote_file_permission_path.isEmpty()) {
			if ("/" != g_remote_file_permission_path) {
				if (g_remote_file_permission_path.endsWith('/')) {
					g_remote_file_permission_path.chop(1);
				}
			}
			std::cout << "GetFiles g_remote_file_permission_path = " << g_remote_file_permission_path.toStdString() << std::endl;
			emit SigRemoteFilePermissionPath();
		}
		if (resp_code == tc::RespCode::kRespCodeOk) {
			file_container = tc::GetFileContainerFromParseJsonData(resp_json_data);
			file_container.home_ = (tc::kRealRootPath == path);
			emit SigGetFiles(file_container);
			QString error_msg = tc::GetRespDetailErrorMsg(resp_json_data);
			if (!error_msg.isEmpty()) {
				FileLogManager::Instance()->AppendLog(error_msg);
			}
			return;
		}
		QString error_msg = tc::GetRespDetailErrorMsg(resp_json_data); 
		//FileLogManager::Instance()->AppendLog(QStringLiteral("浏览远端文件列表: ") + path + QStringLiteral(" 失败") + message + error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_browse_remote_file_list") + path + tcTr("id_file_trans_log_failed") + message + error_msg);
	});
}

// GetFiles2 仅获取文件列表，但是并不要显示界面上
void RemoteFileUtil::GetFiles2(const QString& path) {
	//FileLogManager::Instance()->AppendLog(QStringLiteral("获取远端文件列表: ") + path);
	FileSDKInterface::Instance()->GetFilesList(path, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		QString message = QString::fromStdString(msg_str);
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		FileContainer file_container;
		if (resp_code == tc::RespCode::kRespCodeOk) {
			file_container = tc::GetFileContainerFromParseJsonData(resp_json_data);
			emit SigGetFiles2(file_container);
			return;
		}
		QString error_msg = tc::GetRespDetailErrorMsg(resp_json_data);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("浏览远端文件列表: ") + path + QStringLiteral(" 失败") + message + error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_browse_remote_file_list") + path + tcTr("id_file_trans_log_failed") + message + error_msg);
	});
}

void RemoteFileUtil::RecursiveGetFiles(const QString& path) {
	FileSDKInterface::Instance()->GetRecursiveFilesList(path, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		QString message = QString::fromStdString(msg_str);
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		FileContainer file_container;
		if (resp_code == tc::RespCode::kRespCodeOk) {
			file_container = tc::GetFileContainerFromParseJsonData(resp_json_data);
			emit SigGetFiles2(file_container);
			return;
		}
		QString error_msg = tc::GetRespDetailErrorMsg(resp_json_data);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("浏览远端文件列表: ") + path + QStringLiteral(" 失败") + message + error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_browse_remote_file_list") + path + tcTr("id_file_trans_log_failed") + message + error_msg);
	});
}

void RemoteFileUtil::BatchCreateFolders(std::vector<QString> folders) {
#if 0
	FileLogManager::Instance()->AppendLog(QStringLiteral("在远端批量创建以下文件夹:"));
	for (auto& path : folders) {
		FileLogManager::Instance()->AppendLog(path);
	}
#endif
//	FileSDKInterface::Instance()->BatchCreateFolders(folders, [=](bool res, const QString& message, const QString& resp_json_data) {
	FileSDKInterface::Instance()->BatchCreateFolders(folders, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		if (tc::RespCode::kRespCodeOk == resp_code) {
			return;
		}
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		QString message = QString::fromStdString(msg_str);
		if (resp_json_data.isEmpty()) {
			//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端批量创建文件夹 失败: ") + message);
			FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_create_folders_in_remotely") + message);
			return;
		}
		std::string temp_str = resp_json_data.toStdString();
		nlohmann::json resp_json = nlohmann::json::parse(temp_str);
		QString detail_error_msg;
		if (resp_json.contains("error_msg")) {
			detail_error_msg = QString::fromStdString(resp_json["error_msg"]);
		}
		//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端批量创建文件夹 失败: ") + message + detail_error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_create_folders_in_remotely") + message + detail_error_msg);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端以下文件夹创建失败: "));
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_failed_create_following_folder_remotely"));
		if (resp_json.contains("no_created_paths")) {
			auto error_created_paths = resp_json["no_created_paths"];
			for (auto& error_created_path : error_created_paths) {
				if (error_created_path.is_object()) {
					if (error_created_path.contains("path")) {
						FileLogManager::Instance()->AppendLog(QString::fromStdString(error_created_path["path"]));
					}
				}
			}	
		}		
	});
}

void RemoteFileUtil::Remove(std::vector<QString> paths) {
	//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端批量移除以下路径:"));
	FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_remove_following_paths_in_remotely"));
	for (auto& path : paths) {
		FileLogManager::Instance()->AppendLog(path);
	}
	//FileSDKInterface::Instance()->Remove(paths, [=](bool res, const QString& message, const QString& resp_json_data) {
	FileSDKInterface::Instance()->Remove(paths, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		if (tc::RespCode::kRespCodeOk == resp_code) {
			emit SigRemove();
			return;
		}
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		QString message = QString::fromStdString(msg_str);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("远端文件夹移除失败:") + message);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_remote_folder_removal_failed") + message);
		if (resp_json_data.isEmpty()) {
			return;
		}
		std::string temp_str = resp_json_data.toStdString();
		nlohmann::json resp_json = nlohmann::json::parse(temp_str);
		auto error_msg = QString::fromStdString(resp_json["error_msg"]);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("以下远端文件或文件夹移除失败:") + error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_removal_following_remote_files_or_folders_failed") + error_msg);
		auto no_del_paths = resp_json["no_del_paths"];
		for (auto& path : no_del_paths) {
			if (path.is_object()) {
				if (path.contains("path")) {
					FileLogManager::Instance()->AppendLog(QString::fromStdString(path["path"]));
				}
			}
		}
		//FileLogManager::Instance()->AppendLog(QStringLiteral("请确保相关文件或文件夹没有被占用"));
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_please_ensure_file_is_not_occupied"));
	});
}

void RemoteFileUtil::ReName(const QString& old_path, const QString& new_name) {
	//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端将") + old_path + QStringLiteral(",重命名为:") + new_name);
	FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_at_the_remote_end") + old_path + tcTr("id_file_trans_log_rename_to") + new_name);
	FileSDKInterface::Instance()->ReName(old_path, new_name, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		QString message = QString::fromStdString(msg_str);
		if (tc::RespCode::kRespCodeOk == resp_code) {
			emit SigReName();
			std::string temp_str = resp_json_data.toStdString();
			nlohmann::json resp_json = nlohmann::json::parse(temp_str);
			QString old_path = QString::fromStdString(resp_json["old_path"]);
			QString new_path = QString::fromStdString(resp_json["new_path"]);
			return;
		}
		QString error_msg = tc::GetRespDetailErrorMsg(resp_json_data);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端重命名失败:") + message + error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_remote_renaming_failed") + message + error_msg);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("请确保相关文件或文件夹没有被占用或者该文件名已经被占用或者目标路径存在"));
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_remote_renaming_failed_reason"));
	});
}

//在parent_path 目录下创建新建文件夹
void RemoteFileUtil::CreateNewFolder(const QString& parent_path) {
	//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端:") + parent_path + QStringLiteral(",目录下新建文件夹"));
	FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_at_the_remote_end") + parent_path + tcTr("id_file_trans_log_create_new_folder_in_dir"));
	FileSDKInterface::Instance()->CreateNewFolder(parent_path, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		QString message = QString::fromStdString(msg_str);
		if (tc::RespCode::kRespCodeOk == resp_code) {
			std::string temp_str = resp_json_data.toStdString();
			nlohmann::json resp_json = nlohmann::json::parse(temp_str);
			QString parent_path = QString::fromStdString(resp_json["parent_path"]);
			QString new_created_path = QString::fromStdString(resp_json["new_created_path"]);
			emit this->SigCreateNewFolder(new_created_path);
			return;
		}
		QString error_msg = tc::GetRespDetailErrorMsg(resp_json_data);
		//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端新建文件夹失败:") + message + error_msg);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_failed_to_create_new_folder_remotely") + message + error_msg);
	});
}

void RemoteFileUtil::Exists(const QString& path) {
	//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端判断此路径是否存在:") + path);
	FileSDKInterface::Instance()->Exists(path, [=](tc::RespCode resp_code, const std::string& msg_str, const std::string& resp_json_data_str) {
		QString resp_json_data = QString::fromStdString(resp_json_data_str);
		QString message = QString::fromStdString(msg_str);
		if (tc::RespCode::kRespCodeOk == resp_code) {
			std::string temp_str = resp_json_data.toStdString();
			nlohmann::json resp_json = nlohmann::json::parse(temp_str);
			std::cout << "resp_json[date]" << resp_json["date"] << std::endl;
			emit SigExists(resp_json["ret"], QString::fromStdString(tc::StringUtil::FormatSize(resp_json["size"])), QDateTime::fromSecsSinceEpoch(resp_json["date"]).toString("yyyy-MM-dd hh:mm:ss"));
			return;
		}
		emit SigExists(false, "", "");// 如果这里不发送信号，就无法触发传输任务，就算res 为false，也发送
		//FileLogManager::Instance()->AppendLog(QStringLiteral("在远端判断路径是否存在失败:") + message);
		FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_failed_to_determine_if_path_exists_remotely") + message);
	});
}

}
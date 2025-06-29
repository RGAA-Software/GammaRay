#include "file_msg_answer_cbk.h"
#include <json/json.hpp>
#include "tc_common_new/log.h"


namespace tc {
	void FileMsgAnswerCbkStructure::Add(const std::shared_ptr<tc::Message>& msg, OnMsgParseRespCallbackFuncType parse_msg_callbck) {
		int seq = send_message_seq_++;
		msg->set_sequence(seq);
		std::lock_guard<std::mutex> lock{mutex_};
		send_msg_resp_callback_map_[seq].send_time = std::chrono::system_clock::now();
		send_msg_resp_callback_map_[seq].callback = [=](const std::shared_ptr<tc::Message>& message) {
			if (!parse_msg_callbck) {
				return;
			}
			std::string data;
			nlohmann::json resp_json;
			do {
				if (tc::kFileOperateRespGetFileList == message->type()) {
					if (!message->has_file_operate_resp_get_file_list()) {
						LOGE("message no has_file_operate_resp_get_file_list().");
						break;
					}
					auto file_infos_list = message->file_operate_resp_get_file_list();
					auto target_path = file_infos_list.path();
					resp_json["target_path"] = target_path;
					auto file_infos = file_infos_list.file_infos();
					resp_json["ret"] = file_infos_list.ret();
					resp_json["error_msg"] = file_infos_list.msg_of_error();
					resp_json["file_permission_path"] = file_infos_list.file_permission_path();
					for (auto file_info : file_infos) {
						nlohmann::json file_info_element;
						file_info_element["file_type"] = file_info.type();
						file_info_element["file_name"] = file_info.name();
						file_info_element["file_path"] = file_info.path();
						file_info_element["file_size"] = file_info.size();
						file_info_element["file_date"] = file_info.date();
						resp_json["file_infos"].push_back(file_info_element);
					}
				}
				else if (tc::kFileOperateRespBatchCreateFolders == message->type()) {
					if (!message->has_file_operate_resp_batch_create_folders()) {
						LOGE("message no has_file_operate_resp_batch_create_folders().");
						break;
					}
					auto resp_batch_create_folders = message->file_operate_resp_batch_create_folders();
					resp_json["error_msg"] = resp_batch_create_folders.msg_of_error();
					auto folders = resp_batch_create_folders.paths_of_no_create_folder();
					for (auto& folder : folders) {
						nlohmann::json folder_element;
						folder_element["path"] = folder;
						resp_json["no_created_paths"].push_back(folder_element);
					}
				}
				else if (tc::kFileOperateRespExists == message->type()) {
					if (!message->has_file_operate_resp_exists()) {
						LOGE("message no has_file_operate_resp_exists().");
						break;
					}
					auto resp_file_exists = message->file_operate_resp_exists();
					resp_json["ret"] = resp_file_exists.ret();
					resp_json["path"] = resp_file_exists.path();
					resp_json["size"] = resp_file_exists.size();
					resp_json["date"] = resp_file_exists.date();
				}
				else if (tc::kFileOperateRespCreateNewFolder == message->type()) {
					if (!message->has_file_operate_resp_create_new_folder()) {
						LOGE("message no has_file_operate_resp_create_new_folder().");
						break;
					}
					auto resp_create_new_folder = message->file_operate_resp_create_new_folder();
					resp_json["ret"] = resp_create_new_folder.ret();
					resp_json["parent_path"] = resp_create_new_folder.path_of_parent();
					resp_json["new_created_path"] = resp_create_new_folder.path_of_new_created();
					resp_json["error_msg"] = resp_create_new_folder.msg_of_error();
				}
				else if (tc::kFileOperateRespRename == message->type()) {
					if (!message->has_file_operate_resp_rename()) {
						LOGE("message no has_file_operate_resp_rename().");
						break;
					}
					auto resp_rename = message->file_operate_resp_rename();
					resp_json["ret"] = resp_rename.ret();
					resp_json["old_path"] = resp_rename.path_of_old();
					resp_json["new_path"] = resp_rename.path_of_new();
					resp_json["error_msg"] = resp_rename.msg_of_error();
				}
				else if (tc::kFileOperateRespDel == message->type()) {
					if (!message->has_file_operate_resp_del()) {
						LOGE("message no has_file_operate_resp_del().");
						break;
					}
					auto resp_del = message->file_operate_resp_del();
					resp_json["ret"] = resp_del.ret();
					auto no_del_paths = resp_del.paths_of_no_del();
					for (auto& path : no_del_paths) {
						nlohmann::json element;
						element["path"] = path;
						resp_json["no_del_paths"].push_back(element);
					}
					resp_json["error_msg"] = resp_del.msg_of_error();
					data = resp_json.dump();
				}
				else {
					//LOGE("message type is: {}", static_cast<int>(message->type()));
					break;
				}
			} while (0);
			data = resp_json.dump();

			parse_msg_callbck(message->resp_code(), message->resp_message().c_str(), data.c_str());
		};
	}
}
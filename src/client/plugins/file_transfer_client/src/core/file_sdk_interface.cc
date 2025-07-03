#include "file_sdk_interface.h"
#include <iostream>
#include <string>
#include <mutex>
#include <qstring.h>
#include <qdatetime.h>
#include "tc_message.pb.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/log.h"
#include "tc_message_new/proto_message_maker.h"
#include "widget/file_detail_info.h"
#include "widget/file_transmit_single_task_manager.h"
#include "widget/file_transmit_single_task.h"
#include "file_trans_interface.h"
#include "file_msg_answer_cbk.h"
#include "file_transmit_sdk.h"
#include "widget/file_log_manager.h"

namespace tc {

FileSDKInterface::FileSDKInterface() : QObject() {
	msg_answer_cbk_t_ = std::make_shared<FileMsgAnswerCbkStructure>();	
}

void FileSDKInterface::InitFileTransSDK(const std::string& device_id, const std::string& stream_id) {
	file_transmit_sdk_ = FileTransmitSDK::Make(device_id, stream_id);
}

void FileSDKInterface::RegSendMessageFunc(SendMessageFuncType&& func) {
	if (file_transmit_sdk_) {
		file_transmit_sdk_->RegSendMessageCallback(std::move(func));
	}
}

void FileSDKInterface::OnMessage(const std::shared_ptr<Message>& msg) {
	switch (msg->type())
	{
	case tc::kFileOperateRespGetFileList:
	case tc::kFileOperateRespBatchCreateFolders:
	case tc::kFileOperateRespCreateNewFolder:
	case tc::kFileOperateRespDel:
	case tc::kFileOperateRespRename:
	case tc::kFileOperateRespExists: {
		file_transmit_sdk_->HandleFileOperateRespMessage(msg);
		break;
	}
	case tc::kFileTransRespUpload:
		if (!msg->has_file_trans_resp_upload()) {
			return;
		}
		file_transmit_sdk_->HandleFileUploadMessage(msg->file_trans_resp_upload());
		break;
	case tc::kFileTransRespDownload:
		if (!msg->has_file_trans_resp_download()) {
			return;
		}
		file_transmit_sdk_->HandleFileDownloadMessage(msg->file_trans_resp_download());
		break;
	case tc::kFileTransDataPacket:
		if (!msg->has_file_trans_data_packet()) {
			return;
		}
		file_transmit_sdk_->HandleFileTransmitDataPacket(msg->file_trans_data_packet());
		break;
	case tc::kFileTransDataPacketResponse:
		if (!msg->has_file_trans_data_packet_response()) {
			return;
		}
		file_transmit_sdk_->HandleFileTransDataPacketResponse(msg->file_trans_data_packet_response());
		break;
	default:
		break;
	}
}

void FileSDKInterface::GetFilesList(const QString& path, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	std::string path_str = path.toStdString();
	file_transmit_sdk_->GetFilesList(path_str, std::move(callback));
	RegisterFileOperateCallback();
}

void FileSDKInterface::GetRecursiveFilesList(const QString& path, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	std::string path_str = path.toStdString();
	file_transmit_sdk_->RecursiveGetFilesList(path_str, std::move(callback));
	RegisterFileOperateCallback();
}

void FileSDKInterface::BatchCreateFolders(std::vector<QString> folders, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	QString folder_join;
	for (auto& folder : folders) {
		folder_join = folder_join + folder + path_split_;
	}
	std::string folder_join_str = folder_join.toStdString();
	file_transmit_sdk_->BatchCreateFolders(folder_join_str, std::move(callback));
}

void FileSDKInterface::ReName(const QString& old_path, const QString& new_name, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	std::string old_path_str = old_path.toStdString();
	std::string new_name_str = new_name.toStdString();
	file_transmit_sdk_->Rename(old_path_str, new_name_str, std::move(callback));
}

void FileSDKInterface::Exists(const QString& file_path, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	std::string path = file_path.toStdString();
	file_transmit_sdk_->Exists(path, std::move(callback));
}

void FileSDKInterface::CreateNewFolder(const QString& parent_path, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	std::string parent_path_str = parent_path.toStdString();
	file_transmit_sdk_->CreateNewFolder(parent_path_str, std::move(callback));
}

void FileSDKInterface::Remove(std::vector<QString> paths, OnFileOperateCallbackFuncType&& callback) {
	if (!file_transmit_sdk_) {
		return;
	}
	QString path_join;
	for (auto& path : paths) {
		path_join = path_join + path + path_split_;
	}
	std::string path_join_str = path_join.toStdString();
	file_transmit_sdk_->Remove(path_join_str, std::move(callback));
}

void FileSDKInterface::AbortTransmitTask(const QString& task_id) {
	if (!file_transmit_sdk_) {
		return;
	}
	std::string task_id_str = task_id.toStdString();
	file_transmit_sdk_->AbortFileTransmit(task_id_str);
}

void FileSDKInterface::RegisterFileOperateCallback() {
	static std::once_flag flag;
	std::call_once(flag, [=]() {
		file_transmit_sdk_->RegOnFileUploadEndCallback(std::move([=](ETcFileTransmitState state, std::string task_id) {
			if (ETcFileTransmitState::kSuccess== state) {
				std::cout << "emit on file upload end kSuccess" << std::endl;
				emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kSuccess, EFileTransmitTaskErrorCause::kPlaceholder);

                // upload success callback
                if (upload_end_cbk_) {
                    upload_end_cbk_(task_id, "", true);
                }
			}
			else {
				if (ETcFileTransmitState::kRemoteFileOpenFailed == state) {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kRemoteFileFailedOpen);
				}
				else if (ETcFileTransmitState::kVerifyError == state) {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kVerifyError);
				}
				else if (ETcFileTransmitState::kFileWriteFailed == state) {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileFailedWrite);
				}
				else if (ETcFileTransmitState::kUnknowError == state) {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kUnKnown);
				}
				else if (ETcFileTransmitState::kPacketLoss == state) {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kPacketLoss);
				}
				else if (ETcFileTransmitState::kCreateFolderFailed == state) {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kDirFailedCreate);
				}
				else {
					emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kUnKnown);
				}

                // upload failed callback
                if (upload_end_cbk_) {
                    upload_end_cbk_(task_id, "", false);
                }
			}
		}));

		file_transmit_sdk_->RegOnFileDownloadCallback(std::move([=](ETcFileTransmitState state, std::string task_id, uint64_t download_size, uint64_t file_size) {
			QString task_id_qstr = QString::fromStdString(task_id);

			std::optional<bool> ended_res = FileTransmitSingleTaskManager::Instance()->IsEnded(task_id_qstr);
			if (!ended_res.has_value()) {
				LOGW("can not find task id: {}", task_id_qstr.toStdString());
				return;
			}
			bool ended = ended_res.value();
			if (ended) {
				std::cout << task_id << " IsEnded = " << true << std::endl;
				return;
			}
			auto task_ptr = FileTransmitSingleTaskManager::Instance()->GetTaskById(task_id_qstr);
			if (task_ptr == nullptr) {
				LOGI("Task is null, id: {}", task_id);
				return;
			}
			task_ptr->already_transmit_file_size_ = download_size;

			if (task_ptr->is_delete_) {
				emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kDelete);
				return;
			}
			// 下载进度
			if (ETcFileTransmitState::kDownloadProcess == state) {
				static uint64_t last_time = tc::TimeUtil::GetCurrentTimestamp();
				auto current_time = tc::TimeUtil::GetCurrentTimestamp();
				if (current_time - last_time <= 1000) {
					return;
				}
				last_time = current_time;
				if (file_size == 0) {
					task_ptr->progress_ = 100;
				}
				else {
					task_ptr->progress_ = (int8_t)((download_size * 1.0) / (file_size * 1.0) * 100);
				}
				if (task_ptr->progress_ > 100) {
					task_ptr->progress_ = 100;
				}
				emit this->SigDownloadTransmitTaskProgress(task_id_qstr, task_ptr->progress_);
			}
			// 下载成功
			else if (ETcFileTransmitState::kSuccess == state) {
				printf("ETcFileTransmitState::kSUCCESS \n");
				emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kSuccess, EFileTransmitTaskErrorCause::kPlaceholder);

                if (download_end_cbk_) {
                    download_end_cbk_(task_id, "", true);
                }
			}
            else {
                // 下载失败
                if (ETcFileTransmitState::kFileNoneExist == state) {
                    printf("ETcFileTransmitState::kFileNoneExist \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileNotExists);

                } else if (ETcFileTransmitState::kRemoteFileOpenFailed == state) {
                    printf("ETcFileTransmitState::kRemoteFileOpenFailed \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kRemoteFileFailedOpen);

                } else if (ETcFileTransmitState::kUnknowError == state) {
                    printf("ETcFileTransmitState::kUnknowError \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kUnKnown);

                } else if (ETcFileTransmitState::kCreateFolderFailed == state) {
                    printf("ETcFileTransmitState::kCreateFolderFailed \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kDirFailedCreate);

                } else if (ETcFileTransmitState::kFileOpenFailed == state) {
                    printf("ETcFileTransmitState::kFileOpenFailed \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileFailedOpen);

                } else if (ETcFileTransmitState::kFileWriteFailed == state) {
                    printf("ETcFileTransmitState::kFileWriteFailed \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileFailedWrite);

                } else if (ETcFileTransmitState::kVerifyError == state) {
                    printf("ETcFileTransmitState::kVerifyError \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kVerifyError);

                } else if (ETcFileTransmitState::kFileReadFailed == state) {
                    printf("ETcFileTransmitState::kFileReadFailed \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kFileFailedRead);

                } else if (ETcFileTransmitState::kPacketLoss == state) {
                    printf("ETcFileTransmitState::kPacketLoss \n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kPacketLoss);

                } else {
                    printf("TC_FILE_TRANSMIT_RES kUnKnown\n");
                    emit this->SigTransmitTaskEndRes(QString::fromUtf8(task_id), EFileTransmitTaskState::kError, EFileTransmitTaskErrorCause::kUnKnown);
                }

                if (download_end_cbk_) {
                    download_end_cbk_(task_id, "", false);
                }
            }
		}));
	});
}

void FileSDKInterface::UploadFile(std::string local_file_path, std::string remote_file_path, std::string task_id, UploadFileCallbackFuncType&& upload_callback) {
	if (!file_transmit_sdk_) {
		return;
	}
    if (upload_beg_cbk_) {
        upload_beg_cbk_(task_id, local_file_path);
    }
	file_transmit_sdk_->UploadFile(local_file_path, remote_file_path, task_id, [=, this](ETcFileTransmitState state, uint64_t upload_size, uint64_t file_size) {
        upload_callback(state, upload_size, file_size);

        if (state == ETcFileTransmitState::kUploadReadEnd) {
            // 读取结束，可能并不成功，需要看上面的回调
        }
        else if (state == ETcFileTransmitState::kUploadProcess ) {
            // 读取进度
        }
        else {
            // 错误
            if (upload_end_cbk_) {
                upload_end_cbk_(task_id, local_file_path, false);
            }
        }
    });
}

void FileSDKInterface::DownloadFile(std::string local_file_path, std::string remote_file_path, std::string task_id) {
	if (!file_transmit_sdk_) {
		return;
	}
	file_transmit_sdk_->DownloadFile(local_file_path, remote_file_path, task_id);

    // 回调下载开始
    if (download_beg_cbk_) {
        download_beg_cbk_(task_id, remote_file_path);
    }
}

void FileSDKInterface::SetOnFileUploadBeginCallback(OnFileUploadBeginCallback&& cbk) {
    upload_beg_cbk_ = cbk;
}

void FileSDKInterface::SetOnFileUploadEndCallback(OnFileUploadEndCallback&& cbk) {
    upload_end_cbk_ = cbk;
}

void FileSDKInterface::SetOnFileDownloadBeginCallback(OnFileDownloadBeginCallback&& cbk) {
    download_beg_cbk_ = cbk;
}

void FileSDKInterface::SetOnFileDownloadEndCallback(OnFileDownloadEndCallback&& cbk) {
    download_end_cbk_ = cbk;
}


}
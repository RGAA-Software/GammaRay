#pragma once
#include <string>
#include <functional>
#include <memory>
#include <qstring.h>
#include "tc_message.pb.h"

namespace tc {

enum class ETcFileTransmitState
{
	kSuccess,
	kVerifyError, // 普通校验，仅仅比对文件大小 校验失败
	kMd5VerifyError, // md5 校验失败
	kSignVerificationFails,
	kUnknowError,
	kPathEmpty,
	kGetFileNameError,
	kFileNoneExist,
	kUploadProcess,
	kDownloadProcess,
	kRemoteFileOpenFailed, //远端文件打开失败
	kFileOpenFailed,
	kFileReadFailed,
	kFileWriteFailed, // 写文件出现异常
	kUploadReadEnd,   // 上传文件读取结束，至于是否成功，还要看客户端返回值
	kUploadSaveFileParentDirCreateFailed, //上传文件时候，要保存文件的父目录创建失败
	kUploadSaveFileSizeFailed,  //上传文件完成后，文件大小与源文件不一样
	kCreateFolderFailed,  //创建文件夹失败
	kRemoveFailed, //删除文件出错
	kDownloadTaskNotFound, //下载任务没找到
	kFileReadOnly,         //文件是只读的，上传文件时候，如果要覆盖文件，会失败
	kDownloadSaveFileSizeFailed, // 下载文件完成后，文件大小与源文件不一样
	kCanNotFoundRespCode, //没有返回值
	kUserCancel, //用户取消
	kTImeOut,     //超时
	kPacketLoss, //丢包
};

using OnFileOperateCallbackFuncType = std::function<void(tc::RespCode code, const std::string& message, const std::string& resp_json_data)>;

using UploadFileCallbackFuncType = std::function<void(ETcFileTransmitState, uint64_t upload_size, uint64_t file_size)>; //上传过程中的进度回调

using OnFileUploadEndCallbackFunc = std::function<void(ETcFileTransmitState, std::string task_id)>;

using OnFileDownloadCallbackFunc = std::function<void(ETcFileTransmitState, std::string task_id, uint64_t, uint64_t)>;

using SendMessageFuncType = std::function<bool(std::shared_ptr<tc::Message>)>;

// 回调给客户端上传下载文件的基本状态
using OnFileUploadBeginCallback = std::function<void(const std::string& task_id, const std::string& path)>;
using OnFileUploadEndCallback = std::function<void(const std::string& task_id, const std::string& path, bool success)>;
using OnFileDownloadBeginCallback = std::function<void(const std::string& task_id, const std::string& path)>;
using OnFileDownloadEndCallback = std::function<void(const std::string& task_id, const std::string& path, bool success)>;

}
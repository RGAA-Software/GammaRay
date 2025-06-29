#pragma once
#include <qmetatype.h>

namespace tc {

enum class EFileTransmitTaskState {
	kSuccess,       // 成功
	kWait,          // 等待
	KTransmitting,  // 传输中
	kError,         // 传输失败
	kVerifying,     // 校验中
	kDelete,        // 用户删除
};
Q_DECLARE_METATYPE(tc::EFileTransmitTaskState);

enum class EFileTransmitTaskErrorCause {
	kPlaceholder,   // 占位符
	kUnKnown,       // 未知
	kCancel,        // 用户取消
	kSkip,          // 用户跳过
	kDelete,        // 用户删除
	kMd5VerifyError,// MD5 校验失败
	kSignVerificationFails, // 签名校验失败
	kFileNotExists, //文件不存在
	kFileFailedOpen, //文件打开失败
	kFileFailedRead, //文件读取失败
	kFileReadEnd, //文件读取结束
	kDirFailedCreate, //创建文件夹失败
	kRemoteFileFailedOpen, //远端文件打开失败
	kFileFailedWrite,
	kVerifyError,// 文件大小 校验失败
	kTimeOut,        // 超时
	kTCcaException,      //流路异常
	kPacketLoss, //网络丢包

};
Q_DECLARE_METATYPE(tc::EFileTransmitTaskErrorCause);

enum class EFileTransmitTaskType {
	kUnKnown,
	kUpload,
	kDownload,
};
Q_DECLARE_METATYPE(tc::EFileTransmitTaskType);

}
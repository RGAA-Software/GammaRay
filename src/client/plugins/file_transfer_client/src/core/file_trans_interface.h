#pragma once
#include <string>
#include <memory>
#include <QWidget>
#include "file_transfer_client/src/common/file_trans_def.h"

namespace tc
{

    #ifdef TC_FILE_TRANS_DLL_EXPORTS
    #define TC_FILE_TRANS_DLL_API __declspec(dllexport)
    #else
    #define TC_FILE_TRANS_DLL_API __declspec(dllimport)
    #endif

    class Data;
    class Message;
    class FileTransWidget;
    class FileTransferPlugin;

    class TC_FILE_TRANS_DLL_API FileTransInterface {
    public:
        explicit FileTransInterface(FileTransferPlugin* plugin);
        ~FileTransInterface();

        void ShowFileTrans();

        void OnProtoMessage(std::shared_ptr<Message> msg);
        void SendProtoMessage(std::shared_ptr<Data> msg);
        void Exit();
        bool HasTransTask();

        // 设置基本状态回调
        void SetOnFileUploadBeginCallback(OnFileUploadBeginCallback&& cbk);
        void SetOnFileUploadEndCallback(OnFileUploadEndCallback&& cbk);
        void SetOnFileDownloadBeginCallback(OnFileDownloadBeginCallback&& cbk);
        void SetOnFileDownloadEndCallback(OnFileDownloadEndCallback&& cbk);

        QWidget* GetFileTransWidget();

    private:
        FileTransferPlugin* plugin_ = nullptr;
        FileTransWidget* file_trans_widget_ = nullptr;
    };

}
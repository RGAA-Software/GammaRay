//
// Created by RGAA on 9/07/2024.
//

#ifndef GAMMARAYPC_FILE_TRANSFER_H
#define GAMMARAYPC_FILE_TRANSFER_H

#include <memory>
#include <iostream>
#include <chrono>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <asio2/websocket/ws_client.hpp>
#include <QString>

namespace tc
{

    class FsFile;
    class FsFolder;
    class Thread;
    class ClientContext;
    class FileSystemObject;

    class FileTransferChannel {
    public:
        explicit FileTransferChannel(const std::shared_ptr<ClientContext>& ctx);
        void Start();
        void Exit();
        bool IsConnected();
        void SendFiles(const std::vector<QString>& files_path);
    private:
        void SendFile(const std::shared_ptr<FsFile>& file);
        void RequestSendingFile(const std::shared_ptr<FsFile>& file);
        void ParseRespMessage(std::string_view data);
        void CompleteSending(const std::shared_ptr<FsFile>& file);
        void InfoDialog(const QString& msg);
        void ErrorDialog(const QString& msg);

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<asio2::ws_client> client_ = nullptr;
        std::shared_ptr<Thread> sender_thread_ = nullptr;
        bool stop_sending_ = false;
        std::mutex send_mtx_;
        std::condition_variable send_cv_;
        std::atomic_bool continue_sending_;
    };

}

#endif //GAMMARAYPC_FILE_TRANSFER_H

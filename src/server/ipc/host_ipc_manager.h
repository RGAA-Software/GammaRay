//
// Created by RGAA on 2023/12/23.
//

#ifndef TC_APPLICATION_IPCMANAGER_H
#define TC_APPLICATION_IPCMANAGER_H

#include <memory>
#include <thread>
#include <memory>
#include <functional>

#include <Poco/NamedEvent.h>
#include <Poco/SharedMemory.h>
#include <Poco/NamedMutex.h>

#include "tc_capture_new/capture_message.h"

namespace tc
{

    constexpr auto kHostToClientShmSize = 2 * 1024 * 1024;

    class Data;
    class Settings;

    struct FixHeader {
        uint32_t buffer_length = 0;
        uint32_t buffer_index = 0;
        uint64_t buffer_timestamp = 0;
        // data below
    };

    using IpcMessageCallback = std::function<void(const std::shared_ptr<CaptureBaseMessage>&, const std::shared_ptr<Data>&)>;
    using IpcVideoFrameCallback = std::function<void(const std::shared_ptr<CaptureVideoFrame>&, const std::shared_ptr<Data>&)>;

    class HostIpcManager {
    public:

        static std::shared_ptr<HostIpcManager> Make(uint32_t pid);

        explicit HostIpcManager(uint32_t pid);
        ~HostIpcManager();

        void Send(const std::string& data);
        void Send(const char* data, int size);
        void Send(const std::shared_ptr<Data>& data);
        void WaitForMessage();
        void RegisterVideoFrameCallback(IpcVideoFrameCallback&& cbk);
        void Exit();
        bool Ready();

        void MockSend();

    private:
        std::tuple<std::shared_ptr<CaptureBaseMessage>, std::shared_ptr<Data>> ParseCaptureMessage(const char* data, int size);
        void CallbackMessage(std::shared_ptr<CaptureBaseMessage>&, const std::shared_ptr<Data>&);

    private:
        uint32_t pid_;
        Settings* settings_ = nullptr;
        bool exit_ = false;
        bool init_ = false;
        uint32_t buffer_index_ = 0;

        std::shared_ptr<Poco::NamedEvent> client_to_host_event_ = nullptr;
        std::shared_ptr<Poco::SharedMemory> client_to_host_shm_ = nullptr;
        std::shared_ptr<Poco::NamedMutex> client_to_host_mtx_ = nullptr;

        std::shared_ptr<Poco::NamedEvent> host_to_client_event_ = nullptr;
        std::shared_ptr<Poco::SharedMemory> host_to_client_shm_ = nullptr;
        std::shared_ptr<Poco::NamedMutex> host_to_client_mtx_ = nullptr;

        std::shared_ptr<std::thread> recv_thread_ = nullptr;

        IpcVideoFrameCallback video_frame_callback_;

    };

}

#endif //TC_APPLICATION_IPCMANAGER_H

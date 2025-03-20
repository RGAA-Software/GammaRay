//
// Created by RGAA on 2023/12/23.
//

#include "host_ipc_manager.h"

#include "settings/rd_settings.h"
#include "tc_common_new/ipc_shm.h"
#include "tc_common_new/ipc_msg_queue.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/time_ext.h"

namespace tc
{

    std::shared_ptr<HostIpcManager> HostIpcManager::Make(uint32_t pid) {
        return std::make_shared<HostIpcManager>(pid);
    }

    HostIpcManager::HostIpcManager(uint32_t pid) {
        pid_ = pid;
        settings_ = RdSettings::Instance();
        auto id = pid;
        auto ipc_shm_host_to_client_name = "ipc_shm_host_to_client_" + std::to_string(id);

        auto ipc_event_host_to_client_name = "ipc_event_host_to_client_" + std::to_string(id);
        auto ipc_event_client_to_host_name = "ipc_event_client_to_host_" + std::to_string(id);

        auto mtx_host_to_client_name = "mtx_host_to_client_" + std::to_string(id);
        auto mtx_client_to_host_name = "mtx_client_to_host_" + std::to_string(id);

//        try {
//            host_to_client_event_ = std::make_shared<Poco::NamedEvent>(ipc_event_host_to_client_name);
//            client_to_host_event_ = std::make_shared<Poco::NamedEvent>(ipc_event_client_to_host_name);
//
//            host_to_client_shm_ = std::make_shared<Poco::SharedMemory>(ipc_shm_host_to_client_name,
//                                                                       kHostToClientShmSize,
//                                                                       Poco::SharedMemory::AccessMode::AM_WRITE);
//
//            host_to_client_mtx_ = std::make_shared<Poco::NamedMutex>(mtx_host_to_client_name);
//            client_to_host_mtx_ = std::make_shared<Poco::NamedMutex>(mtx_client_to_host_name);
//        } catch (std::exception& e) {
//            LOGE("HostIpcManager failed....: {}", e.what());
//            return;
//        }
        init_ = true;
    }

    HostIpcManager::~HostIpcManager() {

    }

    void HostIpcManager::Send(const char* data, int size) {
        if (!Ready()) {
            LOGE("HostIpcManager not ready...");
            return;
        }
        if (size > kHostToClientShmSize) {
            LOGE("Host to client overflow!");
            return;
        }

//        host_to_client_mtx_->lock();
//        auto header = FixHeader {
//            .buffer_length = static_cast<uint32_t>(size),
//            .buffer_index = buffer_index_++,
//            .buffer_timestamp = TimeExt::GetCurrentTimestamp(),
//        };
//        auto begin = host_to_client_shm_->begin();
//        memcpy(begin, (char*)&header, sizeof(FixHeader));
//        memcpy(begin + sizeof(FixHeader), data, size);
//        host_to_client_mtx_->unlock();
//
//        host_to_client_event_->set();
    }

    void HostIpcManager::Send(const std::shared_ptr<Data>& data) {
        this->Send(data->CStr(), data->Size());
    }

    void HostIpcManager::Send(const std::string& data) {
        this->Send(data.c_str(), data.size());
    }

    static uint64_t last_recv_timestamp_ = TimeExt::GetCurrentTimestamp();

    void HostIpcManager::WaitForMessage() {
//        auto wait_task = [=, this]() {
//            while (!exit_) {
//                client_to_host_event_->wait();
//
//                auto current_timestamp = TimeExt::GetCurrentTimestamp();
//                auto diff = current_timestamp - last_recv_timestamp_;
//                last_recv_timestamp_ = current_timestamp;
//
//                if (!client_to_host_shm_) {
//                    auto ipc_shm_client_to_host_name = "ipc_shm_client_to_host_" + std::to_string(pid_);
//                    auto client_to_host_buffer_size = settings_->GetShmBufferSize();
//                    client_to_host_shm_ = std::make_shared<Poco::SharedMemory>(ipc_shm_client_to_host_name, client_to_host_buffer_size, Poco::SharedMemory::AccessMode::AM_READ);
//                    LOGI("Client to Host buffer size : {}", client_to_host_buffer_size);
//                }
//
//                client_to_host_mtx_->lock();
//                auto begin = client_to_host_shm_->begin();
//                auto header = (FixHeader*)begin;
//                auto buffer_begin = begin + sizeof(FixHeader);
//                auto msg_tuple = ParseCaptureMessage(buffer_begin, (int)header->buffer_length);
//                client_to_host_mtx_->unlock();
//
//                auto msg = std::get<0>(msg_tuple);
//                auto buffer = std::get<1>(msg_tuple);
//                this->CallbackMessage(msg, buffer);
//
//                //LOGI("Recv msg: {}, buffer index: {}, gap diff: {}, time diff: {}", msg->type, header->buffer_index, diff, (TimeExt::GetCurrentTimePointUS() - header->buffer_timestamp));
//                if (buffer) {
//                    LOGI("Recv buffer size: {}", buffer->Size());
//                }
//            }
//        };
//
//        recv_thread_ = std::make_shared<std::thread>(wait_task);
    }

    // data -> Msg header + Msg body
    // size -> sizeof(Msg header) + sizeof(Msg body)
    std::tuple<std::shared_ptr<CaptureBaseMessage>, std::shared_ptr<Data>> HostIpcManager::ParseCaptureMessage(const char* data, int size) {
        auto msg = (CaptureBaseMessage*)data;

        std::shared_ptr<CaptureBaseMessage> cpy_msg = nullptr;
        std::shared_ptr<Data> cpy_data = nullptr;
        if (msg->type_ == kCaptureVideoFrame) {
            cpy_msg = std::make_shared<CaptureVideoFrame>();
            memcpy(cpy_msg.get(), msg, sizeof(CaptureVideoFrame));
            if (cpy_msg->data_length > 0 && cpy_msg->data_length < size) {
                cpy_data = Data::Make(data + sizeof(CaptureVideoFrame), (int) cpy_msg->data_length);
            }
            auto frame = std::static_pointer_cast<CaptureVideoFrame>(cpy_msg);
            //LOGI("Video in : {}, data length: {}, size: {}", (cpy_msg)->type, cpy_msg->data_length, size);
            //LOGI("VideoFrame, {}x{}, idx: {}, uid: {}", frame->frame_width_, frame->frame_height_, frame->frame_index_, frame->adapter_uid_);
        }
        else if (msg->type_ == kCaptureAudioFrame) {
            cpy_msg = std::make_shared<CaptureAudioFrame>();
            memcpy(cpy_msg.get(), msg, sizeof(CaptureAudioFrame));
            if (cpy_msg->data_length > 0 && cpy_msg->data_length < size) {
                cpy_data = Data::Make(data + sizeof(CaptureAudioFrame), (int) cpy_msg->data_length);
            }
            LOGI("Audio in : {}", std::static_pointer_cast<CaptureAudioFrame>(cpy_msg)->type_);
        }
        else if (msg->type_ == kCaptureDebugInfo) {
            cpy_msg = std::make_shared<CaptureDebugInfo>();
            memcpy(cpy_msg.get(), msg, sizeof(CaptureDebugInfo));
        }

        return {cpy_msg, cpy_data};
    }

    void HostIpcManager::CallbackMessage(std::shared_ptr<CaptureBaseMessage>& msg, const std::shared_ptr<Data>& buffer) {
        if (msg->type_ == kCaptureVideoFrame) {
            auto target_msg = std::static_pointer_cast<CaptureVideoFrame>(msg);
            if (this->video_frame_callback_) {
                this->video_frame_callback_(target_msg, buffer);
            }
        }
    }

    void HostIpcManager::RegisterVideoFrameCallback(tc::IpcVideoFrameCallback &&cbk) {
        this->video_frame_callback_ = std::move(cbk);
    }

    void HostIpcManager::MockSend() {
//        std::thread t([this] () {
//            for (int i = 0; i < 1000000; i++) {
//                auto msg = std::format("host => client : {} ....ok.", i);
//                this->Send(msg);
//                std::this_thread::sleep_for(std::chrono::milliseconds(16));
//            }
//        });
//        t.detach();
    }

    void HostIpcManager::Exit() {
        exit_ = true;
        if (recv_thread_ && recv_thread_->joinable()) {
            recv_thread_->join();
        }
    }

    bool HostIpcManager::Ready() {
//        return init_ && host_to_client_event_ && host_to_client_shm_;
        return false;
    }

}
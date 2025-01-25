//
// Created by RGAA on 25/01/2025.
//

#include "gr_running_pipe.h"
#include "tc_common_new/log.h"
#include <Windows.h>

namespace tc
{

    const char* kPipeName = R"(\\.\pipe\running\render_panel)";
    const char* kPipSignal = R"(Good)";

    GrRunningPipe::GrRunningPipe() {

    }

    GrRunningPipe::~GrRunningPipe() {
        if (recv_handle_ != INVALID_HANDLE_VALUE) {
            exit_receiving_ = true;
            DisconnectNamedPipe(recv_handle_);
            CloseHandle(recv_handle_);
        }
    }

    void GrRunningPipe::StartListening(std::function<void()>&& cbk) {
        recv_thread_ = std::make_shared<std::thread>([=, this]() {
            recv_handle_ = CreateNamedPipeA(kPipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                            1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, NULL);
            if (recv_handle_ != INVALID_HANDLE_VALUE) {
                while(!exit_receiving_) {
                    if (!ConnectNamedPipe(recv_handle_, NULL)) {
                        LOGE("ConnectNamedPipe failed: {}", kPipeName);
                        return;
                    }

                    char buffer[1024] = {0};
                    DWORD length = 0;
                    const int wanted_length = 1024;
                    if (!ReadFile(recv_handle_, buffer, wanted_length, &length, nullptr)) {
                        LOGI("Read from {} failed, error: {}", kPipeName, GetLastError());
                        break;
                    }
                    LOGI("Read: {}", buffer);
                    if (std::string(buffer) == kPipSignal) {
                        cbk();
                    }
                    DisconnectNamedPipe(recv_handle_);
                }
            } else {
                LOGE("Create pipe {} failed!", kPipeName);
            }
        });
    }

    bool GrRunningPipe::SendHello() {
        HANDLE handle = CreateFileA(kPipeName, GENERIC_READ | GENERIC_WRITE,
                                                  0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if(handle == INVALID_HANDLE_VALUE){
            LOGE("Invalid handle, open: {} failed", kPipeName);
            return false;
        }

        DWORD wrote_num;
        std::string msg = kPipSignal;
        if(!WriteFile(handle, msg.c_str(), msg.size(), &wrote_num, nullptr)){
            LOGE("Wrote to {} failed, error: {}", kPipeName, GetLastError());
            return false;
        }
        return true;
    }

}
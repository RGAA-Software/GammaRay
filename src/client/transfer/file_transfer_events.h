//
// Created by RGAA on 17/07/2024.
//

#ifndef GAMMARAYPC_FILE_TRANSFER_EVENTS_H
#define GAMMARAYPC_FILE_TRANSFER_EVENTS_H

#include <string>

namespace tc
{

    class EvtFileTransferReady {
    public:
        std::string id_;
        std::string name_;
        std::string path_;
        uint64_t total_size_;
        uint64_t timestamp_;
    };

    class EvtFileTransferring {
    public:
        std::string id_;
        std::string name_;
        std::string path_;
        float progress_;
        uint64_t total_size_;
        uint64_t transferred_size_;
        uint64_t timestamp_;
    };

    class EvtFileTransferFailed {
    public:
        std::string id_;
        std::string name_;
        std::string path_;
        uint64_t timestamp_;
    };

    class EvtFileTransferDeleteFailed {
    public:
        std::string id_;
        std::string name_;
        std::string path_;
        uint64_t timestamp_;
    };

    class EvtFileTransferSuccess {
    public:
        std::string id_;
        std::string name_;
        std::string path_;
        uint64_t timestamp_;
        uint64_t total_size_;
    };

}

#endif //GAMMARAYPC_FILE_TRANSFER_EVENTS_H

//
// Created by RGAA on 8/04/2025.
//

#ifndef GAMMARAY_CP_FILE_STREAM_H
#define GAMMARAY_CP_FILE_STREAM_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <QFile>
#include <QFileInfo>
#include "cp_data_object.h"
#include "cp_file_struct.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"

namespace tc
{

    class ClipboardPlugin;

    class CpFileStream : public IStream {
    public:

        CpFileStream(ClipboardPlugin* plugin, const ClipboardFileWrapper& fw);

        virtual ~CpFileStream() {

        }

        HRESULT QueryInterface(REFIID riid, void **ppvObject) override;

        ULONG AddRef() override {
            return InterlockedIncrement(&ref_);
        }

        ULONG Release() override {
            ULONG newRef = InterlockedDecrement(&ref_);
            if (newRef == 0) {
                delete this;
            }
            return newRef;
        }

        HRESULT SetSize(ULARGE_INTEGER size) override {
            LOGI("SetSize: {}, low: {}, high: {}", size.QuadPart, size.LowPart, size.HighPart);
            return E_NOTIMPL;
        }

        HRESULT CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) override {
            LOGI("CopyTo");
            return E_NOTIMPL;
        }

        HRESULT Commit(DWORD) override {
            LOGI("Commit");
            return E_NOTIMPL;
        }

        HRESULT Revert(void) override {
            LOGI("Revert");
            return E_NOTIMPL;
        }

        HRESULT LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override {
            LOGI("LockRegion");
            return E_NOTIMPL;
        }

        HRESULT UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override {
            LOGI("UnLockRegion");
            return E_NOTIMPL;
        }

        HRESULT Clone(IStream **) override {
            return E_NOTIMPL;
        }

        HRESULT Read(void *pv, ULONG cb, ULONG *pcbRead) override;

        HRESULT Write(const void *pv, ULONG cb, ULONG *pcbWritten) override {
            return S_OK;
        }

        HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* new_pos) override;

        HRESULT WINAPI Stat(STATSTG *pstatstg, DWORD grfStatFlag) override;

        void OnClipboardRespBuffer(const ClipboardRespBuffer& rb);
        void Exit();
        std::string GetFileId();
        std::string GetDeviceId();
        std::string GetFileName();

    private:
        ClipboardPlugin* plugin_ = nullptr;
        LONG ref_;
        uint64_t file_size_ {0};
        std::atomic_int64_t current_position_ = 0;
        std::atomic_int64_t req_index_ = 0;
        ClipboardFileWrapper cp_file_;
        std::string gen_file_id_;
        std::atomic_bool exit_ = false;
        std::mutex wait_data_mtx_;
        std::condition_variable data_cv_;

        std::optional<ClipboardRespBuffer> resp_buffer_;

    };


}

#endif //GAMMARAY_CP_FILE_STREAM_H

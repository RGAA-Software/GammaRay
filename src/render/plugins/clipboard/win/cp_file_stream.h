//
// Created by RGAA on 8/04/2025.
//

#ifndef GAMMARAY_CP_FILE_STREAM_H
#define GAMMARAY_CP_FILE_STREAM_H

#include <cstdint>
#include <memory>
#include <QFile>
#include <QFileInfo>
#include "cp_data_object.h"
#include "cp_file_struct.h"

namespace tc
{

    class CpFileStream : public IStream {
    public:

        CpFileStream(const FileDetailInfo& info) : ref_(1) {
            mFileDetailInfo = info;
            file_ = std::make_shared<QFile>(info.mRemotePath);
            file_->open(QIODeviceBase::OpenModeFlag::ReadOnly);

            QFileInfo file_info(info.mRemotePath);

            mFileDetailInfo.mFileSize = file_->size();
            mFileDetailInfo.mFileName = file_->fileName();

            target_file_ = std::make_shared<QFile>(std::format("D:\\360Downloads\\z_{}", file_info.fileName().toStdString()).c_str());
            target_file_->open(QIODeviceBase::OpenModeFlag::WriteOnly);
        }

        virtual ~CpFileStream() {

        }

        HRESULT QueryInterface(REFIID riid, void **ppvObject);

        ULONG AddRef() {
            return InterlockedIncrement(&ref_);
        }

        ULONG Release() {
            ULONG newRef = InterlockedDecrement(&ref_);
            if (newRef == 0) {
                delete this;
            }

            return newRef;
        }

        virtual HRESULT SetSize(ULARGE_INTEGER) {
            return E_NOTIMPL;
        }

        virtual HRESULT CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *,
                               ULARGE_INTEGER *) {
            return E_NOTIMPL;
        }

        virtual HRESULT Commit(DWORD) {
            return E_NOTIMPL;
        }

        virtual HRESULT Revert(void) {
            return E_NOTIMPL;
        }

        virtual HRESULT LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
            return E_NOTIMPL;
        }

        virtual HRESULT UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
            return E_NOTIMPL;
        }

        virtual HRESULT Clone(IStream **) {
            return E_NOTIMPL;
        }

        virtual HRESULT Read(void *pv, ULONG cb, ULONG *pcbRead);

        virtual HRESULT Write(const void *pv, ULONG cb, ULONG *pcbWritten) {
            return S_OK;
        }

        virtual HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);

        virtual HRESULT WINAPI Stat(STATSTG *pstatstg, DWORD grfStatFlag);


    private:
        LONG ref_;
        uint64_t file_size_ {0};
        uint64_t current_position_ {0};
        std::shared_ptr<QFile> file_ = nullptr;
        std::shared_ptr<QFile> target_file_ = nullptr;
    public:
        FileDetailInfo mFileDetailInfo;
    };


}

#endif //GAMMARAY_CP_FILE_STREAM_H

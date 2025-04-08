#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "DataObject.h"
#include <QString>
#include <QFile>
#include <QFileInfo>

namespace clipboard
{

    struct FileDetailInfo_ {
        QString mFileName; //可能是文件可能是文件夹
        QString mRemotePath;
        bool mIsFile = true; //是否是文件
        uint64_t mFileSize = 0;
    };
    typedef FileDetailInfo_ FileDetailInfo;

    class FileStream : public IStream {
    public:

        FileStream(const FileDetailInfo& info) : ref_(1) {
            mFileDetailInfo = info;
            file_ = std::make_shared<QFile>(info.mRemotePath);
            file_->open(QIODeviceBase::OpenModeFlag::ReadOnly);

            QFileInfo file_info(info.mRemotePath);

            mFileDetailInfo.mFileSize = file_->size();
            mFileDetailInfo.mFileName = file_->fileName();

            target_file_ = std::make_shared<QFile>(std::format("D:\\360Downloads\\z_{}", file_info.fileName().toStdString()).c_str());
            target_file_->open(QIODeviceBase::OpenModeFlag::WriteOnly);
        }

        virtual ~FileStream() {

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

    class VirtualFileSrcStream : public CDataObject, public IDataObjectAsyncCapability {
    public:
        VirtualFileSrcStream() {}
        ~VirtualFileSrcStream() override;

        void init();

        IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv) {
            if (IsEqualIID(IID_IDataObjectAsyncCapability, riid)) {
                *ppv = (IDataObjectAsyncCapability *) this;
                AddRef();
                return S_OK;
            }
            return CDataObject::QueryInterface(riid, ppv);
        }

        IFACEMETHODIMP_(ULONG) AddRef() {
            return CDataObject::AddRef();
        }

        IFACEMETHODIMP_(ULONG) Release() {
            return CDataObject::Release();
        }

        IFACEMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);

        IFACEMETHODIMP QueryGetData(FORMATETC *pformatetc);

        IFACEMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);

        // IDataObjectAsyncCapability
        virtual HRESULT SetAsyncMode(/* [in] */ BOOL fDoOpAsync);

        virtual HRESULT GetAsyncMode(/* [out] */ __RPC__out BOOL *pfIsOpAsync);

        virtual HRESULT StartOperation(/* [optional][unique][in] */ __RPC__in_opt IBindCtx *pbcReserved);

        virtual HRESULT InOperation(/* [out] */ __RPC__out BOOL *pfInAsyncOp);

        virtual HRESULT EndOperation(
                /* [in] */ HRESULT hResult,
                /* [unique][in] */ __RPC__in_opt IBindCtx *pbcReserved,
                /* [in] */ DWORD dwEffects);

    private:
        uint32_t clip_format_filedesc_ = 0;
        uint32_t clip_format_filecontent_ = 0;
        BOOL in_async_op_ = false;
        //std::map<int, std::shared_ptr<FileStream>> file_streams_;
        std::shared_ptr<FileStream> file_stream_ = nullptr;
    public:
        int mFileCount = 0;
        std::vector<FileDetailInfo> mFileDetailInfos;
    };

    STDAPI VirtualFileSrcStream_CreateInstance(REFIID riid, void **ppv);

};


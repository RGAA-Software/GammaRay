#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include "cp_data_object.h"
#include "cp_file_struct.h"

namespace tc
{

    class CpFileStream;
    class Workspace;

    class CpVirtualFile : public CpDataObject, public IDataObjectAsyncCapability {
    public:
        explicit CpVirtualFile(Workspace* ws);
        ~CpVirtualFile() override;

        void Init();

        IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv) {
            if (IsEqualIID(IID_IDataObjectAsyncCapability, riid)) {
                *ppv = (IDataObjectAsyncCapability *) this;
                AddRef();
                return S_OK;
            }
            return CpDataObject::QueryInterface(riid, ppv);
        }

        IFACEMETHODIMP_(ULONG) AddRef() {
            return CpDataObject::AddRef();
        }

        IFACEMETHODIMP_(ULONG) Release() {
            return CpDataObject::Release();
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

        void OnClipboardFilesInfo(const std::string& stream_id, const std::vector<ClipboardFile>& files);
        void OnClipboardRespBuffer(const ClipboardRespBuffer& resp_buffer);

    private:
        uint32_t clip_format_filedesc_ = 0;
        uint32_t clip_format_filecontent_ = 0;
        BOOL in_async_op_ = false;
        std::shared_ptr<CpFileStream> file_stream_ = nullptr;
        Workspace* workspace_ = nullptr;
        std::vector<ClipboardFile> menu_files_;
        std::vector<ClipboardFileWrapper> task_files_;
    };

    CpVirtualFile* CreateVirtualFile(REFIID riid, void **ppv, Workspace* ws);

};


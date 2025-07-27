#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include "panel_cp_file_struct.h"
#include "tc_common_new/log.h"

#pragma comment(lib, "shlwapi.lib")

namespace tc
{

    class GrContext;
    class CpFileStream;

    class CpVirtualFile : public IDataObject, public IDataObjectAsyncCapability {
    public:
        explicit CpVirtualFile(const std::shared_ptr<GrContext>& ctx);
        ~CpVirtualFile();

        void Init();

        HRESULT QueryInterface(REFIID riid, void **ppv) override {
            if (IsEqualIID(IID_IDataObjectAsyncCapability, riid)) {
                *ppv = (IDataObjectAsyncCapability *) this;
                this->AddRef();
                LOGI("Query interface => IID_IDataObjectAsyncCapability");
                return S_OK;
            }

            static const QITAB qit[] = {
                QITABENT(CpVirtualFile, IDataObject),
                { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }

        ULONG AddRef() override {
            return InterlockedIncrement(&_cRef);
        }

        ULONG Release() override {
            long cRef = InterlockedDecrement(&_cRef);
            if (0 == cRef) {
                delete this;
            }
            return cRef;
        }

        long GetRefCount() {
            return _cRef;
        }

        HRESULT GetData(FORMATETC* pformatetcIn, STGMEDIUM *pmedium) override;
        HRESULT GetDataHere(FORMATETC*, STGMEDIUM*) override {
            return DATA_E_FORMATETC;;
        }

        HRESULT QueryGetData(FORMATETC *pformatetc) override;
        HRESULT EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) override;
        HRESULT GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pFormatetcOut) override {
            pformatetcIn->ptd = NULL;
            return E_NOTIMPL;
        }

        HRESULT SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) override {
            return E_NOTIMPL;
        }

        HRESULT DAdvise(FORMATETC*, DWORD, IAdviseSink* , DWORD* ) override {
            return E_NOTIMPL;
        }

        HRESULT DUnadvise(DWORD) override {
            return E_NOTIMPL;
        }

        HRESULT EnumDAdvise(IEnumSTATDATA**) override {
            return E_NOTIMPL;
        }

        // IDataObjectAsyncCapability
        HRESULT SetAsyncMode(BOOL fDoOpAsync) override;
        HRESULT GetAsyncMode(__RPC__out BOOL *pfIsOpAsync) override;
        HRESULT StartOperation(__RPC__in_opt IBindCtx *pbcReserved) override;
        HRESULT InOperation(__RPC__out BOOL *pfInAsyncOp) override;
        HRESULT EndOperation(HRESULT hResult, __RPC__in_opt IBindCtx *pbcReserved, DWORD dwEffects) override;

        void OnClipboardFilesInfo(const std::string& device_id, const std::string& stream_id, const std::vector<ClipboardFile>& files);
        void OnClipboardRespBuffer(const ClipboardRespBuffer& resp_buffer);

    private:
        void ReportFileTransferBegin();
        void ReportFileTransferEnd();

        // Copy in
        void RecordFileTransferBegin();
        // Copy in
        void RecordFileTransferEnd();

    private:
        CLIPFORMAT clip_format_file_desc_ = 0;
        CLIPFORMAT clip_format_file_content_ = 0;
        CLIPFORMAT clip_format_drop_ = 0;
        CLIPFORMAT clip_format_preferred_drop_effect_ = 0;
        BOOL in_async_op_ = false;
        std::shared_ptr<CpFileStream> file_stream_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        // ? clear when transferring...
        std::vector<ClipboardFile> menu_files_;
        std::vector<ClipboardFileWrapper> task_files_;
        long _cRef;
    };

    CpVirtualFile* CreateVirtualFile(REFIID riid, void **ppv, const std::shared_ptr<GrContext>& ctx);

};


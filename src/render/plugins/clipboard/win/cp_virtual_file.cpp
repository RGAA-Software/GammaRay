#include "cp_virtual_file.h"

#define WIN32_LEAN_AND_MEAN
#include <wininet.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <ShlObj.h>
#include <format>
#include <QFileInfo>
#include "cp_file_stream.h"

#pragma comment(lib, "Wininet.lib")

namespace tc
{

    CpVirtualFile::~CpVirtualFile() {
        if (file_stream_) {
            file_stream_->Release();
        }
    }

    void CpVirtualFile::init() {
        clip_format_filedesc_ = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
        clip_format_filecontent_ = RegisterClipboardFormat(CFSTR_FILECONTENTS);

    }

    STDMETHODIMP CpVirtualFile::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
        ZeroMemory(pmedium, sizeof(*pmedium));

        std::vector<std::string> file_paths = {
            "D:\\360Downloads\\muzzy_01.mp4",
            "D:\\360Downloads\\muzzy_02.mp4",
            "D:\\360Downloads\\test_files\\01_Muzzy_in_Gondoland.mp4"
        };

        HRESULT hr = DATA_E_FORMATETC;
        if (pformatetcIn->cfFormat == clip_format_filedesc_) {
            if (pformatetcIn->tymed & TYMED_HGLOBAL) {
                std::cout << "GetData, file desc. HGLOBAL" << std::endl;
                uint32_t file_count = file_paths.size();
                UINT cb = sizeof(FILEGROUPDESCRIPTORW) + (file_count - 1) * sizeof(FILEDESCRIPTORW);
                HGLOBAL h = GlobalAlloc(GHND | GMEM_SHARE, cb);
                if (!h) {
                    hr = E_OUTOFMEMORY;
                } else {
                    FILEGROUPDESCRIPTORW *pGroupDescriptor = (FILEGROUPDESCRIPTORW *) ::GlobalLock(h);
                    if (pGroupDescriptor) {
                        pGroupDescriptor->cItems = file_count;
                        FILEDESCRIPTORW *pFileDescriptorArray = (FILEDESCRIPTORW *) ((LPBYTE) pGroupDescriptor +sizeof(UINT));

                        for (uint32_t index = 0; index < file_count; ++index) {
                            auto path = file_paths.at(index);
                            QFileInfo file_info(path.c_str());

                            auto filename = file_info.fileName();

                            auto target_filename = std::format(L"z_{}", filename.toStdWString());
                            std::wcout << "target filename : " << target_filename << std::endl;
                            if (filename.contains("01_")) {
                                target_filename = L"test_files\\01_Muzzy_in_Gondoland.mp4";
                            }

                            std::cout << "filename : " << filename.toStdString() << ", filesize: " << file_info.size() << std::endl;

                            wcsncpy_s(pFileDescriptorArray[index].cFileName,
                                      _countof(pFileDescriptorArray[index].cFileName), target_filename.c_str(), _TRUNCATE);
                            pFileDescriptorArray[index].dwFlags =
                                    FD_FILESIZE | FD_ATTRIBUTES | FD_CREATETIME | FD_WRITESTIME | FD_PROGRESSUI;
                            pFileDescriptorArray[index].nFileSizeLow = file_info.size();
                            pFileDescriptorArray[index].nFileSizeHigh = 0;
                            pFileDescriptorArray[index].dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

                            SYSTEMTIME lt;
                            GetLocalTime(&lt);
                            FILETIME ft;
                            SystemTimeToFileTime(&lt, &ft);
                            pFileDescriptorArray[index].ftLastAccessTime = ft;
                            pFileDescriptorArray[index].ftCreationTime = ft;
                            pFileDescriptorArray[index].ftLastWriteTime = ft;

                            /*pFileDescriptorArray[index].dwFlags = FD_ATTRIBUTES | FD_CREATETIME | FD_WRITESTIME | FD_PROGRESSUI;
                            pFileDescriptorArray[index].dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;*/

                        }

                        ::GlobalUnlock(h);

                        pmedium->hGlobal = h;
                        pmedium->tymed = TYMED_HGLOBAL;
                        hr = S_OK;
                    }
                }
            }
        } else if (pformatetcIn->cfFormat == clip_format_filecontent_) {
            std::cout << "stream index: " << pformatetcIn->lindex << std::endl;
            if ((pformatetcIn->tymed & TYMED_ISTREAM)) {
                std::cout << "GetData, file desc. ISTREAM" << std::endl;

                file_stream_ = std::make_shared<CpFileStream>(FileDetailInfo {
                    .mRemotePath = QString::fromStdString(file_paths.at(pformatetcIn->lindex)),
                });

                pmedium->pstm = (IStream *)file_stream_.get();
                pmedium->pstm->AddRef();
                pmedium->tymed = TYMED_ISTREAM;
                hr = S_OK;
            }
        } else if (SUCCEEDED(_EnsureShellDataObject())) {
            hr = _pdtobjShell->GetData(pformatetcIn, pmedium);
        }

        return hr;
    }


    STDMETHODIMP CpVirtualFile::QueryGetData(FORMATETC *pformatetc) {
        HRESULT hr = S_FALSE;
        if (pformatetc->cfFormat == clip_format_filedesc_ ||
            pformatetc->cfFormat == clip_format_filecontent_) {
            hr = S_OK;
        } else if (SUCCEEDED(_EnsureShellDataObject())) {
            hr = _pdtobjShell->QueryGetData(pformatetc);
        }

        return hr;
    }

    STDMETHODIMP CpVirtualFile::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) {
        *ppenumFormatEtc = NULL;
        HRESULT hr = E_NOTIMPL;
        if (dwDirection == DATADIR_GET) {
            FORMATETC rgfmtetc[] =
                    {
                            // the order here defines the accuarcy of rendering
                            {(CLIPFORMAT) clip_format_filedesc_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
#ifdef USE_HGLOBAL
                            { clip_format_filecontent_, NULL, DVASPECT_CONTENT,  -1, TYMED_HGLOBAL },
#else
                            {(CLIPFORMAT) clip_format_filecontent_, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM},
#endif

                    };
            hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::SetAsyncMode(
            /* [in] */ BOOL fDoOpAsync) {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::GetAsyncMode(
            /* [out] */ __RPC__out BOOL *pfIsOpAsync) {
        *pfIsOpAsync = true;// VARIANT_TRUE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::StartOperation(
            /* [optional][unique][in] */ __RPC__in_opt IBindCtx *pbcReserved) {
        in_async_op_ = true;
        IOperationsProgressDialog *pDlg = nullptr;
        ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IOperationsProgressDialog,
                           (LPVOID *) &pDlg);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::InOperation(
            /* [out] */ __RPC__out BOOL *pfInAsyncOp) {
        *pfInAsyncOp = in_async_op_;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::EndOperation(
            /* [in] */ HRESULT hResult,
            /* [unique][in] */ __RPC__in_opt IBindCtx *pbcReserved,
            /* [in] */ DWORD dwEffects) {
        in_async_op_ = false;
        return S_OK;
    }


    STDAPI VirtualFileSrcStream_CreateInstance(REFIID riid, void **ppv) {
        *ppv = NULL;
        CpVirtualFile *p = new CpVirtualFile();
        p->init();
        HRESULT hr = p ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr)) {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
        return hr;
    }
};
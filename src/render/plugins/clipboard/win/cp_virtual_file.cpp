#include "cp_virtual_file.h"

#define WIN32_LEAN_AND_MEAN
#include <wininet.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <shlobj.h>
#include <format>
#include <QFileInfo>
#include "cp_file_stream.h"
#include "tc_common_new/log.h"

#pragma comment(lib, "Wininet.lib")

namespace tc
{

    CpVirtualFile::CpVirtualFile(ClipboardPlugin* plugin) {
        plugin_ = plugin;
    }

    CpVirtualFile::~CpVirtualFile() {
        if (file_stream_) {
            file_stream_->Release();
        }
    }

    void CpVirtualFile::Init() {
        clip_format_filedesc_ = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
        clip_format_filecontent_ = RegisterClipboardFormat(CFSTR_FILECONTENTS);

    }

    STDMETHODIMP CpVirtualFile::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
        ZeroMemory(pmedium, sizeof(*pmedium));

        HRESULT hr = DATA_E_FORMATETC;
        if (pformatetcIn->cfFormat == clip_format_filedesc_) {
            uint32_t file_count = menu_files_.size();
            LOGI("GetData, file count: {}", file_count);
            if (file_count <= 0) {
                return S_FALSE;
            }
            if (pformatetcIn->tymed & TYMED_HGLOBAL) {
                UINT cb = sizeof(FILEGROUPDESCRIPTORW) + (file_count - 1) * sizeof(FILEDESCRIPTORW);
                HGLOBAL h = GlobalAlloc(GHND | GMEM_SHARE, cb);
                if (!h) {
                    hr = E_OUTOFMEMORY;
                    LOGE("GlobalAlloc failed!");
                    return hr;
                }

                auto group_descriptor = (FILEGROUPDESCRIPTORW*)::GlobalLock(h);
                if (!group_descriptor) {
                    LOGE("GlobalLock failed!");
                    return S_FALSE;
                }

                group_descriptor->cItems = file_count;
                auto fd_array = (FILEDESCRIPTORW *) ((LPBYTE) group_descriptor +sizeof(UINT));

                for (uint32_t index = 0; index < file_count; ++index) {
                    auto clipboard_file = menu_files_.at(index);
                    LOGI("GetData, file: {}", clipboard_file.file_name());

                    auto target_filename = QString::fromStdString(clipboard_file.ref_path()).toStdWString();
                    wcsncpy_s(fd_array[index].cFileName,
                              _countof(fd_array[index].cFileName), target_filename.c_str(), _TRUNCATE);
                    fd_array[index].dwFlags =
                            FD_FILESIZE | FD_ATTRIBUTES | FD_CREATETIME | FD_WRITESTIME | FD_PROGRESSUI;
                    fd_array[index].nFileSizeLow = clipboard_file.total_size();
                    fd_array[index].nFileSizeHigh = 0;
                    fd_array[index].dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

                    SYSTEMTIME lt;
                    GetLocalTime(&lt);
                    FILETIME ft;
                    SystemTimeToFileTime(&lt, &ft);
                    fd_array[index].ftLastAccessTime = ft;
                    fd_array[index].ftCreationTime = ft;
                    fd_array[index].ftLastWriteTime = ft;
                }

                ::GlobalUnlock(h);

                pmedium->hGlobal = h;
                pmedium->tymed = TYMED_HGLOBAL;
                hr = S_OK;
            }
        } else if (pformatetcIn->cfFormat == clip_format_filecontent_) {
            if ((pformatetcIn->tymed & TYMED_ISTREAM)) {
                auto file_index = pformatetcIn->lindex;
                if (task_files_.size() <= file_index) {
                    LOGE("Invalid file index: {}, we only have: {}", file_index, menu_files_.size());
                    return S_FALSE;
                }
                menu_files_.clear();

                auto fw = task_files_.at(file_index);
                LOGI("Will get data stream for index: {}, name: {}", file_index, fw.file_.file_name());

                if (file_stream_) {
                    file_stream_->Exit();
                }
                file_stream_ = std::make_shared<CpFileStream>(plugin_, fw);

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
            FORMATETC rgfmtetc[] = {
                // the order here defines the accuarcy of rendering
                {(CLIPFORMAT) clip_format_filedesc_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
                {(CLIPFORMAT) clip_format_filecontent_, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM},
            };
            hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::SetAsyncMode(BOOL fDoOpAsync) {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::GetAsyncMode(BOOL *pfIsOpAsync) {
        *pfIsOpAsync = true;// VARIANT_TRUE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::StartOperation(IBindCtx *pbcReserved) {
        in_async_op_ = true;
        IOperationsProgressDialog *pDlg = nullptr;
        ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IOperationsProgressDialog, (LPVOID *) &pDlg);
        LOGI("StartOperation....");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::InOperation(BOOL *pfInAsyncOp) {
        *pfInAsyncOp = in_async_op_;
        LOGI("InOperation....");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::EndOperation(HRESULT hResult, IBindCtx *pbcReserved, DWORD dwEffects) {
        in_async_op_ = false;
        LOGI("EndOperation....");
        if (file_stream_) {
            file_stream_->Exit();
            file_stream_.reset();
        }

        ::OleFlushClipboard();
        ::OleSetClipboard(nullptr);
        menu_files_.clear();
        task_files_.clear();
        return S_OK;
    }

    void CpVirtualFile::OnClipboardFilesInfo(const std::string& stream_id, const std::vector<ClipboardFile>& files) {
        menu_files_ = files;
        task_files_.clear();
        for (const auto& file : files) {
            ClipboardFile cpy_file;
            cpy_file.CopyFrom(file);
            task_files_.push_back(ClipboardFileWrapper {
                .stream_id_ = stream_id,
                .file_ = cpy_file,
            });
        }
        LOGI("On clipboard files size: {}", files.size());
        for (const auto& file : files) {
            LOGI("Name: {}, ref path: {}, total size: {}KB", file.file_name(), file.ref_path(), file.total_size()/1024);
        }
    }

    void CpVirtualFile::OnClipboardRespBuffer(const ClipboardRespBuffer& resp_buffer) {
        if (file_stream_) {
            file_stream_->OnClipboardRespBuffer(resp_buffer);
        }
    }

    CpVirtualFile* CreateVirtualFile(REFIID riid, void **ppv, ClipboardPlugin* plugin) {
        *ppv = nullptr;
        auto p = new CpVirtualFile(plugin);
        p->Init();
        auto hr = p->QueryInterface(riid, ppv);
        if (SUCCEEDED(hr)) {
            p->Release();
            LOGI("CpVirtualFile ref count: {}", p->GetRefCount());
        }
        else {
            LOGE("Query Clipboard DataObject failed: {:x}", hr);
        }
        return p;
    }
};
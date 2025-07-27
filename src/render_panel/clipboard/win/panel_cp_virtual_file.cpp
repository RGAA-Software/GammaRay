#include "panel_cp_virtual_file.h"
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
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "panel_cp_file_stream.h"
#include "tc_common_new/time_util.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_application.h"
#include "tc_message_new/proto_converter.h"
#include "tc_message_new/rp_proto_converter.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render_plugins/clipboard/clipboard_plugin.h"
#include "render_panel/transfer/file_transfer.h"
#include "render_panel/database/gr_database.h"
#include "render_panel/database/visit_record.h"
#include "render_panel/database/visit_record_operator.h"
#include "render_panel/database/file_transfer_record.h"
#include "render_panel/database/file_transfer_record_operator.h"

#pragma comment(lib, "Wininet.lib")

namespace tc
{

    CpVirtualFile::CpVirtualFile(const std::shared_ptr<GrContext>& ctx) {
        _cRef = 1;
        context_ = ctx;
    }

    CpVirtualFile::~CpVirtualFile() {
        if (file_stream_) {
            file_stream_->Release();
        }
    }

    void CpVirtualFile::Init() {
        clip_format_file_desc_ = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
        clip_format_file_content_ = RegisterClipboardFormat(CFSTR_FILECONTENTS);
        clip_format_drop_ = CF_HDROP;
        clip_format_preferred_drop_effect_ = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
    }

    HRESULT CpVirtualFile::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
        ZeroMemory(pmedium, sizeof(*pmedium));

        HRESULT hr = DATA_E_FORMATETC;
        if (pformatetcIn->cfFormat == clip_format_file_desc_) {
            uint32_t file_count = menu_files_.size();
            LOGI("GetData file format, file count: {}", file_count);
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
                    // use ref_path to process folder
                    auto target_filename = QString::fromStdString(clipboard_file.ref_path()).toStdWString();
                    LOGI("GetData, file: {}, name: {}", clipboard_file.file_name(), QString::fromStdWString(target_filename).toStdString());
                    wcsncpy_s(fd_array[index].cFileName, _countof(fd_array[index].cFileName), target_filename.c_str(), _TRUNCATE);

                    fd_array[index].dwFlags = FD_FILESIZE | FD_ATTRIBUTES | FD_CREATETIME | FD_WRITESTIME | FD_PROGRESSUI;
                    uint64_t file_size = static_cast<uint64_t>(clipboard_file.total_size());
                    fd_array[index].nFileSizeLow = static_cast<DWORD>(file_size & 0xFFFFFFFF);
                    fd_array[index].nFileSizeHigh = static_cast<DWORD>((file_size >> 32) & 0xFFFFFFFF);
                    fd_array[index].dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
                    GetSystemTimeAsFileTime(&fd_array[index].ftLastWriteTime);
                    LOGI("GetData, total size: {}, high: {}, low: {}", file_size, fd_array[index].nFileSizeHigh, fd_array[index].nFileSizeLow);

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
        } else if (pformatetcIn->cfFormat == clip_format_file_content_) {
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
                    // report
                    this->ReportFileTransferEnd();
                    file_stream_->Exit();
                }
                file_stream_ = std::make_shared<CpFileStream>(context_, fw);
                // report
                this->ReportFileTransferBegin();

                pmedium->pstm = (IStream *)file_stream_.get();
                pmedium->pstm->AddRef();
                pmedium->tymed = TYMED_ISTREAM;
                hr = S_OK;
            }
        } /*else if (SUCCEEDED(EnsureShellDataObject())) {
            hr = _pdtobjShell->GetData(pformatetcIn, pmedium);
        }*/
        return hr;
    }

    HRESULT CpVirtualFile::QueryGetData(FORMATETC *pformatetc) {
        HRESULT hr = S_FALSE;
        if (pformatetc->cfFormat == clip_format_file_desc_ ||
            pformatetc->cfFormat == clip_format_file_content_ ||
            pformatetc->cfFormat == clip_format_drop_ ||
            pformatetc->cfFormat == clip_format_preferred_drop_effect_) {
            hr = S_OK;
            return S_OK;
        }
        return E_NOTIMPL;
    }

    HRESULT CpVirtualFile::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) {
        *ppenumFormatEtc = NULL;
        HRESULT hr = E_NOTIMPL;
        if (dwDirection == DATADIR_GET) {
            FORMATETC rgfmtetc[] = {
                // the order here defines the accuarcy of rendering
                { (CLIPFORMAT) clip_format_file_desc_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
                { (CLIPFORMAT) clip_format_file_content_, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM},
                { (CLIPFORMAT) clip_format_drop_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
                { (CLIPFORMAT) clip_format_preferred_drop_effect_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
            };
            hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
            if (hr != S_OK) {
                LOGE("SHCreateStdEnumFmtEtc failed!");
            }
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::SetAsyncMode(BOOL fDoOpAsync) {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::GetAsyncMode(BOOL *pfIsOpAsync) {
        *pfIsOpAsync = true;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpVirtualFile::StartOperation(IBindCtx *pbcReserved) {
        in_async_op_ = true;
        //IOperationsProgressDialog *pDlg = nullptr;
        //::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IOperationsProgressDialog, (LPVOID *) &pDlg);
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
            // report
            this->ReportFileTransferEnd();

            file_stream_->Exit();
            file_stream_.reset();
        }

        ::OleFlushClipboard();
        ::OleSetClipboard(nullptr);
        menu_files_.clear();
        task_files_.clear();
        return S_OK;
    }

    void CpVirtualFile::OnClipboardFilesInfo(const std::string& device_id, const std::string& stream_id, const std::vector<ClipboardFile>& files) {
        menu_files_ = files;
        task_files_.clear();
        for (const auto& file : files) {
            ClipboardFile cpy_file;
            cpy_file.CopyFrom(file);
            task_files_.push_back(ClipboardFileWrapper {
                .device_id_ = device_id,
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

    void CpVirtualFile::ReportFileTransferBegin() {
        if (!file_stream_) {
            return;
        }

        // record in database
        this->RecordFileTransferBegin();

        // send begin message to client
        tc::Message msg;
        msg.set_device_id(file_stream_->GetDeviceId());
        msg.set_stream_id(file_stream_->GetStreamId());
        msg.set_type(MessageType::kClipboardReqAtBegin);
        auto req_buffer = msg.mutable_cp_req_at_begin();
        req_buffer->set_full_name(file_stream_->GetFullPath());
        //auto buffer = ProtoAsData(&msg);
        // todo::
        //plugin_->DispatchTargetFileTransferMessage(file_stream_->GetStreamId(), buffer, false);

        auto rp_msg = tc::MakeRpRawRenderMessage(msg.stream_id(), msg.device_id(), msg.SerializeAsString(), true);
        context_->GetApplication()->PostMessage2Renderer(rp_msg);
        // send end message to client
    }

    void CpVirtualFile::ReportFileTransferEnd() {
        if (!file_stream_) {
            return;
        }

        // record in database
        this->RecordFileTransferEnd();

        // send end message to client
        tc::Message msg;
        msg.set_device_id(file_stream_->GetDeviceId());
        msg.set_stream_id(file_stream_->GetStreamId());
        msg.set_type(MessageType::kClipboardReqAtEnd);
        auto req_buffer = msg.mutable_cp_req_at_end();
        req_buffer->set_full_name(file_stream_->GetFullPath());
        req_buffer->set_success(true);
        //auto buffer = ProtoAsData(&msg);
        // todo::
        //plugin_->DispatchTargetFileTransferMessage(file_stream_->GetStreamId(), buffer, false);

        auto rp_msg = tc::MakeRpRawRenderMessage(msg.stream_id(), msg.device_id(), msg.SerializeAsString(), true);
        context_->GetApplication()->PostMessage2Renderer(rp_msg);
    }

    void CpVirtualFile::RecordFileTransferBegin() {
        context_->PostDBTask([=, this]() {
//            auto event = std::make_shared<GrPluginFileTransferBegin>();
//            event->the_file_id_ = file_stream_->GetFileId();
//            event->begin_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
//            event->visitor_device_id_ = file_stream_->GetDeviceId();
//            event->direction_ = "In";
//            event->file_detail_ = file_stream_->GetFileName();

            auto ips = context_->GetIps();
            std::string ip_address;
            if (!ips.empty()) {
                ip_address = ips[0].ip_addr_;
            }

            auto settings = GrSettings::Instance();
            auto ft_record_op = context_->GetDatabase()->GetFileTransferRecordOp();
            ft_record_op->InsertFileTransferRecord(std::make_shared<FileTransferRecord>(FileTransferRecord {
                .the_file_id_ = file_stream_->GetFileId(),
                .begin_ = (int64_t)TimeUtil::GetCurrentTimestamp(),
                .end_ = 0,
                .visitor_device_ = file_stream_->GetDeviceId(),
                .target_device_ = settings->GetDeviceId().empty() ? ip_address : settings->GetDeviceId(),
                .direction_ = "In",
                .file_detail_ = file_stream_->GetFileName(),
            }));
        });
    }

    void CpVirtualFile::RecordFileTransferEnd() {
        context_->PostDBTask([=, this]() {
//            auto event = std::make_shared<GrPluginFileTransferEnd>();
//            event->the_file_id_ = file_stream_->GetFileId();
//            event->end_timestamp_ = (int64_t)TimeUtil::GetCurrentTimestamp();
//            event->success_ = true;

            auto settings = GrSettings::Instance();
            auto ft_record_op = context_->GetDatabase()->GetFileTransferRecordOp();
            ft_record_op->UpdateVisitRecord(file_stream_->GetFileId(), (int64_t)TimeUtil::GetCurrentTimestamp(), true);
        });
    }

    CpVirtualFile* CreateVirtualFile(REFIID riid, void **ppv, const std::shared_ptr<GrContext>& ctx) {
        *ppv = nullptr;
        auto p = new CpVirtualFile(ctx);
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
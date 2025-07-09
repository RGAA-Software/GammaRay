//
// Created by RGAA on 8/04/2025.
//

#include "cp_file_stream.h"
#include "ct_settings.h"
#include "tc_common_new/log.h"
#include "ct_base_workspace.h"
#include "tc_message_new/proto_converter.h"
#include "tc_client_sdk_new/thunder_sdk.h"
#include "client/plugins/ct_plugin_events.h"
#include "client/plugins/clipboard/clipboard_plugin.h"

namespace tc
{

    HRESULT STDMETHODCALLTYPE CpFileStream::QueryInterface(REFIID riid, void **ppvObject) {
        if (ppvObject == nullptr)
            return E_INVALIDARG;

        *ppvObject = nullptr;

        if (IsEqualIID(IID_IUnknown, riid) ||
            IsEqualIID(IID_ISequentialStream, riid) ||
            IsEqualIID(IID_IStream, riid)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE CpFileStream::Read(void *pv, ULONG cb, ULONG *pcbRead) {
        // read from remote synchronized
        auto settings = plugin_->GetPluginSettings();
        tc::Message msg;
        msg.set_device_id(settings.device_id_);
        msg.set_stream_id(settings.stream_id_);
        msg.set_type(MessageType::kClipboardReqBuffer);
        auto req_buffer = msg.mutable_cp_req_buffer();
        req_buffer->set_req_index(req_index_);
        req_buffer->set_req_size(cb);
        req_buffer->set_req_start(current_position_);
        req_buffer->set_full_name(cp_file_.file_.full_path());

        auto event = std::make_shared<ClientPluginNetworkEvent>();
        event->media_channel_ = false;
        event->buf_ = tc::ProtoAsData(&msg);
        plugin_->CallbackEvent(event);
        //LOGI("request index: {}, current position: {}, req size: {}", req_index_, current_position_, cb);

        std::unique_lock lk(wait_data_mtx_);
        data_cv_.wait_for(lk, std::chrono::seconds(10), [this]() -> bool {
            return resp_buffer_.has_value();
        });

        if (exit_ || !resp_buffer_.has_value()) {
            LOGW("exit copy file: {}", cp_file_.file_.ref_path());
            return S_FALSE;
        }

        if (req_index_ != resp_buffer_->req_index()) {
            LOGE("invalid req index, send: {}, received: {}", req_index_, resp_buffer_->req_index());
            return S_FALSE;
        }

        // copy data
        auto resp_buffer = resp_buffer_.value();
        if (resp_buffer.read_size() > 0) {
            memcpy(pv, resp_buffer.buffer().data(), resp_buffer.read_size());
            *pcbRead = resp_buffer.read_size();
            current_position_ += resp_buffer.read_size();
        }
        req_index_ += 1;

        // clear data
        resp_buffer_.reset();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CpFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* new_pos) {
        switch (dwOrigin) {
            case STREAM_SEEK_SET:
                current_position_ = 0;
                if (new_pos) {
                    new_pos->QuadPart = 0;
                }
                LOGI("seek set: {}", cp_file_.file_.file_name());
                break;
            case STREAM_SEEK_CUR:
                LOGI("seek current: {}", cp_file_.file_.file_name());
                break;
            case STREAM_SEEK_END:
                LOGI("seek end: {}", cp_file_.file_.file_name());
                break;
            default:
                return STG_E_INVALIDFUNCTION;
        }
        return S_OK;
    }

    // 没有被调用
    HRESULT WINAPI CpFileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag) {
        memset(pstatstg, 0, sizeof(STATSTG));
        pstatstg->pwcsName = NULL;
        pstatstg->type = STGTY_STREAM;
        pstatstg->cbSize.QuadPart = cp_file_.file_.total_size();
        return S_OK;
    }

    void CpFileStream::OnClipboardRespBuffer(const ClipboardRespBuffer& rb) {
        ClipboardRespBuffer buffer;
        buffer.CopyFrom(rb);
        resp_buffer_ = buffer;
        data_cv_.notify_all();
    }

    void CpFileStream::Exit() {
        exit_ = true;
        data_cv_.notify_all();
    }

    std::string CpFileStream::GetFileId() {
        return gen_file_id_;
    }

    std::string CpFileStream::GetFileName() {
        return cp_file_.file_.file_name();
    }

    std::string CpFileStream::GetFullPath() {
        return cp_file_.file_.full_path();
    }
}
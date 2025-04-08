//
// Created by RGAA on 8/04/2025.
//

#include "cp_file_stream.h"
#include "tc_common_new/log.h"
#include <iostream>

namespace tc
{

    HRESULT STDMETHODCALLTYPE CpFileStream::QueryInterface(REFIID riid, void **ppvObject) {
        if (ppvObject == NULL)
            return E_INVALIDARG;

        *ppvObject = NULL;

        if (IsEqualIID(IID_IUnknown, riid) ||
            IsEqualIID(IID_ISequentialStream, riid) ||
            IsEqualIID(IID_IStream, riid)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        /*if (IsEqualIID(IID_IOperationsProgressDialog, riid)) {
            return E_NOINTERFACE;
        }*/

        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE CpFileStream::Read(void *pv, ULONG cb, ULONG *pcbRead) {

        char* buffer = (char*)malloc(cb);
        auto read_bytes = file_->read(buffer, cb);

        memcpy(pv, buffer, read_bytes);
        target_file_->write(buffer, read_bytes);

        *pcbRead = read_bytes;

        //std::cout << "cb: " << cb << ",read bytes: " << read_bytes << std::endl;
        return read_bytes > 0 ? S_OK : S_FALSE;
    }

    HRESULT
    STDMETHODCALLTYPE CpFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) {
        //ULARGE_INTEGER new_pos = {0};

        switch (dwOrigin) {
            case STREAM_SEEK_SET:
                current_position_ = 0;
                if (plibNewPosition) {
                    plibNewPosition->QuadPart = 0;
                }
                std::cout << "seek set" << std::endl;
                break;
            case STREAM_SEEK_CUR:
                //new_pos = current_position_;
                std::cout << "seek current" << std::endl;
                break;
            case STREAM_SEEK_END:
                std::cout << "seek end" << std::endl;
                //new_pos = file_size_;
                break;
            default:
                return STG_E_INVALIDFUNCTION;
        }
//        printf("CpFileStream::Seek new_pos  =%d \n", new_pos);
//        new_pos.QuadPart += dlibMove.QuadPart;
//        if (new_pos.QuadPart < 0 || new_pos.QuadPart > file_size_.QuadPart) {
//            return STG_E_INVALIDFUNCTION;
//        }

        return S_OK;
    }

    HRESULT WINAPI CpFileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag) {
        memset(pstatstg, 0, sizeof(STATSTG));

        pstatstg->pwcsName = NULL;
        pstatstg->type = STGTY_STREAM;
        pstatstg->cbSize.QuadPart = mFileDetailInfo.mFileSize;
        printf("CpFileStream::Stat----size: %d\n", (int)mFileDetailInfo.mFileSize);
        return S_OK;
    }
    
}
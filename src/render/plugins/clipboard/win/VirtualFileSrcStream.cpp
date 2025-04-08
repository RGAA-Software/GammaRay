#include "VirtualFileSrcStream.h"

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

#pragma comment(lib, "Wininet.lib")

namespace clipboard
{

    HRESULT STDMETHODCALLTYPE FileStream::QueryInterface(REFIID riid, void **ppvObject) {
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

    HRESULT STDMETHODCALLTYPE FileStream::Read(void *pv, ULONG cb, ULONG *pcbRead) {

        char* buffer = (char*)malloc(cb);
        auto read_bytes = file_->read(buffer, cb);

        memcpy(pv, buffer, read_bytes);
        target_file_->write(buffer, read_bytes);

        *pcbRead = read_bytes;

        //std::cout << "cb: " << cb << ",read bytes: " << read_bytes << std::endl;
        return read_bytes > 0 ? S_OK : S_FALSE;
//        ULONG bytes_to_read = std::min((ULONG) (file_size_.QuadPart - current_position_.QuadPart), cb);
//        const int BUF_SIZE = 256 * 1024;
//        char *fake_data = new char[BUF_SIZE];
//        //memset(fake_data, 'F', BUF_SIZE);
//        memset(fake_data, 0, BUF_SIZE);
//
//        static DWORD Number = 1;
//
//        static HINTERNET hSession = InternetOpenW(L"RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
//        if (hSession != NULL) {
//            static HINTERNET handle2 = InternetOpenUrlW(hSession, L"http://127.0.0.1:30500/muzzy.mp4", NULL, 0,
//                                                        INTERNET_FLAG_DONT_CACHE, 0);
//            if (handle2 != NULL) {
//                InternetReadFile(handle2, fake_data, BUF_SIZE, &Number);
//                if (Number <= 0) {
//                    InternetCloseHandle(handle2);
//                    InternetCloseHandle(hSession);
//                    handle2 = NULL;
//                    hSession = NULL;
//                } else {
//                    static std::ofstream writeFile;
//                    static bool isFirst = true;
//                    if (isFirst) {
//                        writeFile.open(R"(.\httpdown.mp4)", std::ios::out | std::ios::binary);
//                    }
//                    isFirst = false;
//                    writeFile.write(fake_data, Number);
//                    writeFile.flush();
//                }
//            }
//        }
//        ULONG bytes_now = bytes_to_read;
//        while (bytes_now > 0) {
//            ULONG to_read_once = std::min((ULONG) BUF_SIZE, bytes_now);
//            memcpy(pv, fake_data, to_read_once);
//            (char *&) pv += to_read_once;
//            bytes_now -= to_read_once;
//        }
//        delete fake_data;
//        current_position_.QuadPart += bytes_to_read;
//
//        if (pcbRead) {
//            *pcbRead = bytes_to_read;
//        }
//
//        /*
//        * Always returns S_OK even if the end of the stream is reached before the
//        * buffer is filled
//        */
//        ::Sleep(10);
//        static int times = 0;
//        //printf("-------------------------------------FileStream::Read------new----------times=%d\n", ++times);
//
//        return S_FALSE;
    }

    HRESULT
    STDMETHODCALLTYPE FileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) {
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
//        printf("FileStream::Seek new_pos  =%d \n", new_pos);
//        new_pos.QuadPart += dlibMove.QuadPart;
//        if (new_pos.QuadPart < 0 || new_pos.QuadPart > file_size_.QuadPart) {
//            return STG_E_INVALIDFUNCTION;
//        }

        return S_OK;
    }

    HRESULT WINAPI FileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag) {
        memset(pstatstg, 0, sizeof(STATSTG));

        pstatstg->pwcsName = NULL;
        pstatstg->type = STGTY_STREAM;
        pstatstg->cbSize.QuadPart = mFileDetailInfo.mFileSize;
        printf("FileStream::Stat----size: %d\n", (int)mFileDetailInfo.mFileSize);
        return S_OK;
    }

    //////////////////////////////////////////////////////
    VirtualFileSrcStream::~VirtualFileSrcStream() {
        if (file_stream_) {
            file_stream_->Release();
        }
    }

    void VirtualFileSrcStream::init() {
        clip_format_filedesc_ = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
        clip_format_filecontent_ = RegisterClipboardFormat(CFSTR_FILECONTENTS);

    }

    STDMETHODIMP VirtualFileSrcStream::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
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

                //if (!file_streams_.contains(pformatetcIn->lindex)) {
                    //std::cout << "file_stream_ = new FileStream(FAKE_FILE_SIZE);" << std::endl;
                    //auto fs = std::make_shared<FileStream>(FAKE_FILE_SIZE);
                    //file_streams_[pformatetcIn->lindex] = fs;
                //} else {
                //    LARGE_INTEGER mov;
                //    mov.QuadPart = 0;
                //    file_streams_[pformatetcIn->lindex]->Seek(mov, STREAM_SEEK_SET, nullptr);
                //}
                if (!file_stream_) {

                }
                file_stream_ = std::make_shared<FileStream>(FileDetailInfo {
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


    STDMETHODIMP VirtualFileSrcStream::QueryGetData(FORMATETC *pformatetc) {
        HRESULT hr = S_FALSE;
        if (pformatetc->cfFormat == clip_format_filedesc_ ||
            pformatetc->cfFormat == clip_format_filecontent_) {
            hr = S_OK;
        } else if (SUCCEEDED(_EnsureShellDataObject())) {
            hr = _pdtobjShell->QueryGetData(pformatetc);
        }

        return hr;
    }

    STDMETHODIMP VirtualFileSrcStream::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) {
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

    HRESULT STDMETHODCALLTYPE VirtualFileSrcStream::SetAsyncMode(
            /* [in] */ BOOL fDoOpAsync) {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE VirtualFileSrcStream::GetAsyncMode(
            /* [out] */ __RPC__out BOOL *pfIsOpAsync) {
        *pfIsOpAsync = true;// VARIANT_TRUE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE VirtualFileSrcStream::StartOperation(
            /* [optional][unique][in] */ __RPC__in_opt IBindCtx *pbcReserved) {
        in_async_op_ = true;
        IOperationsProgressDialog *pDlg = nullptr;
        ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IOperationsProgressDialog,
                           (LPVOID *) &pDlg);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE VirtualFileSrcStream::InOperation(
            /* [out] */ __RPC__out BOOL *pfInAsyncOp) {
        *pfInAsyncOp = in_async_op_;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE VirtualFileSrcStream::EndOperation(
            /* [in] */ HRESULT hResult,
            /* [unique][in] */ __RPC__in_opt IBindCtx *pbcReserved,
            /* [in] */ DWORD dwEffects) {
        in_async_op_ = false;
        return S_OK;
    }


    STDAPI VirtualFileSrcStream_CreateInstance(REFIID riid, void **ppv) {
        *ppv = NULL;
        VirtualFileSrcStream *p = new VirtualFileSrcStream();
        p->init();
        HRESULT hr = p ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr)) {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
        return hr;
    }
};
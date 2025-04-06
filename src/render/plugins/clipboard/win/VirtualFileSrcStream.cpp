#include "VirtualFileSrcStream.h"

#define WIN32_LEAN_AND_MEAN

#include <wininet.h>
//#include <WinSock2.h>
#include <Windows.h>


#include <iostream>
#include <string>
#include <memory>

#pragma comment(lib, "Wininet.lib")


#include <iostream>
#include <fstream>

#include <ShlObj.h>

namespace clipboard
{

//#define FAKE_FILE_SIZE 24*1024*1024
//#define FAKE_FILE_SIZE 26414343
#define FAKE_FILE_SIZE 105830151
//#define USE_HGLOBAL

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
        ULONG bytes_to_read = std::min((ULONG) (file_size_.QuadPart - current_position_.QuadPart), cb);

        //return STG_E_INVALIDPOINTER;

        printf("FileStream::Read cb = %d\n", cb);
        printf("FileStream::Read bytes_to_read = %d\n", bytes_to_read);


#if 0   // 读取本地文件的方式
        const int BUF_SIZE = 256 * 1024;
        char *fake_data = new char[BUF_SIZE];
        //memset(fake_data, 'F', BUF_SIZE);
        memset(fake_data, 0, BUF_SIZE);

        // F:\mydev\player\yqPlayer\testVideo
        // size 26414343

        {
            static std::ifstream readFile;
            static std::ofstream writeFile;
            static bool isFirst = true;
            if (isFirst) {
                readFile.open(R"(F:\mydev\player\yqPlayer\testVideo\10.mp4)", std::ios::in | std::ios::binary);
                writeFile.open(R"(F:\mydev\player\yqPlayer\testVideo\30.mp4)", std::ios::out | std::ios::binary);
            }
            isFirst = false;
            //char* fake_data_1 = new char[BUF_SIZE];
            readFile.read(fake_data, BUF_SIZE);
            std::cout << "read gcount = " << readFile.gcount() << std::endl;
            writeFile.write(fake_data, readFile.gcount());
            if (!readFile) {
                readFile.close();
                writeFile.flush();
                writeFile.close();
            }
        }
#endif
        // 读取网络资源

        const int BUF_SIZE = 256 * 1024;
        char *fake_data = new char[BUF_SIZE];
        //memset(fake_data, 'F', BUF_SIZE);
        memset(fake_data, 0, BUF_SIZE);

        static DWORD Number = 1;

        static HINTERNET hSession = InternetOpenW(L"RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession != NULL) {
            static HINTERNET handle2 = InternetOpenUrlW(hSession, L"http://127.0.0.1:30500/muzzy.mp4", NULL, 0,
                                                        INTERNET_FLAG_DONT_CACHE, 0);
            if (handle2 != NULL) {
                InternetReadFile(handle2, fake_data, BUF_SIZE, &Number);
                if (Number <= 0) {
                    InternetCloseHandle(handle2);
                    InternetCloseHandle(hSession);
                    handle2 = NULL;
                    hSession = NULL;
                } else {
                    static std::ofstream writeFile;
                    static bool isFirst = true;
                    if (isFirst) {
                        writeFile.open(R"(.\httpdown.mp4)", std::ios::out | std::ios::binary);
                    }
                    isFirst = false;
                    writeFile.write(fake_data, Number);
                    writeFile.flush();
                }
            }
        }
        ULONG bytes_now = bytes_to_read;
        while (bytes_now > 0) {
            ULONG to_read_once = std::min((ULONG) BUF_SIZE, bytes_now);
            memcpy(pv, fake_data, to_read_once);
            (char *&) pv += to_read_once;
            bytes_now -= to_read_once;
        }
        delete fake_data;
        current_position_.QuadPart += bytes_to_read;

        if (pcbRead) {
            *pcbRead = bytes_to_read;
        }

        /*
        * Always returns S_OK even if the end of the stream is reached before the
        * buffer is filled
        */
        ::Sleep(10);
        static int times = 0;
        printf("-------------------------------------FileStream::Read------new----------times=%d\n", ++times);

        return S_FALSE;
    }

    HRESULT
    STDMETHODCALLTYPE FileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) {
        ULARGE_INTEGER new_pos = {0};

        switch (dwOrigin) {
            case STREAM_SEEK_SET:
                break;
            case STREAM_SEEK_CUR:
                new_pos = current_position_;
                break;
            case STREAM_SEEK_END:
                printf("STREAM_SEEK_END ----\n");
                new_pos = file_size_;
                break;
            default:
                return STG_E_INVALIDFUNCTION;
        }
        printf("FileStream::Seek new_pos  =%d \n", new_pos);
        new_pos.QuadPart += dlibMove.QuadPart;
        if (new_pos.QuadPart < 0 || new_pos.QuadPart > file_size_.QuadPart) {
            return STG_E_INVALIDFUNCTION;
        }

        if (plibNewPosition) {
            *plibNewPosition = new_pos;
        }
        current_position_ = new_pos;
        printf("FileStream::Seek return  S_OK \n");
        return S_OK;
    }

    HRESULT WINAPI FileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag) {
        memset(pstatstg, 0, sizeof(STATSTG));

        pstatstg->pwcsName = NULL;
        pstatstg->type = STGTY_STREAM;
        pstatstg->cbSize = file_size_;
        printf("FileStream::Stat----\n");
        return S_OK;
    }

    //////////////////////////////////////////////////////
    VirtualFileSrcStream::~VirtualFileSrcStream() {
        if (file_stream_) {
            file_stream_->Release();
            file_stream_ = nullptr;
        }
    }

    void VirtualFileSrcStream::init() {
        clip_format_filedesc_ = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
        clip_format_filecontent_ = RegisterClipboardFormat(CFSTR_FILECONTENTS);

    }

//	bool VirtualFileSrcStream::set_to_clipboard()
//	{
//		if (clip_format_filedesc_ == 0) {
//			init();
//		}
//		return SUCCEEDED(::OleSetClipboard(this));
//	}

    STDMETHODIMP VirtualFileSrcStream::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
        ZeroMemory(pmedium, sizeof(*pmedium));

        HRESULT hr = DATA_E_FORMATETC;
        if (pformatetcIn->cfFormat == clip_format_filedesc_) {
            if (pformatetcIn->tymed & TYMED_HGLOBAL) {
                uint32_t file_count = 1;
                UINT cb = sizeof(FILEGROUPDESCRIPTORW) + (file_count - 1) * sizeof(FILEDESCRIPTORW);
                HGLOBAL h = GlobalAlloc(GHND | GMEM_SHARE, cb);
                if (!h) {
                    hr = E_OUTOFMEMORY;
                } else {
                    FILEGROUPDESCRIPTORW *pGroupDescriptor = (FILEGROUPDESCRIPTORW *) ::GlobalLock(h);
                    if (pGroupDescriptor) {
                        pGroupDescriptor->cItems = file_count;
                        FILEDESCRIPTORW *pFileDescriptorArray = (FILEDESCRIPTORW *) ((LPBYTE) pGroupDescriptor +
                                                                                     sizeof(UINT));

                        for (uint32_t index = 0; index < file_count; ++index) {
                            wcsncpy_s(pFileDescriptorArray[index].cFileName,
                                      _countof(pFileDescriptorArray[index].cFileName), L"v.mp4", _TRUNCATE);
                            //pFileDescriptorArray[index].dwFlags = FD_UNICODE| FD_FILESIZE | FD_ATTRIBUTES| FD_PROGRESSUI| FD_CREATETIME| FD_SIZEPOINT;
                            pFileDescriptorArray[index].dwFlags =
                                    FD_FILESIZE | FD_ATTRIBUTES | FD_CREATETIME | FD_WRITESTIME | FD_PROGRESSUI;
                            pFileDescriptorArray[index].nFileSizeLow = FAKE_FILE_SIZE;
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
#ifdef USE_HGLOBAL
            if (pformatetcIn->tymed & TYMED_HGLOBAL)
            {
                HGLOBAL h = GlobalAlloc(GHND | GMEM_SHARE, FAKE_FILE_SIZE);
                if (!h) {
                    hr = E_OUTOFMEMORY;
                }
                else {
                    uint8_t *p_data = (uint8_t*)::GlobalLock(h);
                    if (p_data)
                    {

                        memset(p_data, 'A', FAKE_FILE_SIZE);
                        ::GlobalUnlock(h);

                        pmedium->hGlobal = h;
                        pmedium->tymed = TYMED_HGLOBAL;
                        hr = S_OK;
                    }

        }
    }
#else
            if ((pformatetcIn->tymed & TYMED_ISTREAM)) {

#if 1
                if (file_stream_ == nullptr) {
                    std::cout << "file_stream_ = new FileStream(FAKE_FILE_SIZE);" << std::endl;
                    file_stream_ = new FileStream(FAKE_FILE_SIZE);
                } else {
                    LARGE_INTEGER mov;
                    mov.QuadPart = 0;
                    file_stream_->Seek(mov, STREAM_SEEK_SET, nullptr);
                }
                pmedium->pstm = (IStream *) file_stream_;
                pmedium->pstm->AddRef();

#else
                pmedium->pstm = SHOpenRegStream(HKEY_LOCAL_MACHINE,
                    TEXT("Hardware\\Description\\System\\CentralProcessor\\0"),
                    TEXT("Identifier"), STGM_READ);
#endif

                pmedium->tymed = TYMED_ISTREAM;
                hr = S_OK;
            }
#endif
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
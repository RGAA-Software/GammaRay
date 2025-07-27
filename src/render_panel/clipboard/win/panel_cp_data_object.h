//#pragma once
//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
////
//// Sample data object implementation that demonstrates how to leverage the
//// shell provided data object for the SetData() support
//
//#include <windows.h>
//#include <shlwapi.h>
//#include <strsafe.h>
//#include <shlobj.h>
//
//#pragma comment(lib, "shlwapi.lib")
//
//namespace tc
//{
//
//    class CpDataObject : public IDataObject {
//    public:
//        CpDataObject() : _cRef(1), _pdtobjShell(NULL) {
//        }
//
//        virtual ~CpDataObject() {
//            if (_pdtobjShell) {
//                _pdtobjShell->Release();
//            }
//        }
//
//        // IUnknown
////        HRESULT QueryInterface(REFIID riid, void **ppv) override {
////            static const QITAB qit[] = {
////                QITABENT(CpDataObject, IDataObject),
////                {0},
////            };
////            return QISearch(this, qit, riid, ppv);
////        }
//
//        ULONG AddRef() override {
//            return InterlockedIncrement(&_cRef);
//        }
//
//        ULONG Release() override {
//            long cRef = InterlockedDecrement(&_cRef);
//            if (0 == cRef) {
//                delete this;
//            }
//            return cRef;
//        }
//
//        long GetRefCount() {
//            return _cRef;
//        }
//
//        // IDataObject
////        HRESULT GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) override;
//
//        HRESULT GetDataHere(FORMATETC * /* pformatetc */, STGMEDIUM * /* pmedium */) override {
//            return DATA_E_FORMATETC;;
//        }
//
////        HRESULT QueryGetData(FORMATETC *pformatetc) override;
//
//        HRESULT GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pFormatetcOut) override {
//            pformatetcIn->ptd = NULL;
//            return E_NOTIMPL;
//        }
//
//        HRESULT SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) override;
//
////        HRESULT EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) override;
//
//        HRESULT DAdvise(FORMATETC * /* pformatetc */, DWORD /* advf */, IAdviseSink * /* pAdvSnk */, DWORD * /* pdwConnection */) override {
//            return E_NOTIMPL;
//        }
//
//        HRESULT DUnadvise(DWORD /* dwConnection */) override {
//            return E_NOTIMPL;
//        }
//
//        HRESULT EnumDAdvise(IEnumSTATDATA ** /* ppenumAdvise */) override {
//            return E_NOTIMPL;
//        }
//
//    protected:
//        HRESULT EnsureShellDataObject() {
//            // the shell data object imptlements ::SetData() in a way that will store any format
//            // this code delegates to that implementation to avoid having to implement ::SetData()
//            return _pdtobjShell ? S_OK : SHCreateDataObject(NULL, 0, NULL, NULL, IID_PPV_ARGS(&_pdtobjShell));
//        }
//
//        IDataObject *_pdtobjShell = nullptr;
//
//    private:
//        long _cRef;
//
//    };
//
//}
//

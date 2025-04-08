#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Sample data object implementation that demonstrates how to leverage the
// shell provided data object for the SetData() support

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")

namespace tc
{

    class CpDataObject : public IDataObject {
    public:
        CpDataObject() : _cRef(1), _pdtobjShell(NULL) {
        }

        virtual ~CpDataObject() {
            if (_pdtobjShell) {
                _pdtobjShell->Release();
            }
        }

        // IUnknown
        IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv) {
            static const QITAB qit[] = {
                    QITABENT(CpDataObject, IDataObject),
                    {0},
            };
            //printf("CpDataObject %x\n", this);
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP_(ULONG) AddRef() {
            //OutputDebugString(L"CpDataObject AddRef\n");
            return InterlockedIncrement(&_cRef);
        }

        IFACEMETHODIMP_(ULONG) Release() {
            //OutputDebugString(L"CpDataObject Release\n");
            long cRef = InterlockedDecrement(&_cRef);
            if (0 == cRef) {
                delete this;
            }
            return cRef;
        }

        // IDataObject
        IFACEMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);

        IFACEMETHODIMP GetDataHere(FORMATETC * /* pformatetc */, STGMEDIUM * /* pmedium */) {
            return DATA_E_FORMATETC;;
        }

        IFACEMETHODIMP QueryGetData(FORMATETC *pformatetc);

        IFACEMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pFormatetcOut) {
#if 1
            pformatetcIn->ptd = NULL;
            return E_NOTIMPL;
#else
            *pFormatetcOut = *pformatetcIn;
            pFormatetcOut->ptd = NULL;
            return DATA_S_SAMEFORMATETC;
#endif
        }

        IFACEMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);

        IFACEMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);

        IFACEMETHODIMP DAdvise(FORMATETC * /* pformatetc */, DWORD /* advf */, IAdviseSink * /* pAdvSnk */,
                               DWORD * /* pdwConnection */) {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP DUnadvise(DWORD /* dwConnection */) {
            return E_NOTIMPL;
        }

        IFACEMETHODIMP EnumDAdvise(IEnumSTATDATA ** /* ppenumAdvise */) {
            return E_NOTIMPL;
        }

    protected:
        HRESULT _EnsureShellDataObject() {
            // the shell data object imptlements ::SetData() in a way that will store any format
            // this code delegates to that implementation to avoid having to implement ::SetData()
            return _pdtobjShell ? S_OK : SHCreateDataObject(NULL, 0, NULL, NULL, IID_PPV_ARGS(&_pdtobjShell));
        }

        IDataObject *_pdtobjShell;

    private:

        long _cRef;
    };

}    // namespace clipboard


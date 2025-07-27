//#include "panel_cp_data_object.h"
//
//namespace tc
//{
//
////    const WCHAR *const c_szText = L"Clipboard Contents";
//
////    HRESULT CpDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
////        ZeroMemory(pmedium, sizeof(*pmedium));
////
////        HRESULT hr = DATA_E_FORMATETC;
////        if (pformatetcIn->cfFormat == CF_UNICODETEXT) {
////            if (pformatetcIn->tymed & TYMED_HGLOBAL) {
////                UINT cb = sizeof(c_szText[0]) * (lstrlenW(c_szText) + 1);
////                HGLOBAL h = GlobalAlloc(GPTR, cb);
////                hr = h ? S_OK : E_OUTOFMEMORY;
////                if (SUCCEEDED(hr)) {
////                    StringCbCopyW((PWSTR) h, cb, c_szText);
////                    pmedium->hGlobal = h;
////                    pmedium->tymed = TYMED_HGLOBAL;
////                }
////            }
////        } else if (SUCCEEDED(EnsureShellDataObject())) {
////            hr = _pdtobjShell->GetData(pformatetcIn, pmedium);
////        }
////        return hr;
////    }
//
//    HRESULT CpDataObject::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) {
//        HRESULT hr = EnsureShellDataObject();
//        if (SUCCEEDED(hr)) {
//            hr = _pdtobjShell->SetData(pformatetc, pmedium, fRelease);
//        }
//        return hr;
//    }
////
////    HRESULT CpDataObject::QueryGetData(FORMATETC *pformatetc) {
////        HRESULT hr = S_FALSE;
////        if (pformatetc->cfFormat == CF_UNICODETEXT) {
////            hr = S_OK;
////        } else if (SUCCEEDED(EnsureShellDataObject())) {
////            hr = _pdtobjShell->QueryGetData(pformatetc);
////        }
////        return hr;
////    }
//
////    STDMETHODIMP CpDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) {
////        *ppenumFormatEtc = NULL;
////        HRESULT hr = E_NOTIMPL;
////        if (dwDirection == DATADIR_GET) {
////            FORMATETC rgfmtetc[] =
////            {
////                {CF_UNICODETEXT, NULL, 0, 0, TYMED_HGLOBAL},
////            };
////            hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
////        }
////        return hr;
////    }
//
//
//}
//

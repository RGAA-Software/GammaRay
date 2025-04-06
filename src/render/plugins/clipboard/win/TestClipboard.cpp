// TestClipboard.cpp : 定义应用程序的入口点。
//
#include <iostream>
#include <string>
#include <memory>

#include "framework.h"
#include "TestClipboard.h"
#include "DataObject.h"
//#include "VirtualFileSrc.h"
#include "VirtualFileSrcStream.h"
//#include "DragDrop.h"
//#include "MessageWindow.h"


#define MAX_LOADSTRING 100

BOOL InitInstance() {
    ::OleInitialize(nullptr);
    IDataObject* data_obj = nullptr;
    HRESULT hr = clipboard::VirtualFileSrcStream_CreateInstance(IID_IDataObject, (void**)&data_obj);
    if (SUCCEEDED(hr)) {
        ::OleSetClipboard(data_obj);
        data_obj->Release();
    }
    return TRUE;
}


int main() {
    std::cout << "This is a test info" << std::endl;
    // 执行应用程序初始化:
    if (!InitInstance ())
    {
        return FALSE;
    }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    OleUninitialize();
    return 0;
}
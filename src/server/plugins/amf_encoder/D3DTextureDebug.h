#pragma once

#include <combaseapi.h>
#include <atlbase.h>
#include <fstream>

#include <dxgi.h>
#include <d3d11.h>

namespace tc
{

	class D3DTextureDebug {
	public:

		static void SaveAsDDS(ID3D11DeviceContext* device, ID3D11Texture2D* texture, const std::string& name);
		static void PrintTextureDesc(ID3D11Texture2D* texture);
	};

}
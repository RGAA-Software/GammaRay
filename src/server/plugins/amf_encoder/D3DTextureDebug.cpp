#include "D3DTextureDebug.h"

#include "tc_common_new/log.h"
#include "tc_common_new/win32/d3d_debug_helper.h"

namespace tc
{
	void D3DTextureDebug::SaveAsDDS(ID3D11DeviceContext* context, ID3D11Texture2D* texture, const std::string& name) {
        DebugOutDDS(texture, name);
	}

	void D3DTextureDebug::PrintTextureDesc(ID3D11Texture2D* texture) {
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		LOGI("-----------------ID3D11Texture2D Desc------------------");
		LOGI("Width : {} , Height : {}, Format : {}", desc.Width, desc.Height, desc.Format);
		LOGI("MipLevels : {}, ArraySize : {}", desc.MipLevels, desc.ArraySize);
		LOGI("Usage : {}", desc.Usage);
		LOGI("BindFlags : {}", desc.BindFlags);
		LOGI("CPUAccessFlags : {}", desc.CPUAccessFlags);
		LOGI("MiscFlags : {}", desc.MiscFlags);
		LOGI("SampleDesc : {}, {}", desc.SampleDesc.Count, desc.SampleDesc.Quality);
		LOGI("-----------------ID3D11Texture2D Desc------------------ END");
	}

}
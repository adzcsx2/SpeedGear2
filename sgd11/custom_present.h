#pragma once
#include<dxgi.h>
#ifdef __cplusplus
extern "C" {
#endif
	//�Զ���Present�ĸ��Ӳ���
	void CustomPresent(IDXGISwapChain*);
	void CustomResizeBuffers(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
#ifdef __cplusplus
}
#endif

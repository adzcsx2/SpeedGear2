#pragma once
#include<d3d9.h>
#ifdef __cplusplus
extern "C" {
#endif
	//�Զ���Present�ĸ��Ӳ���
	void CustomPresent(LPDIRECT3DDEVICE9);
	void CustomPresentEx(LPDIRECT3DDEVICE9EX);
	void CustomReset(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
	void CustomResetEx(LPDIRECT3DDEVICE9EX, D3DPRESENT_PARAMETERS*, D3DDISPLAYMODEEX*);
	void CustomSCPresent(LPDIRECT3DSWAPCHAIN9);
#ifdef __cplusplus
}
#endif

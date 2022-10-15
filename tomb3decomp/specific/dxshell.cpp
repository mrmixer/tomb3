#include "../tomb3/pch.h"
#include "dxshell.h"

long BPPToDDBD(long BPP)
{
	switch (BPP)
	{
	case 1:
		return DDBD_1;

	case 2:
		return DDBD_2;

	case 4:
		return DDBD_4;

	case 8:
		return DDBD_8;

	case 16:
		return DDBD_16;

	case 24:
		return DDBD_24;

	case 32:
		return DDBD_32;

	default:
		return 0;
	}
}

bool DXSetVideoMode(LPDIRECTDRAW2 ddx, long w, long h, long bpp)
{
	return ddx->SetDisplayMode(w, h, bpp, 0, 0) == DD_OK;
}

bool DXCreateSurface(LPDIRECTDRAW2 ddx, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE3 surf)
{
	LPDIRECTDRAWSURFACE s;
	HRESULT result;

	if (ddx->CreateSurface(desc, &s, 0) != DD_OK)
		return 0;

	result = s->QueryInterface(IID_IDirectDrawSurface3, (LPVOID*)surf);

	if (s)
		s->Release();

	return result == DD_OK;
}

bool DXGetAttachedSurface(LPDIRECTDRAWSURFACE3 surf, LPDDSCAPS caps, LPDIRECTDRAWSURFACE3* attached)
{
	return surf->GetAttachedSurface(caps, attached) == DD_OK;
}

bool DXAddAttachedSurface(LPDIRECTDRAWSURFACE3 surf, LPDIRECTDRAWSURFACE3 attach)
{
	return surf->AddAttachedSurface(attach) == DD_OK;
}

bool DXCreateDirect3DDevice(LPDIRECT3D2 dd3x, GUID guid, LPDIRECTDRAWSURFACE3 surf, LPDIRECT3DDEVICE2* device)
{
	return dd3x->CreateDevice(guid, (LPDIRECTDRAWSURFACE)surf, device) == DD_OK;
}

bool DXCreateViewPort(LPDIRECT3D2 dd3x, LPDIRECT3DDEVICE2 device, long w, long h, LPDIRECT3DVIEWPORT2* lpvp)
{
	D3DVIEWPORT2 vp;

	if (dd3x->CreateViewport(lpvp, 0) != DD_OK)
		return 0;

	if (device->AddViewport(*lpvp) != DD_OK)
		return 0;

	vp.dwSize = sizeof(D3DVIEWPORT2);
	vp.dwX = 0;
	vp.dwY = 0;
	vp.dwWidth = w;
	vp.dwHeight = h;
	vp.dvClipX = -1.0F;
	vp.dvClipY = (float)h / (float)w;
	vp.dvClipWidth = 2.0F;
	vp.dvClipHeight = vp.dvClipY + vp.dvClipY;
	vp.dvMinZ = 0;
	vp.dvMaxZ = 1.0F;

	if ((*lpvp)->SetViewport2(&vp) != DD_OK)
		return 0;

	device->SetCurrentViewport(*lpvp);
	return 1;
}

void DXGetSurfaceDesc(LPDIRECTDRAWSURFACE3 surf, LPDDSURFACEDESC desc)
{
	surf->GetSurfaceDesc(desc);
}

bool DXSurfBlt(LPDIRECTDRAWSURFACE3 surf, LPRECT rect, long FillColor)
{
	DDBLTFX bfx;

	memset(&bfx, 0, sizeof(DDBLTFX));
	bfx.dwSize = sizeof(DDBLTFX);
	bfx.dwFillColor = FillColor;
	return surf->Blt(rect, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bfx) == DD_OK;
}

void DXBitMask2ShiftCnt(ulong mask, uchar* shift, uchar* count)
{
	uchar i;

	for (i = 0; !(mask & 1); i++)
		mask >>= 1;

	*shift = i;

	for (i = 0; mask & 1; i++)
		mask >>= 1;

	*count = i;
}

bool DXCreateDirectDraw(DEVICEINFO* dev, DXCONFIG* conf, LPDIRECTDRAW2* ddx)
{
	LPDIRECTDRAW dd;
	HRESULT result;

	if (DirectDrawCreate(dev->DDInfo[conf->nDD].lpGuid, &dd, 0) != DD_OK)
		return 0;

	result = dd->QueryInterface(IID_IDirectDraw2, (LPVOID*)ddx);

	if (dd)
		dd->Release();

	return result == DD_OK;
}

bool DXCreateDirect3D(LPDIRECTDRAW2 ddx, LPDIRECT3D2* d3dx)
{
	return ddx->QueryInterface(IID_IDirect3D2, (LPVOID*)d3dx) == DD_OK;
}

bool DXSetCooperativeLevel(LPDIRECTDRAW2 ddx, HWND hwnd, long flags)
{
	return ddx->SetCooperativeLevel(hwnd, flags) == DD_OK;
}

__inline void* AddStruct(void* p, long num, long size)	//Note: this function wasn't present/was inlined in the original (taken from TR4)
{
	void* ptr;

	if (!num)
		ptr = MALLOC(size);
	else
		ptr = REALLOC(p, size * (num + 1));

	memset((char*)ptr + size * num, 0, size);
	return ptr;
}

BOOL CALLBACK DXEnumDirectInput(LPCDIDEVICEINSTANCE lpDevInst, LPVOID lpContext)
{
	DINPUTINFO* dinfo;
	DXDIRECTINPUTINFO* info;

	dinfo = (DINPUTINFO*)lpContext;
	dinfo->DIInfo = (DXDIRECTINPUTINFO*)AddStruct(dinfo->DIInfo, dinfo->nDIInfo, sizeof(DXDIRECTINPUTINFO));
	info = &dinfo->DIInfo[dinfo->nDIInfo];

	if (lpDevInst == (LPCDIDEVICEINSTANCE)-4)	//todo, fix me: properly check if guidInstance is valid
		info->lpGuid = 0;
	else
	{
		info->lpGuid = &info->Guid;
		info->Guid = lpDevInst->guidInstance;
	}

	lstrcpy(info->About, lpDevInst->tszProductName);
	lstrcpy(info->Name, lpDevInst->tszInstanceName);
	dinfo->nDIInfo++;
	return DIENUM_CONTINUE;
}

void inject_dxshell(bool replace)
{
	INJECT(0x0048FDB0, BPPToDDBD, replace);
	INJECT(0x0048FEE0, DXSetVideoMode, replace);
	INJECT(0x0048FF10, DXCreateSurface, replace);
	INJECT(0x0048FF60, DXGetAttachedSurface, replace);
	INJECT(0x0048FF80, DXAddAttachedSurface, replace);
	INJECT(0x0048FFA0, DXCreateDirect3DDevice, replace);
	INJECT(0x0048FFC0, DXCreateViewPort, replace);
	INJECT(0x004900B0, DXGetSurfaceDesc, replace);
	INJECT(0x004900C0, DXSurfBlt, replace);
	INJECT(0x0048F1C0, DXBitMask2ShiftCnt, replace);
	INJECT(0x0048FE40, DXCreateDirectDraw, replace);
	INJECT(0x0048FEA0, DXCreateDirect3D, replace);
	INJECT(0x0048FEC0, DXSetCooperativeLevel, replace);
	INJECT(0x0048ECE0, DXEnumDirectInput, replace);
}

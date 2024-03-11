// Dynamic_Wallpaper.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Dynamic_Wallpaper.h"
#include <Windows.h>
#include <mmsystem.h>
#include <digitalv.h>
#pragma comment(lib,"winmm.lib")

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HWND WorkerW;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM Lparam)
{
	if (FindWindowEx(hWnd, 0, L"SHELLDLL_DefView", 0))
	{
		WorkerW = FindWindowEx(0, hWnd, L"WorkerW", 0);
		ShowWindow(WorkerW, SW_HIDE);
		return FALSE;
	}
	return TRUE;
}

VOID SetWallpaperWindow(HWND hWnd)
{
	HWND Progman = FindWindowW(L"Progman", NULL);
	SendMessage(Progman, 0x52C, 0, 0);//使得桌面窗口分离
	SetParent(hWnd, Progman);//设置为底层窗口的子窗口
	EnumWindows(EnumWindowsProc, 0);//枚举到中间层窗口并隐藏
}

VOID ResetWallpaperWindow(HWND hWnd)
{
	SetParent(hWnd, 0);//恢复窗口
	ShowWindow(WorkerW, SW_SHOW);//重新显示中间层窗口
}

MCIERROR WINAPI dbg_mciSendCommandW(
	_In_ MCIDEVICEID mciId,
	_In_ UINT uMsg,
	_In_opt_ DWORD_PTR dwParam1,
	_In_opt_ DWORD_PTR dwParam2)
{
	MCIERROR mciErr = mciSendCommandW(mciId, uMsg, dwParam1, dwParam2);
	if (mciErr != 0)
	{
		wchar_t szErrorBuf[MAXERRORLENGTH];
		if (mciGetErrorStringW(mciErr, szErrorBuf, MAXERRORLENGTH))
		{
#ifdef _DEBUG
			OutputDebugStringW(L"[MCI] Error:");
			OutputDebugStringW(szErrorBuf);
			OutputDebugStringW(L"\n");
#else
			MessageBoxW(NULL, szErrorBuf, L"MCI Error", MB_ICONEXCLAMATION);
			exit(-1);
#endif // _DEBUG
		}
		else
		{
#ifdef _DEBUG
			OutputDebugStringW(L"[MCI] Error:Unknown Error\n");
#else
			MessageBoxW(NULL, L"Unknown Error", L"MCI Error", MB_ICONEXCLAMATION);
			exit(-1);
#endif // _DEBUG
		}
	}

	return mciErr;
}

MCIERROR WINAPI dbg_mciSendStringW(
	_In_ LPCWSTR lpstrCommand,
	_Out_writes_opt_(uReturnLength) LPWSTR lpstrReturnString,
	_In_ UINT uReturnLength,
	_In_opt_ HWND hwndCallback)
{
	MCIERROR mciErr = mciSendStringW(lpstrCommand, lpstrReturnString, uReturnLength, hwndCallback);

	if (mciErr != 0)
	{
		wchar_t szErrorBuf[MAXERRORLENGTH];
		if (mciGetErrorStringW(mciErr, szErrorBuf, MAXERRORLENGTH))
		{
#ifdef _DEBUG
			OutputDebugStringW(L"[MCI] Error:");
			OutputDebugStringW(szErrorBuf);
			OutputDebugStringW(L"\n");
#else
			MessageBoxW(NULL, szErrorBuf, L"MCI Error", MB_ICONEXCLAMATION);
			exit(-1);
#endif // _DEBUG
		}
		else
		{
#ifdef _DEBUG
			OutputDebugStringW(L"[MCI] Error:Unknown Error\n");
#else
			MessageBoxW(NULL, L"Unknown Error", L"MCI Error", MB_ICONEXCLAMATION);
			exit(-1);
#endif // _DEBUG
		}
	}

	return mciErr;
}


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_DYNAMICWALLPAPER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	HWND hWnd;
	if (!(hWnd = InitInstance(hInstance, nCmdShow)))
	{
		return FALSE;
	}

	//处理视频播放

	//打开文件
	WCHAR wcFileName[] = L"play.avi";
	WCHAR wcTypeName[] = L"MPEGVideo";
	MCI_OPEN_PARMSW mciOP = { .lpstrDeviceType = wcTypeName,.lpstrElementName = wcFileName };
	dbg_mciSendCommandW(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&mciOP);

	//设置播放窗口
	MCI_DGV_WINDOW_PARMS mciHwnd{ .hWnd = hWnd };
	dbg_mciSendCommandW(mciOP.wDeviceID, MCI_WINDOW, MCI_DGV_WINDOW_HWND, (DWORD_PTR)&mciHwnd);
	
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	MCI_DGV_PUT_PARMS mciPutParms{ .rc{ 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top} };

	dbg_mciSendCommandW(mciOP.wDeviceID, MCI_PUT, MCI_DGV_RECT | MCI_DGV_PUT_DESTINATION, (DWORD_PTR)&mciPutParms);

	//开始播放
	MCI_PLAY_PARMS mciPP{};
	dbg_mciSendCommandW(mciOP.wDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_DGV_PLAY_REPEAT, (DWORD_PTR)&mciPP);//MCI_WAIT


	MSG msg;
	{
		
		// 主消息循环:
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	

	mciSendCommandW(mciOP.wDeviceID, MCI_CLOSE, NULL, NULL);//关闭文件

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DYNAMICWALLPAPER));
	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DYNAMICWALLPAPER);
	wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP | WS_MAXIMIZE,
	  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);//创建一个全屏窗口

   if (!hWnd)
   {
	  return NULL;
   }

   ShowWindow(hWnd, SW_SHOW);
   UpdateWindow(hWnd);

   return hWnd;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SHORT W, H;
	

	switch (message)
	{
	case WM_CREATE:
		{
			SetWallpaperWindow(hWnd);
		}
		break;
	case WM_SIZE:
		W = LOWORD(lParam);
		H = HIWORD(lParam);

		//MCI_DGV_STATUS_PARMS MciStatusParams;
		//MciStatusParams.dwItem = MCI_DGV_STATUS_HWND;
		//dbg_mciSendCommandW(mciOP.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&MciStatusParams);
		//HWND hVideo = (HWND)MciStatusParams.dwReturn;
		//// 根据主窗体的大小调节视频窗口的大小
		//GetClientRect(hWnd, &ClientRect);
		//MoveWindow(hVideo, 5, 5, ClientRect.right - ClientRect.left - 10, ClientRect.bottom - ClientRect.top - 10, TRUE);

		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: 在此处添加使用 hdc 的任何绘图代码...
			//RECT rect = { 0,0,W,H };
			//FillRect(hdc, &rect, GetSysColorBrush(1));

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		ResetWallpaperWindow(hWnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

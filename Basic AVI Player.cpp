#define BTN_LOADAVI 1
#define BTN_START 2
#define BTN_STOP 3
#define FILE_MENU_EXIT 4

#include "header.h"
#include "Basic AVI Player.h"
#include <Windows.h>
#include <CommCtrl.h>
//#include <windowsx.h>
//#include <WinUser.h>


#pragma comment (lib, "comctl32.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DiagWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void loadMainUI(HWND hwnd);
void DisplayProgress(HWND hwnd);
void registerProgressDiag(HINSTANCE hInstance);
void loadAVI(HWND hwnd);
void loadDiagUI(HWND hwnd);

HWND hEdit, hAnimation, hBtnStart, hBtnStop, hProgressDiag;
int pathLength;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdShow, int nCmdShow) {

	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = ICC_WIN95_CLASSES;

	if (!InitCommonControlsEx(&icce)) {
		MessageBox(NULL, L"InitCommonControlsEx function failed.", L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}


	const wchar_t CLASS_NAME[] = L"Basic AVI Player";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, L"Window Registration failed.", L"Error", MB_OK);
	}

	registerProgressDiag(hInstance);

	HWND hMainWindow = CreateWindowEx(0, CLASS_NAME, L"Basic AVI Player", WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 700, 410, NULL, NULL, NULL, NULL);

	if (hMainWindow == NULL) {
		MessageBox(NULL, L"Window Creation failed.", L"Error", MB_OK);
	}

	ShowWindow(hMainWindow, nCmdShow);


	MSG msg = {};

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {

	case WM_CREATE:
	{
		loadMainUI(hwnd);
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	break;

	case WM_COMMAND:
	{
		switch (wParam) {
			case BTN_LOADAVI:
			{
				SetTimer(hwnd, 9, 10000, NULL);
				DisplayProgress(hwnd);
				return 0;
			}
			break;

			case BTN_START:
			{
				BOOL result = Animate_Play(hAnimation, 0, -1, -1);
				if (result == FALSE) {
					MessageBox(hwnd, L"AVI Failed to start.", L"Error", MB_OK | MB_ICONERROR);
				}
			}
			break;

			case BTN_STOP:
			{
				BOOL result = Animate_Stop(hAnimation);
				if (result == FALSE) {
					MessageBox(hwnd, L"AVI Failed to stop.", L"Error", MB_OK | MB_ICONERROR);
				}
			}
			break;

			case FILE_MENU_EXIT:
			{
				SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			}
			break;
		}
	}
	break;

	case WM_TIMER:
	{
		KillTimer(hwnd, 9);
		DestroyWindow(hProgressDiag);
		loadAVI(hwnd);
	}
	break;

	case WM_CLOSE:
	{
		int result = MessageBox(hwnd, L"Are you sure you want to close?", L"Close?", MB_OKCANCEL | MB_ICONQUESTION);

		if (result == IDOK) {
			DestroyWindow(hAnimation);
			DestroyWindow(hwnd);
		}
		return 0;
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



void loadMainUI(HWND hwnd) {

	HMENU hMainMenu = CreateMenu();
	HMENU hFileMenu = CreatePopupMenu();
	AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_EXIT, L"Exit");

	SetMenu(hwnd, hMainMenu);

	HWND hStatic = CreateWindowEx(0, L"Static", L"File Path:", WS_VISIBLE | WS_CHILD, 20, 10, 70, 30, hwnd, NULL, NULL,
		NULL);
	hEdit = CreateWindowEx(0, L"Edit", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 10, 450, 20, hwnd, NULL, NULL,
		NULL);
	hAnimation = CreateWindowEx(0, L"SysAnimate32", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, 190, 60, 220, 140, hwnd,
		NULL, NULL, NULL);
	HWND hBtnLoad = CreateWindowEx(0, L"BUTTON", L"Load AVI", WS_CHILD | WS_VISIBLE, 530, 40, 100, 30, hwnd,
		(HMENU)BTN_LOADAVI, NULL, NULL);
		//Button_SetElevationRequiredState(hBtnLoad, true);
	hBtnStart = CreateWindowEx(0, L"Button", L"Start", WS_CHILD | WS_VISIBLE, 30, 280, 100, 30, hwnd, (HMENU)BTN_START,
		NULL, NULL);
	hBtnStop = CreateWindowEx(0, L"Button", L"Stop", WS_CHILD | WS_VISIBLE, 530, 280, 100, 30, hwnd, (HMENU)BTN_STOP, NULL,
		NULL);

	EnableWindow(hBtnStart, false);
	EnableWindow(hBtnStop, false);

}



void registerProgressDiag(HINSTANCE hInstance) {

	const wchar_t PROGDIAG[] = L"Progress Diag";

	WNDCLASS wndClass = {};

	wndClass.lpfnWndProc = DiagWndProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = PROGDIAG;

	if (!RegisterClass(&wndClass)) {
		MessageBox(NULL, L"Progress dialog Registration failed.", L"Error", MB_OK);
	}


}
void DisplayProgress(HWND hwnd) {

	hProgressDiag = CreateWindowEx(0, L"Progress Diag", L"Loading", WS_VISIBLE | WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX,
		270, 270, 470, 170, hwnd, NULL, NULL, NULL);
	loadDiagUI(hProgressDiag);
}

void loadDiagUI(HWND hwnd) {

	HWND hStatic = CreateWindowEx(0, L"Static", L"Loading AVI File...", WS_CHILD | WS_VISIBLE, 30, 20, 130, 30, hwnd,
		NULL, NULL, NULL);
	HWND hProgressBar = CreateWindowEx(0, L"msctls_progress32", NULL, WS_CHILD | WS_VISIBLE |PBS_MARQUEE, 33, 60, 390, 30, hwnd,
		NULL, NULL, NULL);
	SendMessage(hProgressBar, PBM_SETMARQUEE, (WPARAM) 1, NULL);
}


void loadAVI(HWND hwnd) {

	int length = GetWindowTextLength(hEdit);  //  C:\Users\opiyo\Downloads\FlickAnimation.avi

	if (length > 0) {
		length++;
		pathLength = length;
		wchar_t *filePath = new wchar_t[length];
		GetWindowText(hEdit, filePath, length);
		//SendMessage(hAnimation, ACM_OPEN, NULL, (WPARAM)filePath);
		BOOL result = Animate_Open(hAnimation, filePath);
		if (result == FALSE) {
			MessageBox(hwnd, L"AVI file failed to load.", L"Error", MB_OK | MB_ICONERROR);
			//return 0;
		}

		EnableWindow(hBtnStart, true);
		EnableWindow(hBtnStop, true);
	}

}


LRESULT CALLBACK DiagWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {

		case WM_CREATE:
		{
			//loadMainUI(hwnd);
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);
		}
		break;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}







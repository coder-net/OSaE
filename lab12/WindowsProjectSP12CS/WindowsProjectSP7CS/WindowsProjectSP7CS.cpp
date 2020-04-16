// WindowsProjectSP7CS.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProjectSP7CS.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
CRITICAL_SECTION cs;
HANDLE threads[3];
HWND main_hwnd;
bool flag = false;

struct ThreadArgs
{
	int x, y, event_handle, next_event_handle;
} args[3];

void debug(int lol)
{
	char b[15];
	_itoa_s((int)lol, b, 15, 10);
	SetWindowTextA(main_hwnd, b);
}

/*Threads that are waiting on a critical section are added to a wait queue; they are
woken and generally acquire the critical section in the order in which they were added
to the queue. However, if threads are added to this queue at a fast enough rate, performance
can be degraded because of the time it takes to awaken each waiting thread.*/
DWORD WINAPI ThreadProc(ThreadArgs* arg)
{
	while (true)
	{
		EnterCriticalSection(&cs);
		if (arg->x == 290) arg->x = 0;
		arg->x += 10;
		
		InvalidateRect(main_hwnd, NULL, true);
		LeaveCriticalSection(&cs);
		Sleep(100);
	}
	return 1;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECTSP7CS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECTSP7CS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECTSP7CS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECTSP7CS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   main_hwnd = hWnd;
   args[0].y = 10;
   args[1].y = 110;
   args[2].y = 210;

   args[0].x = 0;
   args[1].x = 0;
   args[2].x = 0;

   args[0].next_event_handle = 1;
   args[1].next_event_handle = 2;
   args[2].next_event_handle = 0;

   args[0].event_handle = 0;
   args[1].event_handle = 1;
   args[2].event_handle = 2;
   //SetTimer(hWnd, NULL, 20, NULL);
   InitializeCriticalSectionAndSpinCount(&cs, 0x80000400);


   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
				if (!flag)
				{
					flag = true;
					for (int i = 0; i < 3; i++)
					{
						threads[i] = CreateThread(
							NULL,              // default security
							0,                 // default stack size
							(LPTHREAD_START_ROUTINE)ThreadProc,        // name of the thread function
							&(args[i]),              //  thread parameters
							0,                 // default startup flags
							NULL);
					}
					
				}
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
			if (flag)
			{
				HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
				HPEN pen = CreatePen(BS_SOLID, 2, RGB(0, 0, 255));
				SelectObject(hdc, pen);
				SelectObject(hdc, brush);
				for (int i = 0; i < 3; i++)
				{
					Rectangle(hdc, args[i].x, args[i].y, args[i].x + 50, args[i].y + 30);
					Rectangle(hdc, args[i].x + 52, args[i].y, args[i].x + 75, args[i].y + 30);
					Rectangle(hdc, args[i].x + 75, args[i].y + 15, args[i].x + 100, args[i].y + 30);
					Rectangle(hdc, args[i].x + 85, args[i].y + 7, args[i].x + 90, args[i].y + 15);
				}
			}
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// lab13.cpp : Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "lab13.h"
#include <TlHelp32.h>
#include <windowsx.h>

#define MAX_LOADSTRING 100
#define LB_MAX_COUNT 500

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
// My global vriables
HWND listbox1, listbox2;
HMENU hPopupMenu;

int processCount = 0;
PROCESSENTRY32 processes[LB_MAX_COUNT];

int moduleCount = 0;
MODULEENTRY32 modules[LB_MAX_COUNT];


// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB13, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB13));

    MSG msg;

    // Цикл основного сообщения:
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
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB13));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB13);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

#define LISTBOX1 2001
#define LISTBOX2 2002

void ClearListbox(HWND lb) {
	while (SendMessage(lb, LB_GETCOUNT, 0, NULL))
		SendMessage(lb, LB_DELETESTRING, 0, NULL);
}

void ShowProcesses()
{
	HANDLE th = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	processCount = 0;
	processes[0].dwSize = sizeof(PROCESSENTRY32);
	BOOL result = Process32First(th, processes);
	while (result) {
		processes[++processCount].dwSize = sizeof(PROCESSENTRY32);
		result = Process32Next(th, processes + processCount);
	}

	CloseHandle(th);

	ClearListbox(listbox1);
	for (int i = 0; i < processCount; ++i)
		SendMessage(listbox1, LB_ADDSTRING, 0, (LPARAM)processes[i].szExeFile);
}

void ShowModules(PROCESSENTRY32 process)
{
	HANDLE th = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process.th32ProcessID);

	moduleCount = 0;
	modules[0].dwSize = sizeof(MODULEENTRY32);
	BOOL result = Module32First(th, modules);
	while (result) {
		modules[++moduleCount].dwSize = sizeof(MODULEENTRY32);
		result = Module32Next(th, modules + moduleCount);
	}

	CloseHandle(th);

	ClearListbox(listbox2);
	for (int i = 0; i < moduleCount; ++i)
		SendMessage(listbox2, LB_ADDSTRING, 0, (LPARAM)modules[i].szModule);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   listbox1 = CreateWindow(L"LISTBOX", L"Processes", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
	   20, 20, 250, 500, hWnd, (HMENU)LISTBOX1, hInstance, NULL);
   listbox2 = CreateWindow(L"LISTBOX", L"Modules", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
	   300, 20, 250, 500, hWnd, (HMENU)LISTBOX2, hInstance, NULL);

   ShowProcesses();

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

#define SET_PRIORITY_IDLE 2010
#define SET_PRIORITY_NORMAL 2011
#define SET_PRIORITY_HIGH 2012
#define SET_PRIORITY_REAL_TIME 2013

void SetPriority(DWORD priorityClass) {
	INT position = SendMessage(listbox1, LB_GETCURSEL, 0, NULL);
	if (position != -1) {
		HANDLE selectedProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[position].th32ProcessID);
		SetPriorityClass(selectedProcess, priorityClass);
		CloseHandle(selectedProcess);
	}
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
			case LISTBOX1:
				if (wmEvent == LBN_SELCHANGE) {
					int position = SendMessage(listbox1, LB_GETCURSEL, 0, NULL);
					if (position != -1) {
						ShowModules(processes[position]);
					}
				}
				break;
			case SET_PRIORITY_IDLE:
				SetPriority(IDLE_PRIORITY_CLASS);
				break;
			case SET_PRIORITY_NORMAL:
				SetPriority(NORMAL_PRIORITY_CLASS);
				break;
			case SET_PRIORITY_HIGH:
				SetPriority(HIGH_PRIORITY_CLASS);
				break;
			case SET_PRIORITY_REAL_TIME:
				// pass
				// нельзя установить для прикладной программы приоритет REAL_TIME
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CONTEXTMENU:
		if ((HWND)wParam == listbox1) {
			int position = SendMessage(listbox1, LB_GETCURSEL, 0, NULL);
			if (position != -1) {
				// Get priority of selected process
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[position].th32ProcessID);
				int pC = GetPriorityClass(hProcess);
				CloseHandle(hProcess);

				hPopupMenu = CreatePopupMenu();
				AppendMenu(hPopupMenu, pC == IDLE_PRIORITY_CLASS ? MF_CHECKED : MF_STRING, SET_PRIORITY_IDLE, L"Idle");
				AppendMenu(hPopupMenu, pC == NORMAL_PRIORITY_CLASS ? MF_CHECKED : MF_STRING, SET_PRIORITY_NORMAL, L"Normal");
				AppendMenu(hPopupMenu, pC == HIGH_PRIORITY_CLASS ? MF_CHECKED : MF_STRING, SET_PRIORITY_HIGH, L"High");
				AppendMenu(hPopupMenu, pC == REALTIME_PRIORITY_CLASS ? MF_CHECKED : MF_STRING, SET_PRIORITY_REAL_TIME, L"Real time");
				TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, hWnd, NULL);
			}
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
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

// Обработчик сообщений для окна "О программе".
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

#include "stdafx.h"
#include "PassGenerator.h"

#define MAX_LOADSTRING 100

#define USE_NUMBERS 1000
#define USE_SYMBOLS 1001
#define USE_LETTERS 1002
#define GENERATE    1003
#define TEXTPASS    1004

#define MIN_PASS_LENGTH 15
#define MAX_PASS_LENGTH 25

#define CreateWin(Text, Style, x, y, posX, posY,hMenu) CreateWindow(_T("button"), Text, Style, x, y, posX, posY, \
hWnd, hMenu, hInstance, NULL);

#define SET_FLAG(USE_CONSTANT, USE_FLAG) hwndCheck = GetDlgItem(hWnd, USE_CONSTANT); \
flags = SendMessage(hwndCheck, BM_GETCHECK, 0, 0) == BST_CHECKED ? flags |= USE_FLAG : flags ^= USE_FLAG;

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
DWORD flags = 0;

struct HWNDSTRUCT
{
	HWND hLetter;
	HWND hNumbers;
	HWND hSymbol;
	HWND hGenerate;
	HWND hPass;
} HWNDSTRUCT;

enum SYMBOLS
{
	USE_NUMBER = 1,
	USE_SYMBOL = 2,
	USE_LETTER = 4,
};

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
BOOL                InitButtons(HINSTANCE , HWND);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
WCHAR*				GeneratePassword(DWORD);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PASSGENERATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	srand(time(NULL));

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PASSGENERATOR));

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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PASSGENERATOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
      CW_USEDEFAULT, 0, 285, 300, nullptr, nullptr, hInstance, nullptr);

   if (InitButtons(hInstance, hWnd) && !hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL InitButtons(HINSTANCE hInstance, HWND hWnd)
{
	HWNDSTRUCT.hLetter = CreateWin(_T("Use letter"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 10, 125, 30, (HMENU)USE_LETTERS);
	HWNDSTRUCT.hNumbers = CreateWin(_T("Use numbers"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 135, 10, 125, 30, (HMENU)USE_NUMBERS);
	HWNDSTRUCT.hSymbol = CreateWin(_T("Use symbols"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 40, 125, 30, (HMENU)USE_SYMBOLS);
	HWNDSTRUCT.hGenerate = CreateWin(_T("Generate password"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 285 / 2 - 75, 300 / 2 - 50, 130, 30, (HMENU)GENERATE);

	HWNDSTRUCT.hPass = CreateWindow(_T("EDIT"), _T(""), WS_BORDER | WS_CHILD | WS_VISIBLE, 20, 300 / 2, 230, 20,
		hWnd, (HMENU)TEXTPASS, hInstance, NULL);

	//Проверка на то, все ли контролы успешно созданы
	return HWNDSTRUCT.hSymbol 
		&& HWNDSTRUCT.hNumbers 
		&& HWNDSTRUCT.hLetter 
		&& HWNDSTRUCT.hGenerate 
		&& HWNDSTRUCT.hPass;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
			HWND hwndCheck = NULL;
			LRESULT res = 0;
			WCHAR* pass = NULL;
            int wmId = LOWORD(wParam);
            
            switch (wmId)
            {
			case USE_NUMBERS:
				SET_FLAG(USE_NUMBERS, USE_NUMBER);
				break;
			case USE_LETTERS:
				SET_FLAG(USE_LETTERS, USE_LETTER);
				break;
			case USE_SYMBOLS:
				SET_FLAG(USE_SYMBOLS, USE_SYMBOL);
				break;
			case GENERATE:
			    pass = GeneratePassword(flags);

				if (!pass)
				{
					MessageBox(NULL, _T("Incorrect generation settings"), _T("Error"), MB_ICONERROR);
				}
				else
				{
					SetWindowText(HWNDSTRUCT.hPass, pass);
					delete[] pass;
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

WCHAR* GeneratePassword(DWORD flag)
{
	if (!flag)
	{
		return NULL;
	}

	char s_latin[] = {
		'A','b','C','d','E','f','G','H','i','J','k','L','m','N','o','P','q','R','s','T','u','V','w','X','y','Z', 
		'a','B','c','D','e','F','g','h','I','j','K','l','M','n','O','p','Q','r','S','t','U','v','W','x','Y','z' };

	char s_esc[] = {
		'!','@','#','$','%','^','&','*','(',')','-','=','+','.',',',':',';','"','?'};

	char s_num[] = {
		'1','2','3','4','5','6','7','8','9','0'};

	CONST DWORD len = MIN_PASS_LENGTH + rand() % (MAX_PASS_LENGTH - MIN_PASS_LENGTH);
	CHAR* pass = new CHAR[len + 1];
	DWORD i = 0;

	while (i < len)
	{
		int nrand = rand() % 3 + 1;

		if (nrand == 1 && flag & USE_LETTER)
		{
			pass[i] = s_latin[rand() % sizeof(s_latin)];
		}
		else if (nrand == 2 && flag & USE_NUMBER)
		{
			pass[i] = s_num[rand() % sizeof(s_num)];
		}
		else if (nrand == 3 && flag & USE_SYMBOL)
		{
			pass[i] = s_esc[rand() % sizeof(s_esc)];
		}
		else
		{
			/*Если произошла некорректная генерация из за настроек,
			  тогда пытаемся сгенерировать значение сначала.*/ 
			continue;
		}
		++i;
	}
	pass[len] = '\0';

	WCHAR* wPass = new WCHAR[len + 1];
	size_t outSize;
	mbstowcs_s(&outSize, wPass, len + 1, pass, len);
	delete[] pass;

	return wPass;
}
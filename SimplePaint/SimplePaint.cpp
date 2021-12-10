// SimplePaint.cpp : Defines the entry point for the application.
// Em xin demo project2, simple paint !! 

#include "framework.h"
#include "SimplePaint.h"
#include <WindowsX.h>
#include <vector>
#include <fstream>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#include <commdlg.h>
#include <sstream>

using namespace std;
#define MAX_LOADSTRING 100

#define IMAGE_WIDTH     18
#define IMAGE_HEIGHT    18
#define BUTTON_WIDTH    0
#define BUTTON_HEIGHT   0

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

CHOOSECOLOR  cc; // Thông tin màu chọn
COLORREF  acrCustClr[16]; // Mảng custom color
DWORD  rgbCurrent = RGB(255, 0, 0); // Red
HBRUSH  hbrush; // Tạo ra brush từ màu đã chọn

HWND hwnd2;
Point p1, p2; // Điểm đầu và cuối để vẽ các đối tượng
ShapeMode currShapeMode = DRAWLINE; // Xác định đối tượng vẽ đang chọn hiện tại
vector<CShape*> shapes; //Mảng các đối tượng
CShape* currShape = NULL;
bool isPreview = false; //Trạng thái của chuột
int currPenSize = 2;
int currPenStyle = PS_SOLID;
HPEN hPen = CreatePen(currPenStyle, currPenSize, rgbCurrent);
string defaultFilePath = "Untitled.bin";
stringstream fileName;
int countFileSaved = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OnDestroy(HWND hwnd);
void OnPaint(HWND hwnd);
void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);

void initNewShape();
void saveToBinaryFile(string filePath);
void loadFromBinaryFile(string filePath);
void openFileDialog();
void saveFileDialog();

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
    LoadStringW(hInstance, IDC_SIMPLEPAINT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEPAINT));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEPAINT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIMPLEPAINT);
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
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLButtonUp);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	hwnd2 = hwnd;
	//Toolbar
	InitCommonControls();
	TBBUTTON tbButtons[] =
	{
		{ STD_FILENEW,	ID_FILE_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
		{ STD_FILEOPEN,	ID_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
		{ STD_FILESAVE,	ID_FILE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }
	};

	HWND hToolBarWnd = CreateToolbarEx(hwnd,
		WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR, sizeof(tbButtons) / sizeof(TBBUTTON), HINST_COMMCTRL,
		0, tbButtons, sizeof(tbButtons) / sizeof(TBBUTTON),
		BUTTON_WIDTH, BUTTON_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT,
		sizeof(TBBUTTON));

	TBBUTTON userButtons[] =
	{
		{ 0, 0,	TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0 },
		{ 0, ID_DRAW_ELLIPSE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
		{ 1, ID_DRAW_RECTANGLE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
		{ 2, ID_DRAW_LINE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	};
	TBADDBITMAP	tbBitmap = { hInst, IDB_BITMAP1 };
	// Thêm danh sách ảnh vào toolbar
	int idx = SendMessage(hToolBarWnd, TB_ADDBITMAP, (WPARAM) sizeof(tbBitmap) / sizeof(TBADDBITMAP),
		(LPARAM)(LPTBADDBITMAP)&tbBitmap);
	// Xác định chỉ mục hình ảnh c ủa mỗi button từ ảnh bự liên hoàn nhiều tấm
	userButtons[1].iBitmap += idx;
	userButtons[2].iBitmap += idx;
	userButtons[3].iBitmap += idx;
	// Thêm nút bấm vào toolbar
	SendMessage(hToolBarWnd, TB_ADDBUTTONS, (WPARAM) sizeof(userButtons) / sizeof(TBBUTTON),
		(LPARAM)(LPTBBUTTON)&userButtons);

	return TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case ID_FILE_NEW:
		shapes.clear();
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case ID_FILE_OPEN:
		openFileDialog();
		break;
	case ID_FILE_SAVE:
		fileName.str("");
		fileName << "Untitled" << countFileSaved << ".bin";
		saveToBinaryFile(fileName.str());
		countFileSaved++;
		break;
	case ID_FILE_SAVEAS:
		saveFileDialog();
		break;
	case ID_CHOOSE_COLOR:
		ZeroMemory(&cc, sizeof(CHOOSECOLOR));
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = hwnd;
		cc.lpCustColors = (LPDWORD)acrCustClr;
		cc.rgbResult = rgbCurrent;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;
		if (ChooseColor(&cc))
		{
			hbrush = CreateSolidBrush(cc.rgbResult);
			rgbCurrent = cc.rgbResult;
		}
		break;
	case ID_DRAW_LINE:
		currShapeMode = DRAWLINE;
		break;
	case ID_DRAW_RECTANGLE:
		currShapeMode = DRAWRECT;
		break;
	case ID_DRAW_ELLIPSE:
		currShapeMode = DRAWELLIPSE;
		break;
	case IDM_EXIT:
		DestroyWindow(hwnd);
		break;
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;
	}
}

void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	for (int i = 0; i < shapes.size(); i++)
	{
		hPen = CreatePen(shapes[i]->getPenStyle(), currPenSize, shapes[i]->getColor());
		SelectObject(hdc, hPen);
		shapes[i]->Draw(hdc);
	}

	if (isPreview)
	{
		//Vẽ nét vẽ tạm thời khi chưa Mouse Up
		hPen = CreatePen(currPenStyle, currPenSize, rgbCurrent);
		SelectObject(hdc, hPen);
		currShape->SetData(p2.x, p2.y, p1.x, p1.y, rgbCurrent, currPenStyle);
		currShape->Draw(hdc);
	}

	EndPaint(hwnd, &ps);
}

void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	//isPreview = true;
	//fromX = x;
	//fromY = y;
	//HDC hdc = GetDC(hwnd);
	////SetPixel(hdc, x, y, RGB(255, 0, 0));
	//MoveToEx(hdc, x, y, NULL);
	//ReleaseDC(hwnd, hdc);
	if (!isPreview)
	{
		p1 = newPoint(x, y);
		initNewShape();
		isPreview = true;
	}
}

void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	isPreview = false;
	p2 =  newPoint(x, y);

	//Vẽ chính thức 
	switch (currShapeMode)
	{
	case  DRAWLINE:
	{
		 CLine* line = new  CLine;
		line->SetData(p2.x, p2.y, p1.x, p1.y, rgbCurrent, currPenStyle);
		shapes.push_back(line);
	}
	break;
	case  DRAWRECT:
	{
		 CRectangle* rect = new  CRectangle;
		rect->SetData(p1.x, p1.y, p2.x, p2.y, rgbCurrent, currPenStyle);
		shapes.push_back(rect);
	}
	break;
	case  DRAWELLIPSE:
	{
		 CEllipse* ellipse = new  CEllipse;
		ellipse->SetData(p1.x, p1.y, p2.x, p2.y, rgbCurrent, currPenStyle);
		shapes.push_back(ellipse);
	}
	break;
	default:
		MessageBox(hwnd, L"Lỗi", L"Thông báo", MB_OK | MB_ICONERROR);
	}

	InvalidateRect(hwnd, NULL, TRUE);
}

void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (isPreview)
	{
		p2 = newPoint(x, y);
		InvalidateRect(hwnd, NULL, TRUE); 
	}
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

void initNewShape()
{
	switch (currShapeMode)
	{
	case DRAWLINE:
		currShape = new CLine();
		break;
	case DRAWRECT:
		currShape = new CRectangle();
		break;
	case DRAWELLIPSE:
		currShape = new CEllipse();
		break;
	default:
		break;
	}
}

void saveToBinaryFile(string filePath) {
	 ofstream out;
	out.open(filePath,  iostream::out |  iostream::binary |  iostream::trunc);

	if (out.is_open()) {
		int size = shapes.size();
		out.write(reinterpret_cast<const char *>(&size), sizeof(size));

		for ( CShape* shape : shapes) {
			int type = shape->getType();
			COLORREF color = shape->getColor();
			RECT* rect = shape->getDimens();
			out.write(reinterpret_cast<const char*>(&type), sizeof(type));
			out.write(reinterpret_cast<const char*>(&color), sizeof(COLORREF));
			out.write(reinterpret_cast<const char*>(rect), sizeof(RECT));
		}
	}
	else {
		OutputDebugString(L"Can't open data.bin to write");
	}

	out.close();
}

void loadFromBinaryFile(string filePath) {
	 ifstream in;
	in.open(filePath,  iostream::in |  iostream::binary);

	if (in.is_open()) {
		char* buffer = new char[255];
		int size;
		in.read(buffer, sizeof(size));

		size = buffer[0];
		shapes.clear();


		for (int i = 0; i < size; i++)
		{
			char* item_buff = new char[255];

			 CShape* shape = NULL;

			int type;
			COLORREF color;
			in.read(item_buff, sizeof(type));
			type = item_buff[0];
			in.read(item_buff, sizeof(COLORREF));
			color = item_buff[0];
			int r = GetRValue(color);
			color = item_buff[1];
			int g = GetGValue(color);
			color = item_buff[2];
			int b = GetBValue(color);

			color = RGB(r, g, b);

			switch (type)
			{
			case  DRAWLINE:
				shape = new  CLine();
				break;
			case  DRAWRECT:
				shape = new  CRectangle();
				break;
			case  DRAWELLIPSE:
				shape = new  CEllipse();
				break;
			default:
				break;
			}

			shape->setType(( ShapeMode)type);
			shape->setColor(color);

			RECT* rect;
			in.read(item_buff, sizeof(RECT));
			rect = reinterpret_cast<RECT*>(item_buff);
			shape->setDimens(rect);

			shapes.push_back(shape);

			delete[] item_buff;
			item_buff = NULL;
		}

		delete[] buffer;
	}
	else {

		OutputDebugString(L"Can't open data.bin to read");
	}

	in.close();
}

void openFileDialog()
{
	OPENFILENAME ofn;
	WCHAR szFilePath[MAX_PATH] = L"";
	WCHAR szFileTitle[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = hwnd2;
	ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFilePath;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"bin";

	if (GetOpenFileName(&ofn))
	{
		// Do something usefull with the filename stored in szFileName 
		wstring ws(szFilePath);
		// your new String
		string fileName(ws.begin(), ws.end());
		loadFromBinaryFile(fileName);
		InvalidateRect(hwnd2, NULL, TRUE);
	}
}

void saveFileDialog()
{
	OPENFILENAME ofn;
	WCHAR szFileName[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = hwnd2;
	ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"bin";

	if (GetSaveFileName(&ofn))
	{
		// Do something usefull with the filename stored in szFileName 
		wstring ws(szFileName);
		// your new String
		string fileName(ws.begin(), ws.end());

		defaultFilePath = fileName;
		saveToBinaryFile(fileName);
	}
}
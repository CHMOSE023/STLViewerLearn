#include "STLWindows.h" 
#include "Windows.h"
#include "OpenGLDC.h"

STLWindows::STLWindows() :m_hWnd(NULL), m_makeReuslt(false), m_OpenGLDC(nullptr)
{
	m_OpenGLDC = new OpenGLDC;
}

STLWindows::~STLWindows() 
{
	if (m_OpenGLDC)
	{
		DestroyWindow(m_hWnd);
		delete m_OpenGLDC;
		m_OpenGLDC = nullptr;
	}
}

BOOL STLWindows::Initialize(int width, int height, const wchar_t* title)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));

	WNDCLASSEXW wcex = {0};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = hBlackBrush;
	wcex.lpszClassName = title;

	RegisterClassExW(&wcex);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int posX = (screenWidth - width) / 2;
	int posY = (screenHeight - height) / 2;

	m_hWnd = CreateWindow(
		title,                    
		title,                   
		WS_OVERLAPPEDWINDOW,
		posX, posY,
		width, height,
		nullptr,
		nullptr,
		hInstance,                
		this
	);

	 if (!m_hWnd)
	 {
		 return FALSE;
	 }

	 ShowWindow(m_hWnd, SW_SHOW);        
	 UpdateWindow(m_hWnd);               

	 return TRUE;
}

LRESULT STLWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	{
		if (WM_CLOSE == message)
		{
			PostQuitMessage(0);
		}

		if (WM_CREATE == message)
		{
			CREATESTRUCT* pSTRUCT = (CREATESTRUCT*)lParam;
			STLWindows* pApp = (STLWindows*)pSTRUCT->lpCreateParams;

			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pApp);
			return  pApp->EventProc(hWnd, WM_CREATE, wParam, lParam);
		}
		else
		{
			STLWindows* pApp = (STLWindows*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (pApp)
			{
				return  pApp->EventProc(hWnd, message, wParam, lParam);
			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
	}
}

LRESULT STLWindows::EventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		m_makeReuslt = m_OpenGLDC->Initialize(hWnd);   
		break;
	case WM_SIZE:
		m_OpenGLDC->Resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		m_OpenGLDC->OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		m_OpenGLDC->OnLButtonUp();
		break;
	case WM_MOUSEMOVE:
		m_OpenGLDC->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL:
		m_OpenGLDC->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	case WM_KEYDOWN:
		
		break;
	case WM_KEYUP:
	
		break; 
	case WM_DESTROY:	
		m_OpenGLDC->Shutdown();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return S_OK;
}

void STLWindows::Run()
{  
	MSG msg = { 0 };
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg); 
		}
		else
		{ 
			m_OpenGLDC->Render(); 	// 无消息时渲染，保证画面持续更新
			
		}
	}
}

 
#include "app.h"

namespace {
	const auto ClassName = TEXT("SampleWindowClass");
}

/* constructor */
App::App(uint32_t width, uint32_t height)
	:m_hInst (nullptr)
	,m_hWnd (nullptr)
	,m_Width (width)
	,m_Height (height)
{/* do nothing */ }

App::~App()
{/* do nothing */ }

void App::Run() {
	if (InitApp()) {
		MainLoop();
	}
	TermApp();
}

bool App::InitApp() {
	if (!InitWnd()) {
		return false;
	}
	return true;
}

bool App::InitWnd() {
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		return false;
	}

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	// register window
	if (!RegisterClassEx(&wc)) {
		return false;
	}
	m_hInst = hInst;

	// size of window
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// create window
	m_hWnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("Sample"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_hInst,
		nullptr
	);
	
	if (m_hWnd == nullptr) {
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	UpdateWindow(m_hWnd);

	SetFocus(m_hWnd);

	return true;
}

// window procedure
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_DESTROY:
	{PostQuitMessage(0);}
	break;

	default : 
	{/* do nothing */}
	break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

// main loop
void App::MainLoop() {
	MSG msg = {};
	
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

// quit
void App::TermApp() {
	TermWnd();
}

void App::TermWnd() {
	if (m_hInst != nullptr) {
		UnregisterClass(ClassName, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}
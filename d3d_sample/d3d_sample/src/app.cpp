#include "app.h"
#include "assert.h"

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

template<typename T>
void SafeRelease(T*& ptr) {
	if (ptr != nullptr) {
		ptr->Release();
		ptr = nullptr;
	}
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
void App::MainLoop()
{
	MSG msg = {};
	
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

// quit
void App::TermApp()
{
	TermWnd();
}

void App::TermWnd()
{
	if(m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}

// initialize d3d
bool App::InitD3D()
{
	// debug flag
	#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

		// activate debug layer
		if (SUCCEEDED(hr)) debug->EnableDebugLayer();

	}
	#endif
	// create device
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(m_pDevice.GetAddressOf())
	);
	if (FAILED(hr)) {
		return false;
	}

	// create command queue
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
		if (FAILED(hr)) {
			return false;
		}
	}


	// setting for swap-chain
	{
		// create DXGI factory
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) {
			return false;
		}
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// create swap-chain
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			return false;
		}
		// get IDXGISwapChain3
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			SafeRelease(pFactory);
			return false;
		}

		// get back-buffer index
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
		SafeRelease(pFactory);
		SafeRelease(pSwapChain);

	}

	

	// create command allocator
	{
		for (auto i = 0u; i < FrameCount; ++i) {
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_pCmdAllocator[i])
			);
			if (FAILED(hr)) {
				return false;
			}
		}
	}


	// create command list
	hr = m_pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pCmdAllocator[m_FrameIndex].Get(),
		nullptr,
		IID_PPV_ARGS(&m_pCmdList)
	);
	if (FAILED(hr)) {
		return false;
	}

	// setting for descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FrameCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	// create descriptor heap
	hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
	if (FAILED(hr)) {
		return false;
	}


	auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (auto i = 0u; i < FrameCount; ++i) {
		hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
		if (FAILED(hr)) return false;

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		// create render-target-view
		m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

		m_HandleRTV[i] = handle;
		handle.ptr += incrementSize;
	}


	// create fence
	{
		/* CPUÇ∆GPUÇÃìØä˙É^ÉCÉ~ÉìÉOÇèàóùÇ∑ÇÈ */
		for (auto i = 0u; i < FrameCount; ++i) {
			m_FenceCounter[i] = 0;
		}
		// create fence
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_pFence)
		);
		if (FAILED(hr)) return false;

		// create event
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr) return false;
	}
	// close command-list
	m_pCmdList->Close();
	return true;
}

/* ï`âÊèàóù */
void App::Render() {
	// start record command
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	// setting resource barrier
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// resource barrier
	m_pCmdList->ResourceBarrier(1, &barrier);

	// setting render-get
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	// setting clear-color
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// clear render-target-view
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	// ï`âÊèàóù
	{
		// ToDo : É|ÉäÉSÉìï`âÊópÇÃèàóùÇí«â¡
	}

	// setting resource-barrier
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// resource-barrier
	m_pCmdList->ResourceBarrier(1, &barrier);

	// close record command
	m_pCmdList->Close();

	// execute command
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// display
	Present(1);
}

/* display and prepare for next frame */
void App::Present(uint32_t interval) {
	// display on screen
	m_pSwapChain->Present(interval, 0);

	// signal
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(m_pFence.Get(), currentValue);

	// backbuffer-index
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// wait preparing for next frame
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex]){
		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	// increment fence-counter
	m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

/* wait complete GPU*/
void App::WaitGpu() {
	assert(m_pQueue != nullptr);
	assert(m_pFence != nullptr);
	assert(m_FenceEvent != nullptr);

	// signal
	m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);

	// set event when completed
	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

	// wait
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	// increment counter
	m_FenceCounter[m_FrameIndex]++;
}

/* terminate d3d */
void App::TermD3D() {
	// wait complete GPU
	WaitGpu();

	if (m_FenceEvent != nullptr) {
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	// term fence
	m_pFence.Reset();

	// term render-target-view
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i) {
		m_pColorBuffer[i].Reset();
	}

	// term command-list
	m_pCmdList.Reset();

	// term command-allocator
	for (auto i = 0u; i < FrameCount; ++i) m_pCmdAllocator[i].Reset();

	// term swap-chain
	m_pSwapChain.Reset();
	
	// term command-queue
	m_pQueue.Reset();

	// term device
	m_pDevice.Reset();
}
#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include<wrl/client.h>

//-----------------------
// Tipe definitions.
// ----------------------
template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;


// linker
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class App {
public : 
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	/* private variables */
	static const uint32_t FrameCount = 2; // double buffering

	HINSTANCE m_hInst;
	HWND m_hWnd;
	uint32_t m_Width;
	uint32_t m_Height;

	// variables for d3d
	ComPtr<ID3D12Device> m_pDevice;
	ComPtr<ID3D12CommandQueue> m_pQueue;
	ComPtr<IDXGISwapChain3> m_pSwapChain;
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;
	ComPtr<ID3D12Fence> m_pFence;

	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount];
	uint32_t m_FrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];

	/* private methods */
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	// methods for d3d
	bool InitD3D();
	void TermD3D();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
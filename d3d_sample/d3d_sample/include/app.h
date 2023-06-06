#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include<wrl/client.h>
#include<DirectXMath.h>
#include <d3dcompiler.h>
#
//-----------------------
// Tipe definitions.
// ----------------------
template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;


// linker
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

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
	
	struct alignas(256) Transform
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Proj;
	};

	template<typename T>
	struct ConstantBufferView
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
		D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
		T* pBuffer; // �o�b�t�@�擪�ւ̃|�C���^
	};
	// variables for d3d
	ComPtr<ID3D12Device> m_pDevice;
	ComPtr<ID3D12CommandQueue> m_pQueue;
	ComPtr<IDXGISwapChain3> m_pSwapChain;
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;
	ComPtr<ID3D12Fence> m_pFence;

	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV; // descriptor heap (constant_buffer_view, shader_resource_view, unordered_access_view)
	ComPtr<ID3D12Resource> m_pVB; // vertex buffer
	ComPtr<ID3D12Resource> m_pCB[FrameCount]; // constant buffer
	ComPtr<ID3D12RootSignature> m_pRootSignature; // root signature
	ComPtr<ID3D12PipelineState> m_pPSO; // pipeline state

	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount];
	uint32_t m_FrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];

	D3D12_VERTEX_BUFFER_VIEW m_VBV; // vertex buffer view
	D3D12_VIEWPORT m_Viewport; // viewport
	D3D12_RECT m_Scissor; // scissor rect
	
	ConstantBufferView<Transform> m_CBV[FrameCount]; // Constant buffer view
	float m_RotateAngle; // rotate angle

	/* private methods */
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();
	bool OnInit();
	void OnTerm();

	// methods for d3d
	bool InitD3D();
	void TermD3D();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
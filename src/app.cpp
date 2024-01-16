#include "app.h"
#include "assert.h"
#include<stdio.h>
#include<iostream>
namespace {
    const auto ClassName = TEXT("SampleWindowClass");
}

struct Vertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};



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

    // initialize d3d12
    if (!InitD3D()) {
        return false;
    }

    if (!OnInit()) {
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
bool App::OnInit() {
    // create vertex buffer
    {
        Vertex vertices[] = {
                {DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
                {DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
                {DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
                {DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
        };

        // heap property
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // setting resource
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(vertices);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // create resource
        auto hr = m_pDevice->CreateCommittedResource(
                &prop,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_pVB.GetAddressOf())
        );
        if (FAILED(hr)) { return false; }

        // mapping
        void* ptr = nullptr;
        hr = m_pVB->Map(0, nullptr, &ptr);
        if (FAILED(hr)) { return false; }

        // set vertex data
        memcpy(ptr, vertices, sizeof(vertices));

        // unmapping
        m_pVB->Unmap(0, nullptr);

        // setting VBV
        m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
        m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
        m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
    }

    // create index buffer
    {
        uint32_t indices[] = { 0,1,2,0,2,3 };

        // heap property
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // setting resource
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(indices);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // create resource
        auto hr = m_pDevice->CreateCommittedResource(
                &prop,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_pIB.GetAddressOf())
        );
        if (FAILED(hr)) {
            return false;
        }

        // mapping
        void* ptr = nullptr;
        hr = m_pIB->Map(0, nullptr, &ptr);
        if (FAILED(hr)) {
            return false;
        }

        memcpy(ptr, indices, sizeof(indices));

        m_pIB->Unmap(0, nullptr);

        // setting index buffer view
        m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
        m_IBV.Format = DXGI_FORMAT_R32_UINT;
        m_IBV.SizeInBytes = sizeof(indices);
    }

    // setting descriptor heap for cbuffer
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 2 * FrameCount;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;
        auto hr = m_pDevice->CreateDescriptorHeap(
                &desc,
                IID_PPV_ARGS(m_pHeapCBV.GetAddressOf())
        );
        if (FAILED(hr)) { return false; }


    }

    // create cbuffer
    {
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // setting resource
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(Transform);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (auto i = 0; i < FrameCount*2; ++i) {
            auto hr = m_pDevice->CreateCommittedResource(
                    &prop,
                    D3D12_HEAP_FLAG_NONE,
                    &desc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(m_pCB[i].GetAddressOf())
            );
            if (FAILED(hr)) {
                return false;
            }
            auto address = m_pCB[i]->GetGPUVirtualAddress();
            auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
            auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();

            handleCPU.ptr += incrementSize * i;
            handleGPU.ptr += incrementSize * i;

            // setting cbufferView
            m_CBV[i].HandleCPU = handleCPU;
            m_CBV[i].HandleGPU = handleGPU;
            m_CBV[i].Desc.BufferLocation = address;
            m_CBV[i].Desc.SizeInBytes = sizeof(Transform);

            // create cbufferView
            m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, handleCPU);

            // mapping
            hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));
            if (FAILED(hr)) {
                return false;
            }

            auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
            auto targetPos = DirectX::XMVectorZero();
            auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            auto fovY = DirectX::XMConvertToRadians(37.5f);
            auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

            // setting Matrix
            m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();
            m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
            m_CBV[i].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
        }

        // create root signature
        {

            auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
            flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
            flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

            // setting root params
            D3D12_ROOT_PARAMETER param = {};
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            param.Descriptor.ShaderRegister = 0;
            param.Descriptor.RegisterSpace = 0;
            param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

            // setting root signature
            D3D12_ROOT_SIGNATURE_DESC desc = {};
            desc.NumParameters = 1;
            desc.NumStaticSamplers = 0;
            desc.NumStaticSamplers = 0;
            desc.pParameters = &param;
            desc.pStaticSamplers = nullptr;
            desc.Flags = flag;

            ComPtr<ID3DBlob> pBlob;
            ComPtr<ID3DBlob> pErrorBlob;

            // serialize
            auto hr = D3D12SerializeRootSignature(
                    &desc,
                    D3D_ROOT_SIGNATURE_VERSION_1_0,
                    pBlob.GetAddressOf(),
                    pErrorBlob.GetAddressOf()
            );
            if (FAILED(hr)) {
                return false;
            }

            // root signature
            hr = m_pDevice->CreateRootSignature(
                    0,
                    pBlob->GetBufferPointer(),
                    pBlob->GetBufferSize(),
                    IID_PPV_ARGS(m_pRootSignature.GetAddressOf())
            );
            if (FAILED(hr)) {
                return false;
            }
        }

        // create pipeline state
        {
            // setting input layout
            D3D12_INPUT_ELEMENT_DESC elements[2];
            elements[0].SemanticName = "POSITION";
            elements[0].SemanticIndex = 0;
            elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
            elements[0].InputSlot = 0;
            elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elements[0].InstanceDataStepRate = 0;

            elements[1].SemanticName = "COLOR";
            elements[1].SemanticIndex = 0;
            elements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            elements[1].InputSlot = 0;
            elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elements[1].InstanceDataStepRate = 0;

            // setting rasterizer state
            D3D12_RASTERIZER_DESC descRS;
            descRS.FillMode = D3D12_FILL_MODE_SOLID;
            descRS.CullMode = D3D12_CULL_MODE_NONE;
            descRS.FrontCounterClockwise = FALSE;
            descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            descRS.DepthClipEnable = FALSE;
            descRS.MultisampleEnable = FALSE;
            descRS.MultisampleEnable = FALSE;
            descRS.AntialiasedLineEnable = FALSE;
            descRS.ForcedSampleCount = 0;
            descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

            // setting render target blend
            D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
                    FALSE, FALSE,
                    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                    D3D12_LOGIC_OP_NOOP,
                    D3D12_COLOR_WRITE_ENABLE_ALL
            };

            // setting blend state
            D3D12_BLEND_DESC descBS;
            descBS.AlphaToCoverageEnable = FALSE;
            descBS.AlphaToCoverageEnable = FALSE;
            descBS.IndependentBlendEnable = FALSE;
            for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
                descBS.RenderTarget[i] = descRTBS;
            }

            // setting depth_stencil_state
            D3D12_DEPTH_STENCIL_DESC descDSS = {};
            descDSS.DepthEnable = TRUE;
            descDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            descDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            descDSS.StencilEnable = FALSE;

            ComPtr<ID3DBlob> pVSBlob;
            ComPtr<ID3DBlob> pPSBlob;

            // load vertex shader
            auto hr = D3DReadFileToBlob(L"SampleVS.cso", pVSBlob.GetAddressOf());
            if (FAILED(hr)) {
                return false;
            }

            // load pixel shader
            hr = D3DReadFileToBlob(L"SamplePS.cso", pPSBlob.GetAddressOf());
            if (FAILED(hr)) {
                return false;
            }

            // setting pipeline state

            //std::cout << "uouo\n\n\n\n" << std::endl;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
            desc.InputLayout = { elements, _countof(elements) };
            desc.pRootSignature = m_pRootSignature.Get();
            desc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
            desc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
            desc.RasterizerState = descRS;
            desc.BlendState = descBS;
            desc.DepthStencilState = descDSS;
            // desc.DepthStencilState.DepthEnable = FALSE;
            // desc.DepthStencilState.StencilEnable = FALSE;
            desc.SampleMask = UINT_MAX;
            desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            desc.NumRenderTargets = 1;
            desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;

            // create pipeline state
            hr = m_pDevice->CreateGraphicsPipelineState(
                    &desc,
                    IID_PPV_ARGS(m_pPSO.GetAddressOf())
            );
            if (FAILED(hr)) {
                return false;
            }
        }

        // setting viewport and scissor rect(�`��̈�)
        {
            m_Viewport.TopLeftX = 0;
            m_Viewport.TopLeftY = 0;
            m_Viewport.Width = static_cast<float>(m_Width);
            m_Viewport.Height = static_cast<float>(m_Height);
            m_Viewport.MinDepth = 0.0f;
            m_Viewport.MaxDepth = 1.0f;

            m_Scissor.left = 0;
            m_Scissor.right = m_Width;
            m_Scissor.top = 0;
            m_Scissor.bottom = m_Height;
        }
    }
    return true;
}

void App::OnTerm() {
    for (auto i = 0; i < FrameCount; ++i) {
        if (m_pCB[i].Get() != nullptr) {
            m_pCB[i]->Unmap(0, nullptr);
            memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
        }
        m_pVB.Reset();
    }
    m_pVB.Reset();
    m_pPSO.Reset();
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
        else {
            Render();
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
    // ��������Adaptor���g���ď����������,GPU�̏��ɃA�N�Z�X�ł��Ă���
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

    // create z-stencil buffer
    {
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resDesc.Alignment = 0;
        resDesc.Width = m_Width;  // �T�C�Y��View Descriptor�Ɠ���
        resDesc.Height = m_Height;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_D32_FLOAT; // D32��D��Depth��D
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth = 1.0;
        clearValue.DepthStencil.Stencil = 0;

        hr = m_pDevice->CreateCommittedResource(
                &prop,
                D3D12_HEAP_FLAG_NONE,
                &resDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(m_pDepthBuffer.GetAddressOf())
        );
        if (FAILED(hr)) {
            return false;
        }

        // setting for descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 1;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        hr = m_pDevice->CreateDescriptorHeap(
                &heapDesc,
                IID_PPV_ARGS(m_pHeapDSV.GetAddressOf())
        );
        if (FAILED(hr)) { return false; }

        auto handle = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;
        m_pDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), &viewDesc, handle);
        m_HandleDSV = handle;
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
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // create swap-chain
        IDXGISwapChain* pSwapChain = nullptr;
        /* CreateSwapChain�ɂ����VRAM��Ƀt���[���o�b�t�@�̗̈悪�m�ۂ���� */
        hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
        //hr = pFactory->CreateSwapChainForComposition(m_pQueue.Get(), &desc, &pSwapChain);

        if (FAILED(hr)) {
            SafeRelease(pFactory);
            return false;
        }
        // get IDXGISwapChain3
        hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        if (FAILED(hr)) {
            SafeRelease(pFactory);
            SafeRelease(pFactory); //?
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
        /* CPU��GPU�̓����^�C�~���O���������� */
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
/* initialize */



/* �`�揈�� */
void App::Render() {

    // �X�V
    {
        DrawPerFrame();
    }

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

    // setting render-target
    m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, &m_HandleDSV);

    // setting clear-color
    float clearColor[] = { 0.15f, 0.25f, 0.25f, 1.0f };

    // clear render-target-view
    m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

    // clear depth stencil view
    m_pCmdList->ClearDepthStencilView(m_HandleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // �`�揈��
    {
        m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
        m_pCmdList->SetDescriptorHeaps(1, m_pHeapCBV.GetAddressOf());
        m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex].Desc.BufferLocation);
        m_pCmdList->SetPipelineState(m_pPSO.Get());

        m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
        m_pCmdList->IASetIndexBuffer(&m_IBV);
        m_pCmdList->RSSetViewports(1, &m_Viewport);
        m_pCmdList->RSSetScissorRects(1, &m_Scissor);

        // draw front triangle
        m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex * 2 + 0].Desc.BufferLocation);
        m_pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
        // draw back triangle
        m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex * 2 + 1].Desc.BufferLocation);
        m_pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

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
    /* IDXGISwapChain::Present */
    Present(1);
}

/* display and prepare for next frame */
void App::Present(uint32_t interval) {
    // display on screen
    m_pSwapChain->Present(interval, 0);

    // backbuffer-index
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    // signal
    const auto currentValue = m_FenceCounter[m_FrameIndex];
    m_pQueue->Signal(m_pFence.Get(), currentValue);



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
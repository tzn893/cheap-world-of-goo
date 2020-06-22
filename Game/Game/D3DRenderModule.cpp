#include "D3DRenderModule.h"
#include "WindowsApplication.h"
#include "Common.h"
#include "d3d12x.h"
#include <DirectXColors.h>

using namespace Game;

namespace Game {
	extern IApplication* app;
	extern FileLoader* gFileLoader;
}

void GetHardwareAdapter(IDXGIFactory4* factory,IDXGIAdapter1** adapter) {
	int i = 1;
	while (factory->EnumAdapters1(i++,adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		(*adapter)->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		else return;
	}

	adapter = nullptr;
	return;
}

bool D3DRenderModule::createRenderTargetView() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc;
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NodeMask = 0;
	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDesc.NumDescriptors = mBackBufferNum;

	if (FAILED(mDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&mRtvHeap)) ) ) {
		return false;
	}

	rtvDescriptorIncreament = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i != mBackBufferNum; i++) {
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));

		mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1,rtvDescriptorIncreament);
	}

	D3D12_RESOURCE_DESC depthDesc;
	depthDesc.Format = mDepthBufferFormat;
	depthDesc.Alignment = 0;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthDesc.Height = height;
	depthDesc.Width = width;
	depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthDesc.MipLevels = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;

	//We assume that the default depth stencil compare function is less so the 
	//default value of the depth buffer is 1.0f
	D3D12_CLEAR_VALUE cValue = {};
	cValue.Format = mDepthBufferFormat;
	cValue.DepthStencil.Depth = 1.f;
	cValue.DepthStencil.Stencil = 0;

	HRESULT hr = mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,&depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&cValue,IID_PPV_ARGS(&mDepthStencilBuffer));

	if (FAILED(hr)) return false;

	D3D12_DESCRIPTOR_HEAP_DESC depthHeapDesc;
	depthHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	depthHeapDesc.NodeMask = 0;
	depthHeapDesc.NumDescriptors = 1;
	depthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	hr = mDevice->CreateDescriptorHeap(&depthHeapDesc, IID_PPV_ARGS(&dsvHeap));

	if (FAILED(hr)) return false;

	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(),
		nullptr,dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}

void D3DRenderModule::waitForFenceValue(uint64_t fence) {
	mCmdQueue->Signal(mFence.Get(), fence);
	if (mFence->GetCompletedValue() < fence) {
		mFence->SetEventOnCompletion(fence, waitEvent);
		WaitForSingleObject(waitEvent, INFINITE);
	}
}

bool D3DRenderModule::initialize() {
	#ifdef _DEBUG || DEBUG
	{
		ComPtr<ID3D12Debug> debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		debug->EnableDebugLayer();
		debug->Release();
	}
	#endif

	#define HR_FAIL(hr,log) if(FAILED(hr)){\
		Log(log"\n");\
		return false;\
	}

	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&mDxgiFactory));
	HR_FAIL(hr, "d3d12 error : fail to create dxgi factory");

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(mDxgiFactory.Get(),&adapter);

	if (adapter == nullptr) {
		Log("d3d12 error : no hardware adapter on this device\n");
		return false;
	}
	else {
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		Log("====================================\n"
			"d3d12 current device attributes :\n"
			"%ls ,\n"
			"%.2f GB video memory,\n"
			"%.2f GB shared system memory\n"
			"====================================\n",
			desc.Description,
			desc.DedicatedVideoMemory / 1073741824.,
			desc.SharedSystemMemory / 1073741824.);
	
	}

	hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	HR_FAIL(hr, "d3d12 error : fail to create device");

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = mDevice->CreateCommandQueue(&desc,IID_PPV_ARGS(&mCmdQueue));
	if (FAILED(hr)) {
		Log("d3d12 error : fail to create command queue\n");
		return false;
	}


//-------------------------//
	hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&mCmdAllc));
	HR_FAIL(hr, "d3d12 error : fail to create command allocator");

	hr = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCmdAllc.Get(),nullptr,IID_PPV_ARGS(&mCmdList));
	HR_FAIL(hr, "d3d12 error : fail to create command list");

	mCmdList->Close();
//-------------------------//


	WindowsApplication* wapp = dynamic_cast<WindowsApplication*>(app);

	IDXGISwapChain1* sc1;

	DXGI_SWAP_CHAIN_DESC1 scDesc;
	ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	scDesc.BufferCount = mBackBufferNum;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scDesc.Format = mBackBufferFormat;
	scDesc.Width = width = wapp->getSysConfig().width;
	scDesc.Height = height = wapp->getSysConfig().height;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Stereo = FALSE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	winMain = wapp->getMainWnd();

	hr = mDxgiFactory->CreateSwapChainForHwnd(
		mCmdQueue.Get(),
		winMain,
		&scDesc,
		nullptr,
		nullptr,
		&sc1
	);

	HR_FAIL(hr, "d3d12 error : fail to create swap chain");

	mSwapChain.Attach(reinterpret_cast<IDXGISwapChain3*>(sc1));
	mCurrentFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	if (!createRenderTargetView()) {
		Log("d3d12 error : fail to create render target views\n");
		return false;
	}

	fenceVal = 0;
	hr = mDevice->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence));
	HR_FAIL(hr,"d3d12 error : fail to create fence");

	waitEvent = CreateEventEx(nullptr, L"", NULL, EVENT_ALL_ACCESS);

	if (!initialize2D()) {
		Log("d3d12 error : fail to initialize 2D render module\n");
		return false;
	}

	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1.0f;

	sissorRect.bottom = height;
	sissorRect.top = 0;
	sissorRect.left = 0;
	sissorRect.right = width;

	return true;
}

void D3DRenderModule::tick() {
	//------------------------------------//
	if (mFence->GetCompletedValue() < fenceVal) 
		return;

	mCmdAllc->Reset();
	mCmdList->Reset(mCmdAllc.Get(), nullptr);

	mCmdList->RSSetScissorRects(1, &sissorRect);
	mCmdList->RSSetViewports(1, &viewPort);

	mCmdList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(
		mBackBuffers[mCurrentFrameIndex].Get(),D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(mCurrentFrameIndex,rtvDescriptorIncreament);

	mCmdList->OMSetRenderTargets(1, &rtvHandle,TRUE,
		&dsvHeap->GetCPUDescriptorHandleForHeapStart());

	mCmdList->ClearRenderTargetView(rtvHandle, DirectX::Colors::SteelBlue,
		0,nullptr);
	mCmdList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr);

	//drawing 2d objects
	draw2D();

	mCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		mBackBuffers[mCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	));

	mCmdList->Close();

	ID3D12CommandList* toExcute[] = {mCmdList.Get()};
	mCmdQueue->ExecuteCommandLists(_countof(toExcute),toExcute);

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mBackBufferNum;
	mCmdQueue->Signal(mFence.Get(), ++fenceVal);

	if (FAILED(mSwapChain->Present(0, 0))) {
		Log("fail to present swap chain\n");
		dynamic_cast<WindowsApplication*>(app)->Quit();
		return;
	}
	//------------------------------------//
}


void D3DRenderModule::finalize() {
	waitForFenceValue(fenceVal);

	mDxgiFactory = nullptr;

	mSwapChain = nullptr;
	mDevice = nullptr;

	for (int i = 0; i != mBackBufferNum; i++)
		mBackBuffers[i] = nullptr;

	mCmdQueue = nullptr;

	//--------------------//
	mCmdAllc = nullptr;
	mCmdList = nullptr;
	//--------------------//

	mFence = nullptr;

	Data2D.mUploadProj->Unmap(0,nullptr);
	if(Data2D.mUploadVertex)
		Data2D.mUploadVertex->Unmap(0,nullptr);

	CloseHandle(waitEvent);
}

void D3DRenderModule::point2D(Vector2 pos,float width, Vector4 Color,float depth) {
	width = width / 2.;
	
	// The upper triangle
	Vertex2D p = {Vector3(pos.x - width,pos.y + width,depth),Color};
	Data2D.vertexList.push_back(p);
	p.Position = Vector3(pos.x + width, pos.y + width, depth);
	Data2D.vertexList.push_back(p);
	p.Position = Vector3(pos.x - width, pos.y - width, depth);
	Data2D.vertexList.push_back(p);

	//The lower triangle
	p.Position = Vector3(pos.x + width, pos.y + width, depth);
	Data2D.vertexList.push_back(p);
	p.Position = Vector3(pos.x + width, pos.y - width, depth);
	Data2D.vertexList.push_back(p);
	p.Position = Vector3(pos.x - width, pos.y - width, depth);
	Data2D.vertexList.push_back(p);

}

void D3DRenderModule::line2D(Vector2 start,Vector2 end,float width, Vector4 Color,float depth) {
	width = width / 2.;

	Vector2 dir = normalize(start - end), vert = Vector2(- dir.y,dir.x);

	Vector2 p0 = start + vert * width, p1 = end + vert * width,
		p2 = start - vert * width, p3 = end - vert * width;

	Vertex2D p;
#define PUSH_BACK_VERTEX_(vec) p = {Vector3(vec.x,vec.y,depth),Color};Data2D.vertexList.push_back(p)

	//The upper triangle
	PUSH_BACK_VERTEX_(p2);
	PUSH_BACK_VERTEX_(p1);
	PUSH_BACK_VERTEX_(p0);

	//The lower triangle
	PUSH_BACK_VERTEX_(p1);
	PUSH_BACK_VERTEX_(p2);
	PUSH_BACK_VERTEX_(p3);
}

void D3DRenderModule::set2DViewPort(Vector2 center,float _height) {
	float _width = _height * (float)width / (float)height;
	
	Data2D.Proj = Mat4x4(
		2. /_width,0          ,0,- 2. * center.x /_width ,
		0         ,2. /_height,0,- 2. * center.y /_height,
		0         ,0          ,1,0                       ,
		0         ,0          ,0,1
	).T();

	memcpy(Data2D.projBufferWriter, &Data2D.Proj, sizeof(Data2D.Proj));
}

bool D3DRenderModule::initialize2D() {

	//The shader have not been compiled.Find the source code and compile them 
	Buffer shader2D;
	ComPtr<ID3DBlob> VS2D, PS2D;

	{
		if (!gFileLoader->FileReadAndClose("Shaders\\Geometry2D.hlsl", shader2D)) {
			Log("Unable to find file Shaders\\Geometry2D.hlsl");
			return false;
		}

		auto compile = [](Buffer& code, const char* entry,
			const char* target, ID3DBlob** byteCode) -> bool {
				ComPtr<ID3DBlob> errorCode;

				UINT compileFlag = 0;
#ifdef _DEBUG
				compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

				HRESULT hr = D3DCompile(code.data, code.size,
					nullptr, nullptr,
					D3D_COMPILE_STANDARD_FILE_INCLUDE,
					entry, target, compileFlag, 0,
					byteCode, &errorCode);

				if (FAILED(hr)) {
					Log("fail to compile entry %s target %s , reason : %s \n",
						entry, target, errorCode->GetBufferPointer());
					return false;
				}

				return true;
		};


		if (!compile(shader2D, "VS", "vs_5_0", &VS2D) || !compile(shader2D, "PS", "ps_5_0", &PS2D))
			return false;

		//gFileLoader->FileWriteAndClose("Shaders\\Geometry2D_VS.vs", VS2D);
		//gFileLoader->FileWriteAndClose("Shaders\\Geometry2D_PS.ps", PS2D);

		//VS2D = nullptr, PS2D = nullptr;
	}
	
	//the name of the 2d vertex shader is Game2D_VS and 
	//the name of the 2d pixel shadere is Game2D_PS while
	//the system defined shaders' name start with 'Game'
	mShaderByteCodes["Game2D_VS"] = VS2D;
	mShaderByteCodes["Game2D_PS"] = PS2D;

	D3D12_INPUT_ELEMENT_DESC desc;
	
	//The input layout corresponding to the shader Game2D
	InputLayout layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	RootSignature rootSig(1,0);
	rootSig[0].initAsConstantBuffer(0,0);
	if (!rootSig.EndEditingAndCreate(mDevice.Get())) {
		Log("fail to create rootsignature");
		return false;
	}
	mRootSignatures["Game2D"] = rootSig.GetRootSignature();

	mPsos["Game2D"] = std::make_unique<GraphicPSO>();
	GraphicPSO* pso2D = mPsos["Game2D"].get();

	pso2D->LazyBlendDepthRasterizeDefault();
	pso2D->SetDepthStencilViewFomat(mDepthBufferFormat);
	pso2D->SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	pso2D->SetInputElementDesc(layout);
	pso2D->SetNodeMask(0);

	pso2D->SetPixelShader(PS2D->GetBufferPointer(),PS2D->GetBufferSize());
	pso2D->SetVertexShader(VS2D->GetBufferPointer(),VS2D->GetBufferSize());

	pso2D->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pso2D->SetRenderTargetFormat(mBackBufferFormat);
	pso2D->SetRootSignature(&rootSig);
	pso2D->SetSampleDesc(1, 0);
	pso2D->SetSampleMask(UINT_MAX);

	if (!pso2D->Create(mDevice.Get())) {
		Log("fail to create Pipeline State Object for 2D rendering\n");
		return false;
	}


	//initialize the data part
	Data2D.Proj = Mat4x4::I();
	const uint32_t bufferSize = (sizeof(Mat4x4) + 255) & (~255);
	
	//The size of the constant buffer must be multiple of the 256
	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Data2D.mUploadProj)
	);

	Data2D.mUploadProj->Map(0,nullptr,&Data2D.projBufferWriter);
	memcpy(Data2D.projBufferWriter, &Data2D.Proj, sizeof(Mat4x4));

	Data2D.currVSize = 0;
	Data2D.mUploadVertex = nullptr;
	return true;
}

void D3DRenderModule::draw2D() {
	//update data
	//if the current resource buffer's size is smaller than the 
	//vertex data in main memory.Then create a resource block 
	//2 times the main memory requires

	if (!Data2D.vertexList.size()) return;

	size_t vSize = Data2D.vertexList.size();

	if (Data2D.currVSize < Data2D.vertexList.size()) {
		Data2D.mUploadVertex = nullptr;

		Data2D.currVSize = Data2D.vertexList.size() * 2;

		mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex2D) * Data2D.currVSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,IID_PPV_ARGS(&Data2D.mUploadVertex)
		);

		Data2D.mUploadVertex->Map(0, nullptr, &Data2D.vertexBufferWriter);
	}

	//Upload vertex data to the vertex buffer
	memcpy(Data2D.vertexBufferWriter,Data2D.vertexList.data(),
		sizeof(Vertex2D) * vSize);

	//draw 2d items
	mCmdList->SetPipelineState(mPsos["Game2D"]->GetPSO());
	mCmdList->SetGraphicsRootSignature(mRootSignatures["Game2D"].Get());

	mCmdList->SetGraphicsRootConstantBufferView(0,Data2D.mUploadProj->GetGPUVirtualAddress());

	D3D12_VERTEX_BUFFER_VIEW vbv = {};
	vbv.BufferLocation = Data2D.mUploadVertex->GetGPUVirtualAddress();
	vbv.SizeInBytes = vSize * sizeof(Vertex2D);
	vbv.StrideInBytes = sizeof(Vertex2D);

	mCmdList->IASetVertexBuffers(0 , 1 , &vbv);
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCmdList->DrawInstanced(vSize, 1, 0, 0);

	//after every frame clear up the vertex buffer
	Data2D.vertexList.clear();
}
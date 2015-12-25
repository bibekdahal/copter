#include "pch.h"
#include "CopterMain.h"
#include "Common\DirectXHelper.h"

using namespace Copter;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
CopterMain::CopterMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	m_renderer = ref new Renderer(m_deviceResources);
	m_game = ref new Game();
	m_game->Initialize(m_renderer);
	m_game->NewGame(0);
	
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
    
}

CopterMain::~CopterMain()
{
    m_game->CleanUp();
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

void CopterMain::CreateWindowSizeDependentResources() 
{
	m_renderer->CreateWindowSizeDependentResources();
}

void CopterMain::Update() 
{
	m_timer.Tick([&](double ttotal, double tdelta)
	{
		
		m_game->Update((float)ttotal, (float)tdelta);
		m_renderer->Update(ttotal, tdelta);
	});
}

bool CopterMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	m_renderer->Render();
	m_game->Render();
	return true;
}

// Notifies renderers that device resources need to be released.
void CopterMain::OnDeviceLost()
{
	m_renderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void CopterMain::OnDeviceRestored()
{
	m_renderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}


using namespace Windows::System;
void CopterMain::KeyDown(Windows::UI::Core::KeyEventArgs^ Args)
{
	if (Args->VirtualKey == VirtualKey::Space)
		m_game->PointerPressed(-1, -1);

    else if (Args->VirtualKey == VirtualKey::Enter || Args->VirtualKey == VirtualKey::Escape)
		m_game->Pause(1);

	/*else if (Args->VirtualKey == VirtualKey::N)
		m_game->NewGame();
	else if (Args->VirtualKey == VirtualKey::E)
		m_game->ChangeDifficulty(0);
	else if (Args->VirtualKey == VirtualKey::H)
		m_game->ChangeDifficulty(1);
	else if (Args->VirtualKey == VirtualKey::Number2)
		m_game->NewGame(0);
	else if (Args->VirtualKey == VirtualKey::Number3)
		m_game->NewGame(1);*/
	m_game->keyInput(Args);
}
void CopterMain::KeyUp(Windows::UI::Core::KeyEventArgs^ Args)
{
	if (Args->VirtualKey == VirtualKey::Space)
		m_game->PointerReleased();

	m_game->keyInputUp(Args);
}
void CopterMain::Pause(int i)
{
	m_game->Pause(i);
}
void CopterMain::PointerPressed(int mx, int my)
{
	m_game->PointerPressed(mx, my);
}
void CopterMain::PointerReleased()
{
	m_game->PointerReleased();
}
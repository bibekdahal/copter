#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Renderer.h"
#include "Game.h"

namespace Copter
{
	class CopterMain : public DX::IDeviceNotify
	{
	public:
		CopterMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~CopterMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		void KeyDown(Windows::UI::Core::KeyEventArgs^ Args);
		void KeyUp(Windows::UI::Core::KeyEventArgs^ Args);
		void Pause(int i);
        void PointerPressed(int mx, int my);
        void PointerMoved(int mx, int my) { m_game->PointerMoved(mx, my); }
		void PointerReleased();

        void SetMouseMovement(int mv) { m_game->SetMouseMovement(mv); }
	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Renderer^ m_renderer;
		Game^ m_game;
		DX::StepTimer m_timer;
	};
}
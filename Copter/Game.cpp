#include "pch.h"
#include "Game.h"
#include <stdlib.h>

using namespace DirectX;
using namespace Copter;

float block_speed[] = {500.0f, 750.0f};
float copter_speed[] = {300.0f, 410.0f};

#define MENU_MAX 4
int menu_items = MENU_MAX - 1;

Game::Game() : m_press(false), m_difficulty(1), m_3d(false)
{
}


void Game::Initialize(Renderer^ renderer)
{
    m_renderer = renderer;
    m_renderer->AddFont(L"Gabriola", 35.0f, 1.0f, 1.0f, 1.0f);
	Initialize2d();
    Initialize3d();

    m_audioManager = ref new AudioManager();
    Audio ^ audio = ref new Audio("Assets\\copter.wav");
    m_copterSound = m_audioManager->AddAudio(audio);
    Audio ^ audio2 = ref new Audio("Assets\\explosion.wav");
    m_explosionSound = m_audioManager->AddAudio(audio2);

    m_button = m_renderer->AddSprite(L"button.dds", 256, 64, 1, 2);
    m_menu = m_renderer->AddSprite(L"menu.dds", 256, 64, 1, 5);
    m_ins2d = m_renderer->AddSprite(L"instruction2d.dds", 800, 480);
    m_ins3d = m_renderer->AddSprite(L"instruction3d.dds", 800, 480);
    m_justNewed = false;
}


void Game::NewGame(int game)
{
    if (game == 0) m_3d = false;
    else if (game == 1) m_3d = true;
	if (m_3d) NewGame3d();
    else NewGame2d();
    m_selection = -1;
    m_displayingMenu = true;
    menu_items = MENU_MAX - 1;
}

void Game::Update(float timeTotal, float timeDelta)
{
    if (m_3d) {
        Update3d(timeTotal, timeDelta); 
        if (m_copter3d.dead) menu_items = MENU_MAX - 1;
    }
    else {
        Update2d(timeTotal, timeDelta); 
        if (m_copter.dead) menu_items = MENU_MAX - 1;
    }
}

void Game::DrawMenu()
{
    m_renderer->BeginRender2D();
    float x = m_renderer->GetRenderTargetSize().Width / 2 - 128;
    float y = m_renderer->GetRenderTargetSize().Height / 2 - ((64 + 5) * menu_items) / 2;
    for (int i = 0; i < menu_items; ++i)
    {
        m_renderer->DrawSprite(m_button, x, y, (m_selection == i) ? 1 : 0, 0.01f);
        if (i == menu_items - 1)
            m_renderer->DrawSprite(m_menu, x, y, (m_difficulty == 0) ? 4 : 3);
        else
            m_renderer->DrawSprite(m_menu, x, y, (menu_items == MENU_MAX) ? i : i + 1);
        y += 5 + 64;
    }
}
void Game::Render()
{
	if (m_3d) Render3d();
	else Render2d();

    if (m_displayingMenu)
        DrawMenu();
    else if (m_justNewed)
    {
        m_renderer->BeginRender2D();
        float x = m_renderer->GetRenderTargetSize().Width / 2 - 800/2;
        float y = m_renderer->GetRenderTargetSize().Height / 2 - 480 / 2;
        if (m_3d) m_renderer->DrawSprite(m_ins3d, x, y, 0);
        else m_renderer->DrawSprite(m_ins2d, x, y, 0);
    }
}

void Game::ChangeDifficulty(int difficulty) 
{
    if (difficulty == -1) m_difficulty = (m_difficulty == 0) ? 1 : 0;
	else m_difficulty = difficulty;
}



void Game::Pause(int pause)
{
	if (pause==0)
		m_pause = false;
    else if (pause == 1)
        m_pause = true;
    else
        m_pause = !m_pause;
    if (m_pause)
    {
        if (m_displayingMenu)
        {
            if (menu_items != MENU_MAX) NewGame(); 
            m_displayingMenu = false;
            return;
        }
        m_displayingMenu = true;
        menu_items = MENU_MAX;
    }
    else
        m_displayingMenu = false;

}

void Game::UpdateSize()
{
}

void Game::PointerMoved(int mx, int my)
{
    if (m_displayingMenu)
    {
        m_selection = -1;
        float x = m_renderer->GetRenderTargetSize().Width / 2 - 128;
        float y = m_renderer->GetRenderTargetSize().Height / 2 - ((64 + 5) * menu_items) / 2;
        if (mx >= x && mx <= x + 256 && my >= y && my <= y + (64 + 5) * menu_items)
        {
            for (int i = 0; i < menu_items; ++i)
            {
                if (my >= y && my <= y + 64) m_selection = i;
                y += 64 + 5;
            }
        }
    }
}

void Game::PointerPressed(int mx, int my)
{
    PointerMoved(mx, my);
    m_press = true;
    m_justNewed = false;
    if (!m_displayingMenu)
        m_pause = false;
}
void Game::PointerReleased()
{
	m_press = false;
    if (m_displayingMenu)
    {
        int selection = (menu_items == MENU_MAX - 1 && m_selection != -1) ? m_selection + 1 : m_selection;
        switch (selection)
        {
        case 0:
            m_displayingMenu = false;
            m_pause = true;
            break;
        case 1:
            NewGame(0);
            m_displayingMenu = false;
            m_pause = true;
            m_justNewed = true;
            break;
        case 2:
            NewGame(1);
            m_displayingMenu = false;
            m_pause = true;
            m_justNewed = true;
            break;
        case 3:
            ChangeDifficulty();
            NewGame();
            m_pause = true;
            break;
        }
    }
}


void Game::Uninitialize()
{
}

void Game::keyInput(Windows::UI::Core::KeyEventArgs^ Args)
{
	if ((int)Args->VirtualKey<256)
		keys[(int)Args->VirtualKey]=true;
	
}
void Game::keyInputUp(Windows::UI::Core::KeyEventArgs^ Args)
{
	if ((int)Args->VirtualKey<256)
		keys[(int)Args->VirtualKey]=false;
}

Game::~Game()
{
}








/*
		{  0.5f,  0.5f, -0.5f,   0.333333f,  0.666667f, -0.666667f,  0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f,   0.408248f, -0.408248f,  0.816497f,  1.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f,   0.666667f, -0.666667f, -0.333333f,  1.0f, 1.0f },
		{  0.5f,  0.5f,  0.5f,   0.816497f,  0.408248f,  0.408248f,  0.0f, 1.0f },
		
		{ -0.5f,  0.5f,  0.5f,  -0.333333f,  0.666667f,  0.666667f,  0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f,  -0.408248f, -0.408248f, -0.816497f,  1.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f,  -0.666667f, -0.666667f,  0.333333f,  1.0f, 1.0f },
		{ -0.5f,  0.5f, -0.5f,  -0.333333f,  0.666667f,  0.666667f,  0.0f, 1.0f },
		
		{  0.5f,  0.5f,  0.5f,   0.816497f,  0.408248f,  0.408248f,  0.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f,  -0.816497f,  0.408248f, -0.408248f,  1.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f,  -0.333333f,  0.666667f,  0.666667f,  1.0f, 1.0f },
		{  0.5f,  0.5f, -0.5f,   0.333333f,  0.666667f, -0.666667f,  0.0f, 1.0f },
		
		{  0.5f, -0.5f, -0.5f,   0.666667f, -0.666667f, -0.333333f,  0.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f,  -0.666667f, -0.666667f,  0.333333f,  1.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f,  -0.408248f, -0.408248f, -0.816497f,  1.0f, 1.0f },
		{  0.5f, -0.5f,  0.5f,   0.408248f, -0.408248f,  0.816497f,  0.0f, 1.0f },
		
		{  0.5f, -0.5f,  0.5f,   0.408248f, -0.408248f,  0.816497f,  0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f,  -0.333333f,  0.666667f,  0.666667f,  1.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f,  -0.666667f, -0.666667f,  0.333333f,  1.0f, 1.0f },
		{  0.5f,  0.5f,  0.5f,   0.816497f,  0.408248f,  0.408248f,  0.0f, 1.0f },
		
		{ -0.5f, -0.5f, -0.5f,  -0.408248f, -0.408248f, -0.816497f,  0.0f, 0.0f },
		{  0.5f,  0.5f, -0.5f,   0.333333f,  0.666667f, -0.666667f,  1.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f,   0.666667f, -0.666667f, -0.333333f,  1.0f, 1.0f },
		{ -0.5f,  0.5f, -0.5f,  -0.816497f,  0.408248f, -0.408248f,  0.0f, 1.0f },

*/
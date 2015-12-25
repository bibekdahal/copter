#include "pch.h"
#include "Game.h"
#include <stdlib.h>

using namespace DirectX;
using namespace Copter;

using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Platform;

#define MAX_BLOCK_WIDTH 48*4

extern float block_speed[2];
extern float copter_speed[2];

void Game::Initialize2d()
{
	m_spr_bar = m_renderer->AddSprite(L"bar.dds", 48.0f, 12.0f);
	m_copter.spr.sprid = m_renderer->AddSprite(L"copter.dds", COPTER_WIDTH, COPTER_HEIGHT, 4, 1);
	m_copter.spr_expl.sprid = m_renderer->AddSprite(L"explosion.dds", 600.0f, 491.0f, 4, 5);
	
	block bk;
	bk.spr = m_renderer->AddSprite(L"block1.dds", 48.0f, 60.0f);
	bk.width = 48.0f, bk.height = 60.0f;
	m_block_obj.push_back(bk);

	bk.spr = m_renderer->AddSprite(L"block2.dds", 96.0f, 60.0f);
	bk.width = 96.0f;
	m_block_obj.push_back(bk);

	bk.spr = m_renderer->AddSprite(L"block3.dds", 96.0f, 120.0f);
	bk.height = 120.0f;
	m_block_obj.push_back(bk);

	bk.spr = m_renderer->AddSprite(L"block4.dds", 192.0f, 180.0f);
	bk.height = 180.0f;
	bk.width = 192.0f;
	m_block_obj.push_back(bk);
}


void Game::NewGame2d()
{
	m_3d = false;
	m_pause = true;
	m_gone = m_score = 0;
	m_maxby = 5;
	
	m_copter.spr.imgid = 0;
	m_copter.spr.anim_tm = 0.0f;
	m_copter.spr_expl.imgid = 0;
	m_copter.spr_expl.anim_tm = 0.0f;

	Windows::Foundation::Size sz = m_renderer->GetRenderTargetSize();
	m_copter.pos.x = sz.Width/2.0f - COPTER_WIDTH/2.0f;
	m_copter.pos.y = sz.Height/2.0f - COPTER_HEIGHT/2.0f;

	m_copter.dead = false;
	m_copter.expl_dead = false; 
	m_wwidth = sz.Width;
	m_wheight = sz.Height;

	m_topbottom.clear(); m_etop.clear(); m_ebottom.clear(); m_blocks.clear();
	for (int i=0;i<(int)(m_wwidth/48 + 4);i++)
	{
		DirectX::XMFLOAT2 ps;
		ps.x = i*48.0f;
		ps.y = 0.0f;
		m_topbottom.push_back(ps);
		ps.y = 12.0f;
		m_topbottom.push_back(ps);
		ps.y = 24.0f;
		m_topbottom.push_back(ps);
		ps.y = m_wheight-12.0f;
		m_topbottom.push_back(ps);
		ps.y = m_wheight-24.0f;
		m_topbottom.push_back(ps);
		ps.y = m_wheight-36.0f;
		m_topbottom.push_back(ps);
	}

	m_lastx = (int)(m_wwidth/48 + 4) * 48.0f;
	AddBlocks(m_lastx);

	m_lastobsx = (int)(m_wwidth/48 + 10) * 48.0f;
	AddObstacle(m_lastobsx);


    m_audioManager->Play(m_copterSound, true);
}


void Game::AddBlocks(float x)
{
	DirectX::XMFLOAT2 ps;
	ps.x = x;

	for (int i=0;i<(rand()%(int)(m_maxby+1));i++)
	{
		ps.y = (i+3)*12.0f;
		m_etop.push_back(ps);
	}

	for (int i=0;i<m_maxby;i++)
	{
		ps.y = m_wheight - (i+3)*12.0f - 12.0f;
		m_ebottom.push_back(ps);
	}
}

void Game::AddObstacle(float x)
{
	block_ins bk;
	bk.x = x;
	
	if (m_maxby<15)
		bk.bk = rand() % m_block_obj.size();
	else if (m_maxby < 10)
		bk.bk = rand() % (m_block_obj.size()-1);
	else
		bk.bk = rand() % (m_block_obj.size()-2);
 	if (bk.bk == 0)
	{
		if (m_maxby>=20)
			bk.y = (10 + rand() % ((int)(m_wheight/12) - m_maxby-10 - 5))*12.0f;
		else
		{
			bk.y = 12.0f*20;
			m_blocks.push_back(bk);

			bk.y = (int)(m_wheight/12 - 21) * 12.0f;
			m_blocks.push_back(bk);

			bk.x += 48.0f * 5;
			bk.y = (12.0f*20 + bk.y) / 2;
		}
	}
	else if (bk.bk == 1)
	{
		if (m_maxby<10 && (rand()%2)==0)
		{
			bk.y = 12.0f*15;
			m_blocks.push_back(bk);

			bk.y = (int)(m_wheight/12 - 16) * 12.0f;
		}
		else
			bk.y = (15 + rand() % ((int)(m_wheight/12) - m_maxby - 15 - 5))*12.0f;
	}
	else if (bk.bk == 3)
	{
		bk.y = (m_maxby + rand() % ((int)(m_wheight/12) - m_maxby*2 - 15))*12.0f;
		if ((rand()%2)==0)
		{
			bk.bk = 2;
		}
	}
	else
	{
		bk.y = (m_maxby + rand() % ((int)(m_wheight/12) - m_maxby*2 - 10))*12.0f;
	}

	m_blocks.push_back(bk);
}

void Game::Update2d(float timeTotal, float timeDelta)
{

	if (m_copter.dead)
	{
        if (m_copter.expl_dead) {
            m_displayingMenu = true; return;
        }

		m_copter.spr_expl.anim_tm += timeDelta*10.0f;
		m_copter.spr_expl.imgid = (unsigned)floor(m_copter.spr_expl.anim_tm);
		if (m_copter.spr_expl.anim_tm>= 4*5) 
			m_copter.expl_dead = true;
		if (m_copter.spr_expl.imgid>4*5-1) m_copter.spr_expl.imgid = 4*5-1;
		return;
	}

	m_copter.spr.anim_tm += timeDelta*10.0f;
	m_copter.spr.imgid = (unsigned)floor(m_copter.spr.anim_tm);
	if (m_copter.spr.anim_tm>= 4*5) m_copter.spr.anim_tm=0.0f;
	
	if (m_pause) return;

	
#define BLOCK_SPEED block_speed[m_difficulty]*timeDelta
	
	for (unsigned i=0;i<m_topbottom.size();i++)
	{
		m_topbottom[i].x -= BLOCK_SPEED;
		if (m_topbottom[i].x < -48.0f)
		{
			//if (m_top[i].y < 3*12.0f) 
			m_topbottom[i].x += ((unsigned)(m_wwidth/48) + 4)*48.0f;
		}
		else if (m_topbottom[i].x > m_copter.pos.x-48.0f && m_topbottom[i].x < m_copter.pos.x + COPTER_WIDTH + 48.0f)
		{
            if (CheckCollisionCopter(m_topbottom[i].x, m_topbottom[i].x + 48.0f, m_topbottom[i].y, m_topbottom[i].y + 12.0f,
                m_copter.pos.x, m_copter.pos.y))
                //m_copter.pos.x, m_copter.pos.x+COPTER_WIDTH, m_copter.pos.y+16.0f, m_copter.pos.y + COPTER_HEIGHT-16.0f))
            {
                m_copter.dead = true;
                m_audioManager->Stop(m_copterSound);
                m_audioManager->Play(m_explosionSound);
            }
		}
	}

	if (!m_etop.empty())
	{
		for (auto i=m_etop.begin(); i!=m_etop.end();++i)
		{
			i->x -= BLOCK_SPEED;
			if (i->x > m_copter.pos.x-48.0f && i->x < m_copter.pos.x + COPTER_WIDTH + 48.0f)
			{
				if (CheckCollisionCopter(i->x, i->x+48.0f, i->y, i->y + 12.0f,
					m_copter.pos.x, m_copter.pos.y))
				//m_copter.pos.x, m_copter.pos.x+COPTER_WIDTH, m_copter.pos.y+16.0f, m_copter.pos.y + COPTER_HEIGHT-16.0f))
                {
                    m_copter.dead = true;
                    m_audioManager->Stop(m_copterSound);
                    m_audioManager->Play(m_explosionSound);
                }
			}
		}

		while (!m_etop.empty() && m_etop.begin()->x < -48.0f)
			m_etop.pop_front();
	}

	if (!m_ebottom.empty())
	{
		for (auto i=m_ebottom.begin(); i!=m_ebottom.end();++i)
		{
			i->x -= BLOCK_SPEED;
			if (i->x > m_copter.pos.x-48.0f && i->x < m_copter.pos.x + COPTER_WIDTH + 48.0f)
			{
				if (CheckCollisionCopter(i->x, i->x+48.0f, i->y, i->y + 12.0f,
					m_copter.pos.x, m_copter.pos.y))
				//m_copter.pos.x, m_copter.pos.x+COPTER_WIDTH, m_copter.pos.y+16.0f, m_copter.pos.y + COPTER_HEIGHT-16.0f))
                {
                    m_copter.dead = true;
                    m_audioManager->Stop(m_copterSound);
                    m_audioManager->Play(m_explosionSound);
                }
			}
		}

		while (!m_ebottom.empty() && m_ebottom.begin()->x < -48.0f)
			m_ebottom.pop_front();
	}

	
	m_lastx -= BLOCK_SPEED;
	
	float endx = (unsigned)(m_wwidth/48 + 4) * 48.0f;
	if (m_lastx <= endx - 48.0f)
	{
		m_lastx += 48.0f;
		AddBlocks(m_lastx);
	}


	if (!m_blocks.empty())
	{
		for (auto i=m_blocks.begin(); i!=m_blocks.end();++i)
		{
			i->x -= BLOCK_SPEED;
			if (CheckCollisionCopter(i->x, i->x+m_block_obj[i->bk].width , i->y, i->y + m_block_obj[i->bk].height,
				m_copter.pos.x, m_copter.pos.y))
				//m_copter.pos.x, m_copter.pos.x+COPTER_WIDTH, m_copter.pos.y+16.0f, m_copter.pos.y + COPTER_HEIGHT-16.0f))
            {
                m_copter.dead = true;
                m_audioManager->Stop(m_copterSound);
                m_audioManager->Play(m_explosionSound);
            }
		}

		while (!m_blocks.empty() && m_blocks.begin()->x < - MAX_BLOCK_WIDTH)
			m_blocks.pop_front();
	}

	
	m_lastobsx -= BLOCK_SPEED;
	endx = (unsigned)(m_wwidth/48 + 10) * 48.0f;
	if (m_lastobsx <= endx - 48.0f*9)
	{
		m_lastobsx += 48.0f*9;
		AddObstacle(m_lastobsx);
	}


		//m_copter.pos.y -= copter_speed[m_difficulty]*timeDelta;
	//else
		//m_copter.pos.y += copter_speed[m_difficulty]*timeDelta;

    m_copter.vely += timeDelta * 9.8f * 10;
    if (m_press)
        m_copter.vely = -copter_speed[m_difficulty] / 13;
    m_copter.pos.y += timeDelta * m_copter.vely * 10;

	if (timeDelta < 0.1)
	{
		m_gone+=timeDelta;
		m_score+=timeDelta*10;

        {
            ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
            auto value = localSettings->Values->Lookup(m_difficulty == 0 ? "BestScore2DEasy" : "BestScore2DHard");
            int bscore = 0;
            if (value) bscore = safe_cast<int>(value);
            if (bscore < (int)m_score)
                localSettings->Values->Insert(m_difficulty == 0 ? "BestScore2DEasy" : "BestScore2DHard", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)m_score)));
        }


	}
	if (m_gone>5)
	{
		m_maxby++;
		m_gone=0;
	}
}

void Game::Render2d()
{
	
	m_renderer->BeginRender2D();
	for (unsigned i=0;i<m_topbottom.size();i++)
		m_renderer->DrawSprite(m_spr_bar, m_topbottom[i].x, m_topbottom[i].y, 0, 5.0f);
	
	if (!m_etop.empty())
	for (auto i=m_etop.begin(); i!=m_etop.end();++i)
		m_renderer->DrawSprite(m_spr_bar, i->x, i->y, 0, 5.0f);

	if (!m_ebottom.empty())
	for (auto i=m_ebottom.begin(); i!=m_ebottom.end();++i)
		m_renderer->DrawSprite(m_spr_bar, i->x, i->y, 0, 2.0f);

	if (!m_blocks.empty())
	for (auto i=m_blocks.begin(); i!=m_blocks.end();++i)
		m_renderer->DrawSprite(m_block_obj[i->bk].spr, i->x, i->y, 0, 3.0f);

	if (m_copter.dead)
	{
		if (!m_copter.expl_dead)
		m_renderer->DrawSprite(m_copter.spr_expl.sprid, m_copter.pos.x, m_copter.pos.y, m_copter.spr_expl.imgid, 1.0f, 
								-600.0f/2+COPTER_WIDTH/2.0f, -491.0f/2+COPTER_HEIGHT/2.0f);
	}
	else
		m_renderer->DrawSprite(m_copter.spr.sprid, m_copter.pos.x, m_copter.pos.y, m_copter.spr.imgid, 1.0f);

    m_renderer->DrawText(0, L"Distance: " + ((int)m_score).ToString(), 20, 20, 400, 50);
    ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
    auto values = localSettings->Values;
    auto value = localSettings->Values->Lookup(m_difficulty == 0 ? "BestScore2DEasy" : "BestScore2DHard");
    int bscore = 0;
    if (value) bscore = safe_cast<int>(value);
    m_renderer->DrawText(0, L"Best Distance: " + bscore.ToString(), 20, 60, 400, 50);
    m_renderer->DrawText(0, L"Difficulty: " + (m_difficulty == 0 ? "Easy" : "Hard"), 20, 100, 400, 50);
	
}

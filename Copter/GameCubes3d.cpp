
#include "pch.h"
#include "Game.h"
#include <stdlib.h>


#ifdef _DEBUG
#pragma comment(lib, "bullet-2.82-r2704/lib/BulletDynamics_Debug")
#pragma comment(lib, "bullet-2.82-r2704/lib/BulletCollision_Debug")
#pragma comment(lib, "bullet-2.82-r2704/lib/LinearMath_Debug")
#else
#pragma comment(lib, "bullet-2.82-r2704/lib/BulletDynamics")
#pragma comment(lib, "bullet-2.82-r2704/lib/BulletCollision")
#pragma comment(lib, "bullet-2.82-r2704/lib/LinearMath")
#endif

using namespace DirectX;
using namespace Copter;

using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Platform;

#define MAX_BLOCK3D_HEIGHT 2.0f

extern float block_speed[2];
extern float copter_speed[2];

extern Vertex cubeVertices[24];
extern unsigned int cubeIndices[36];

//unsigned blck;
void Game::Initialize3d()
{
    m_broadphase = new btDbvtBroadphase();
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
    m_dynamicsWorld->setGravity(btVector3(0, -10, 0));

	m_copter3d.model = m_renderer->AddModel("Assets\\copter.dat", &m_copter3d.shape, 0.25f);
	m_copter3d.tailMesh = m_renderer->AddMesh(m_copter3d.model, "Assets\\tail.dat");
	m_copter3d.topMesh = m_renderer->AddMesh(m_copter3d.model, "Assets\\top.dat", 2);
	m_copter3d.rotAngle = 0.0f;
    m_copter3d.rigidBody = NULL;


    m_groundModel = m_renderer->AddModel();
    m_renderer->AddMesh(m_groundModel, cubeVertices, 24, cubeIndices, 36, 0);
    m_renderer->SetMaterial(m_groundModel, 0.07f, 0.33f, 0.0f, 1.0f);
    m_renderer->ScaleModelAbs(m_groundModel, 1000, 0.1f, 1000);
    m_renderer->TranslateModel(m_groundModel, 0, -12.0f, 0);

    m_gcshape = new btBoxShape(btVector3(1000, 0.1f, 1000));
    {
        btDefaultMotionState* mstate = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -12, 0)));
        btRigidBody::btRigidBodyConstructionInfo rbci(0.0f, mstate, m_gcshape, btVector3(0, 0, 0));
        m_ground = new btRigidBody(rbci);
        m_dynamicsWorld->addRigidBody(m_ground);
    }{
        /*btDefaultMotionState* mstate = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 12, 0)));
        btRigidBody::btRigidBodyConstructionInfo rbci(0.0f, mstate, m_gcshape, btVector3(0, 0, 0));
        m_ceiling = new btRigidBody(rbci);
        m_dynamicsWorld->addRigidBody(m_ceiling);*/
        m_ceiling = NULL;
    }


	m_blockModel = m_renderer->AddModel();
	m_renderer->AddTexture(m_blockModel, "cube_texture.dds");

	m_renderer->AddMesh(m_blockModel, cubeVertices, 24, cubeIndices, 36, 0);
	block3d bck;
    bck.length = bck.width = bck.height = 0.8f; bck.shape = new btBoxShape(btVector3(0.8f / 2, 0.8f / 2, 0.8f / 2));
	m_block3d_obj.push_back(bck);
	
    bck.length = bck.width = bck.height = 1.8f; bck.shape = new btBoxShape(btVector3(1.8f / 2, 1.8f / 2, 1.8f / 2));
	m_block3d_obj.push_back(bck);
}

void Game::ClearBlocks3d()
{
    if (!m_blocks3d.empty())
    for (auto i = m_blocks3d.begin(); i != m_blocks3d.end(); ++i)
    if (i->rigidBody)
    {
        m_dynamicsWorld->removeRigidBody(i->rigidBody);
        delete i->rigidBody->getMotionState();
        delete i->rigidBody;
        i->rigidBody = NULL;
    }
    m_blocks3d.clear();
}

float initialTheta = XMConvertToRadians(-10);
float cx = -2.98f * sinf(initialTheta), cz = -2.98f * cosf(initialTheta), cy = 1.20f;
float ctheta;
void Game::NewGame3d()
{
    m_mouseMovement = 0;
	ctheta = initialTheta;
	m_3d = true;
	m_pause = true;
	m_gone = m_score = 0;

	m_copter3d.dead = false;
	m_copter3d.expl_dead = false;
	m_copter.spr_expl.imgid = 0;
	m_copter.spr_expl.anim_tm = 0.0f;

	m_copter3d.pos = XMFLOAT3(0.0f,0.0f,0.0f);

    if (m_copter3d.rigidBody)
    {
        m_dynamicsWorld->removeRigidBody(m_copter3d.rigidBody);
        delete m_copter3d.rigidBody->getMotionState();
        delete m_copter3d.rigidBody; m_copter3d.rigidBody = NULL;
    }

    btQuaternion quat;
    quat.setRotation(btVector3(1, 0, 0), btRadians(90));
    btDefaultMotionState* mstate = new btDefaultMotionState(btTransform(quat, btVector3(m_copter3d.pos.x, m_copter3d.pos.y, m_copter3d.pos.z)));
    btVector3 inertia(0, 0, 0);
    float mass = 4.0f;
    m_copter3d.shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo rbci(mass, mstate, m_copter3d.shape, inertia);
    m_copter3d.rigidBody = new btRigidBody(rbci);
    m_dynamicsWorld->addRigidBody(m_copter3d.rigidBody);

	m_renderer->ScaleModelAbs(m_copter3d.model, 0.25f);
	m_renderer->RotateModelX(m_copter3d.model, XMConvertToRadians(90));
	m_renderer->TranslateModel(m_copter3d.model, m_copter3d.pos.x, m_copter3d.pos.y, m_copter3d.pos.z);
	m_renderer->SetCamEye(cx+m_copter3d.pos.x,cy+m_copter3d.pos.y,cz+m_copter3d.pos.z);
	m_renderer->SetCamAt(m_copter3d.pos.x,m_copter3d.pos.y+0.5f,m_copter3d.pos.z);

	m_lastobsz = 50.0f;
    ClearBlocks3d();

    m_audioManager->Play(m_copterSound, true);
}

void Game::AddObstacle3d(float z)
{
	block3d_ins bk;
	bk.z = z;
	
	float x ;
	float y = -10.0f;

	while (y < 10.0f)
	{
		x = -10.0f;
		while (x < 10.0f)
		{
			int i;
			if (y >= 8.0f || y == -10.0f)
				i = (rand()%5);
			else
				i = (rand()%30);
			float dx = 1.0f;
			if (i==3)
			{
				bk.bk = rand()%m_block3d_obj.size();
				bk.x = x;
				bk.y = y;

                btDefaultMotionState* mstate = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(bk.x, bk.y, bk.z)));
                btVector3 inertia(0, 0, 0);
                float mass = 1000.0f;
                m_block3d_obj[bk.bk].shape->calculateLocalInertia(mass, inertia);
                btRigidBody::btRigidBodyConstructionInfo rbci(mass, mstate, m_block3d_obj[bk.bk].shape, inertia);
                bk.rigidBody = new btRigidBody(rbci);
                bk.rigidBody->setLinearFactor(btVector3(0, 0, 1));
                bk.rigidBody->setLinearVelocity(btVector3(0, 0, -block_speed[m_difficulty] / 50.0f));
                m_dynamicsWorld->addRigidBody(bk.rigidBody);

				m_blocks3d.push_back(bk);
				dx = m_block3d_obj[bk.bk].length;
			}
			x += dx + 0.1f;
		}
		y += MAX_BLOCK3D_HEIGHT;
	}
}

void Game::CleanUp()
{
    if (m_gcshape) { delete m_gcshape; m_gcshape = NULL; }
    if (m_ground)
    {
        m_dynamicsWorld->removeRigidBody(m_ground);
        delete m_ground->getMotionState();
        delete m_ground; m_ground = NULL;
    }if (m_ceiling)
    {
        m_dynamicsWorld->removeRigidBody(m_ceiling);
        delete m_ceiling->getMotionState();
        delete m_ceiling; m_ceiling = NULL;
    }


    if (m_copter3d.rigidBody) 
    {
        m_dynamicsWorld->removeRigidBody(m_copter3d.rigidBody);
        delete m_copter3d.rigidBody->getMotionState();
        delete m_copter3d.rigidBody; m_copter3d.rigidBody = NULL;
    }
    if (m_copter3d.shape) { delete m_copter3d.shape; m_copter3d.shape = NULL; }

    ClearBlocks3d();

    for (unsigned i = 0; i < m_block3d_obj.size(); ++i)
    if (m_block3d_obj[i].shape)
    {
        delete m_block3d_obj[i].shape;
        m_block3d_obj[i].shape = NULL;
    }

    delete m_dynamicsWorld;
    delete m_solver;
    delete m_collisionConfiguration;
    delete m_dispatcher;
    delete m_broadphase;
}

void Game::Update3d(float timeTotal, float timeDelta)
{	

	using namespace Windows::System;

	if (m_copter3d.dead) 
	{
		/*float cxx = cz*sinf(ctheta);
		float czz = cz*cosf(ctheta);
		m_renderer->SetCamEye(cxx + m_copter3d.pos.x, cy + m_copter3d.pos.y, czz + m_copter3d.pos.z);
		m_renderer->SetCamAt(m_copter3d.pos.x, m_copter3d.pos.y + 0.5f, m_copter3d.pos.z);
		ctheta += timeDelta;*/


        if (!m_copter3d.expl_dead) //return;
        {
            m_copter.spr_expl.anim_tm += timeDelta*10.0f;
            m_copter.spr_expl.imgid = (unsigned)floor(m_copter.spr_expl.anim_tm);
            if (m_copter.spr_expl.anim_tm >= 4 * 5)
                m_copter3d.expl_dead = true;
            if (m_copter.spr_expl.imgid > 4 * 5 - 1) m_copter.spr_expl.imgid = 4 * 5 - 1;
        }
		//return;
	}

   
	if (!m_copter3d.dead) m_copter3d.rotAngle += timeDelta*15;
	m_renderer->RotateMeshXAbs(m_copter3d.model, m_copter3d.tailMesh, m_copter3d.rotAngle);
	m_renderer->RotateMeshZAbs(m_copter3d.model, m_copter3d.topMesh, m_copter3d.rotAngle);
	m_renderer->TranslateMesh(m_copter3d.model,  m_copter3d.tailMesh, 0.1388594f, -4.704177f, -0.076594f);
	m_renderer->TranslateMesh(m_copter3d.model,  m_copter3d.topMesh, 0.0f, -0.1106166f, -1.279995f);
	

    if (m_pause) return;
    m_dynamicsWorld->stepSimulation(timeDelta, 10);

    m_renderer->ScaleModelAbs(m_copter3d.model, 0.25f);
    if (!m_copter3d.dead)
    {
#define BLOCK3D_SPEED (block_speed[m_difficulty]*timeDelta/50.0f)

        bool leftMovement = keys[(int)VirtualKey::A] || keys[(int)VirtualKey::Left] || m_mouseMovement == 1;
        bool rightMovement = keys[(int)VirtualKey::D] || keys[(int)VirtualKey::Right] || m_mouseMovement == 2;

        if (leftMovement)
            m_renderer->RotateModelY(m_copter3d.model, XMConvertToRadians(10));
        if (rightMovement)
            m_renderer->RotateModelY(m_copter3d.model, XMConvertToRadians(-10));

#define COPTER3D_SPEED (copter_speed[m_difficulty]*timeDelta/50.0f)
        if (m_press)
        {
            if (m_copter3d.pos.y < 9.0f)
                m_copter3d.rigidBody->setLinearVelocity(btVector3(0, copter_speed[m_difficulty] / 50.0f, 0));
            else
                m_copter3d.rigidBody->setLinearVelocity(btVector3(0, 0, 0));
            //m_copter3d.pos.y += COPTER3D_SPEED;
        }
        //else
        //if (m_copter3d.pos.y > -9.0f)
        //m_copter3d.pos.y -= COPTER3D_SPEED;

        if (leftMovement)
        if (m_copter3d.pos.x > -9.0f)
            m_copter3d.rigidBody->setLinearVelocity(btVector3(-3, m_copter3d.rigidBody->getLinearVelocity().y(), 0));
        else m_copter3d.rigidBody->setLinearVelocity(btVector3(0, m_copter3d.rigidBody->getLinearVelocity().y(), 0));
        //m_copter3d.pos.x -= COPTER3D_SPEED;
        if (rightMovement)
        if (m_copter3d.pos.x < 9.0f)
            m_copter3d.rigidBody->setLinearVelocity(btVector3(3, m_copter3d.rigidBody->getLinearVelocity().y(), 0));
        else m_copter3d.rigidBody->setLinearVelocity(btVector3(0, m_copter3d.rigidBody->getLinearVelocity().y(), 0));
        //m_copter3d.pos.x += COPTER3D_SPEED;

        if (leftMovement && rightMovement)
            m_copter3d.rigidBody->setLinearVelocity(btVector3(0, m_copter3d.rigidBody->getLinearVelocity().y(), 0));
    }
	m_lastobsz -= BLOCK3D_SPEED;
	float endz = 50.0f;
	if (m_lastobsz <= endz - 2.0f)
	{
		m_lastobsz += 2.0f;
		AddObstacle3d(m_lastobsz);
	}

    btTransform trans;
    m_copter3d.rigidBody->getMotionState()->getWorldTransform(trans);
    m_copter3d.pos = XMFLOAT3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
    m_renderer->RotateModelAbs(m_copter3d.model, trans.getRotation().w(), trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z());
    m_renderer->TranslateModel(m_copter3d.model, m_copter3d.pos.x, m_copter3d.pos.y, m_copter3d.pos.z);


	if (!m_blocks3d.empty())
	{
		for (auto i = m_blocks3d.begin(); i != m_blocks3d.end(); ++i)
        {
            btTransform trans;
            //i->rigidBody->setWorldTransform()
            i->rigidBody->getMotionState()->getWorldTransform(trans);
            auto newpos = trans.getOrigin();
            //newpos.setZ(newpos.getZ() - BLOCK3D_SPEED);
            //trans.setOrigin(newpos);
            //i->rigidBody->getMotionState()->setWorldTransform(trans);
            i->x = newpos.getX(); i->y = newpos.getY(); i->z = newpos.getZ();
		//	block3d* bk = &m_block3d_obj[i->bk];
		//	if (i->z + bk->width / 2.0f > m_copter3d.pos.z + COPTER3D_MINZ2 && i->z - bk->width / 2.0f < m_copter3d.pos.z + COPTER3D_MAXZ1)
		//	{
		//		/*if (CheckCollisionCopter(i->x-bk->length/2.0f, i->x+bk->length/2.0f, i->y-bk->height/2.0f, i->y+bk->height/2.0f,
		//		i->z-bk->width/2.0f, i->z+bk->width/2.0f,
		//		m_copter3d.pos.x, m_copter3d.pos.y, m_copter3d.pos.z))*/
		//		if (CheckCollisionCopter(i->x, i->y, i->z, bk->length / 2.0f, bk->height / 2.0f, bk->width / 2.0f,
		//			m_copter3d.pos.x, m_copter3d.pos.y, m_copter3d.pos.z))
		//			m_copter3d.dead = true;
		//	}
		}

        while (!m_blocks3d.empty() && m_blocks3d.begin()->z < m_copter3d.pos.z - 5.0f)
        {
            auto i = m_blocks3d.begin();
            if (i->rigidBody)
            {
                m_dynamicsWorld->removeRigidBody(i->rigidBody);
                delete i->rigidBody->getMotionState();
                delete i->rigidBody;
                i->rigidBody = NULL;
            }
            m_blocks3d.pop_front();
        }
	}
		
    
	m_renderer->SetCamEye(cx+m_copter3d.pos.x,cy+m_copter3d.pos.y,cz+m_copter3d.pos.z);
	m_renderer->SetCamAt(m_copter3d.pos.x,m_copter3d.pos.y + 0.5f,m_copter3d.pos.z);

	if (timeDelta < 0.1)
	{
		m_gone+=timeDelta;
		if (!m_copter3d.dead) m_score+=timeDelta*10;
        //else
        {
            ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
            auto value = localSettings->Values->Lookup(m_difficulty == 0 ? "BestScore3DEasy" : "BestScore3DHard");
            int bscore = 0;
            if (value) bscore = safe_cast<int>(value);
            if (bscore < (int)m_score)
                localSettings->Values->Insert(m_difficulty == 0 ? "BestScore3DEasy" : "BestScore3DHard", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)m_score)));
        }
	}
	if (m_gone>5)
	{
		m_maxby++;
		m_gone=0;
	}

    if (!m_copter3d.dead)
    for (int i = 0; i < m_dispatcher->getNumManifolds(); ++i)
    {
        btPersistentManifold* pManifold = m_dispatcher->getManifoldByIndexInternal(i);
        if (pManifold->getNumContacts() > 0)
        {
            const btRigidBody* pBody0 = static_cast<const btRigidBody*>(pManifold->getBody0());
            const btRigidBody* pBody1 = static_cast<const btRigidBody*>(pManifold->getBody1());

            if (pBody0 == m_copter3d.rigidBody || pBody1 == m_copter3d.rigidBody)
            {
                m_copter3d.dead = true;
                m_audioManager->Stop(m_copterSound);
                m_audioManager->Play(m_explosionSound);
            }
        }
    }

}

void Game::Render3d()
{
	m_renderer->BeginRender3D();
	//if (!m_copter3d.dead)
	m_renderer->DrawModel(m_copter3d.model);
	
	/*m_renderer->ScaleModelAbs(blck, COPTER3D_MAXX1 - COPTER3D_MINX1, COPTER3D_MAXY1 - COPTER3D_MINY1, COPTER3D_MAXZ1 - COPTER3D_MINZ1);
	m_renderer->TranslateModel(blck, m_copter3d.pos.x + (COPTER3D_MAXX1 + COPTER3D_MINX1) / 2.0f,
		m_copter3d.pos.y + (COPTER3D_MAXY1 + COPTER3D_MINY1) / 2.0f,
		m_copter3d.pos.z + (COPTER3D_MAXZ1 + COPTER3D_MINZ1) / 2.0f);
	m_renderer->DrawModel(blck);
	m_renderer->ScaleModelAbs(blck, COPTER3D_MAXX2 - COPTER3D_MINX2, COPTER3D_MAXY2 - COPTER3D_MINY2, COPTER3D_MAXZ2 - COPTER3D_MINZ2);
	m_renderer->TranslateModel(blck, m_copter3d.pos.x + (COPTER3D_MAXX2 + COPTER3D_MINX2) / 2.0f,
		m_copter3d.pos.y + (COPTER3D_MAXY2 + COPTER3D_MINY2) / 2.0f,
		m_copter3d.pos.z + (COPTER3D_MAXZ2 + COPTER3D_MINZ2) / 2.0f);
	m_renderer->DrawModel(blck);*/

	if (!m_blocks3d.empty())
	{
		for (auto i=m_blocks3d.begin(); i!=m_blocks3d.end();++i)
		{
			block3d* bk = &m_block3d_obj[i->bk];
			m_renderer->ScaleModelAbs(m_blockModel, bk->length, bk->height, bk->width);
			m_renderer->TranslateModel(m_blockModel, i->x, i->y, i->z);
			m_renderer->DrawModel(m_blockModel);
		}
	}

    m_renderer->DrawModel(m_groundModel);
	
	
	m_renderer->DrawText(0, L"Distance: " + ((int)m_score).ToString(), 20,20,400,50);
    ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
    auto values = localSettings->Values;
    auto value = localSettings->Values->Lookup(m_difficulty == 0 ? "BestScore3DEasy" : "BestScore3DHard");
    int bscore = 0;
    if (value) bscore = safe_cast<int>(value);
    m_renderer->DrawText(0, L"Best Distance: " + bscore.ToString(), 20, 60, 400, 50);
    m_renderer->DrawText(0, L"Difficulty: " + (m_difficulty == 0 ? "Easy" : "Hard"), 20, 100, 400, 50);

	if (!m_copter3d.dead) return;
	if (m_copter3d.expl_dead) 
	{
		//m_renderer->DrawText(0, L"You Lost!", -1, -1, 100.0f, 100.0f);
        m_displayingMenu = true;
		return;
	}
	/*
	m_renderer->BeginRender2D();
	//m_renderer->NeverHide(true);
	m_renderer->DrawBillboard(m_copter.spr_expl.sprid, m_copter3d.pos.x, m_copter3d.pos.y, m_copter3d.pos.z-0.1f, m_copter.spr_expl.imgid, 0.0f, 0.0f, 10.0f, 10.0f);
	//m_renderer->NeverHide(false);
	*/
}
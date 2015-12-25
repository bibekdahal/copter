#pragma once

#include "pch.h"
#include "Content\Renderer.h"
#include "Content\AudioManager.h"
#include <list>

#include "misc.h"

namespace Copter
{
	struct sprite
	{
		float anim_tm;
		unsigned sprid;
		unsigned imgid;
	};

	struct block
	{
		unsigned spr;
		float width, height;
	};

	struct block_ins
	{
		unsigned bk;
		float x, y;
	};

	struct block3d
	{
        float length, width, height;
        btCollisionShape* shape;
	};

	struct block3d_ins
	{
		unsigned bk;
        float x, y, z;
        btRigidBody* rigidBody;
	};

	struct copter
	{
		DirectX::XMFLOAT2 pos;
        float vely;
		sprite spr;
		sprite spr_expl;
		bool dead;
		bool expl_dead;
	};

	struct copter3d
	{
		DirectX::XMFLOAT3 pos;
		unsigned model;
		unsigned topMesh;
		unsigned tailMesh;
		float rotAngle;
		bool dead;
		bool expl_dead;

        btRigidBody* rigidBody;
        btCollisionShape* shape;
	};

	ref class Game sealed
	{
	public:
		Game();
		void Initialize(Renderer^ renderer);
        void CleanUp();
        void NewGame(int game = -1);
		void Update(float timeTotal, float timeDelta);
		void Render();
		void Uninitialize();
		void UpdateSize();

		void Pause(int pause = -1);
		void ChangeDifficulty(int difficulty = -1);

        void PointerPressed(int mx, int my);
        void PointerMoved(int mx, int my);
		void PointerReleased();

		void keyInput(Windows::UI::Core::KeyEventArgs^ Args);
		void keyInputUp(Windows::UI::Core::KeyEventArgs^ Args);

        void SetMouseMovement(int mv) { m_mouseMovement = mv; }

		virtual ~Game();
	private:
        bool keys[256];
        int m_mouseMovement;

        void NewGame2d();
        void Update2d(float timeTotal, float timeDelta);
        void Render2d();

        void NewGame3d();
        void Update3d(float timeTotal, float timeDelta);
        void Render3d();
        void ClearBlocks3d();

		void Initialize2d();
		void Initialize3d();
		Renderer^ m_renderer;

        void DrawMenu();

		copter m_copter;
		copter3d m_copter3d;

		std::vector<DirectX::XMFLOAT2> m_topbottom;
		std::list<DirectX::XMFLOAT2> m_etop;
		std::list<DirectX::XMFLOAT2> m_ebottom;

		std::vector<block> m_block_obj;
		std::list<block_ins> m_blocks;
		unsigned m_spr_bar;

		std::vector<block3d> m_block3d_obj;
		std::list<block3d_ins> m_blocks3d;
		unsigned m_blockModel;

		float m_wwidth;
		float m_wheight;

		void AddBlocks(float x);
		float m_lastx;

		void AddObstacle(float x);
		float m_lastobsx;

		void AddObstacle3d(float z);
		float m_lastobsz;

		bool m_press;
		int m_maxby;
		double m_gone;
		bool m_pause;
		double m_score;
		int m_difficulty;
		bool m_3d;

        btBroadphaseInterface* m_broadphase;
        btDefaultCollisionConfiguration* m_collisionConfiguration;
        btCollisionDispatcher* m_dispatcher;
        btSequentialImpulseConstraintSolver* m_solver;
        btDiscreteDynamicsWorld* m_dynamicsWorld;

        unsigned int m_groundModel;
        btRigidBody* m_ground;
        btRigidBody* m_ceiling;
        btCollisionShape* m_gcshape;


        AudioManager^ m_audioManager;
        unsigned int m_copterSound;
        unsigned int m_explosionSound;

        unsigned int m_button, m_menu, m_ins2d, m_ins3d;
        int m_selection;
        bool m_displayingMenu, m_justNewed;
	};
};


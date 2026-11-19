#include "CppUnitTest.h"
#include "Bullet.h"
#include "Constants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    TEST_CLASS(BulletManagerTests)
    {
        BulletManager bm;

    public:
        TEST_METHOD(SpawnPlayer_CriaProjetilQueSobe)
        {
            bm.spawnPlayer({100.f, 400.f});
            Assert::AreEqual(size_t(1), bm.getAll().size());
            const auto &b = bm.getAll()[0];
            Assert::IsTrue(b.owner == BulletOwner::Player);
            Assert::IsTrue(b.vel.y < 0.f);
            Assert::IsTrue(b.alive);
        }

        TEST_METHOD(SpawnEnemy_CriaProjetilQueDesceComACor)
        {
            bm.spawnEnemy({100.f, 100.f}, sf::Color::Green);
            const auto &b = bm.getAll()[0];
            Assert::IsTrue(b.owner == BulletOwner::Enemy);
            Assert::IsTrue(b.vel.y > 0.f);
            Assert::IsTrue(b.color == sf::Color::Green);
        }

        TEST_METHOD(SpawnTriple_CriaTresProjeteis)
        {
            bm.spawnTriple({450.f, 600.f});
            Assert::AreEqual(size_t(3), bm.getAll().size());
            Assert::IsTrue(bm.getAll()[0].vel.x < 0.f);
            Assert::AreEqual(0.f, bm.getAll()[1].vel.x, 0.001f);
            Assert::IsTrue(bm.getAll()[2].vel.x > 0.f);
        }

        TEST_METHOD(Update_MoveProjetilConformeVelocidade)
        {
            bm.spawnPlayer({100.f, 400.f});
            bm.update(0.1f);
            Assert::AreEqual(400.f - Cfg::PLAYER_BULLET_SPD * 0.1f, bm.getAll()[0].pos.y, 0.01f);
        }

        TEST_METHOD(Update_RemoveProjetilForaDaTela)
        {
            bm.spawnPlayer({100.f, 10.f});
            bm.update(1.f);
            Assert::AreEqual(size_t(0), bm.getAll().size());
        }

        TEST_METHOD(Update_RemoveProjetilMarcadoComoMorto)
        {
            bm.spawnPlayer({100.f, 400.f});
            bm.getAll()[0].alive = false;
            bm.update(0.01f);
            Assert::AreEqual(size_t(0), bm.getAll().size());
        }

        TEST_METHOD(KillAll_LimpaTudo)
        {
            bm.spawnTriple({100.f, 400.f});
            bm.spawnEnemy({200.f, 100.f});
            bm.killAll();
            Assert::AreEqual(size_t(0), bm.getAll().size());
        }
    };
}

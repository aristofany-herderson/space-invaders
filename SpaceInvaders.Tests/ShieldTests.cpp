#include "CppUnitTest.h"
#include "Shield.h"
#include "Constants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    TEST_CLASS(ShieldTests)
    {
        // Conta blocos vivos de um escudo.
        static int vivos(const Shield &s)
        {
            int n = 0;
            for (const auto &b : s.getBlocks())
                if (b.alive)
                    ++n;
            return n;
        }

        static sf::Vector2f centroDoPrimeiroBloco(const Shield &s)
        {
            return s.getBlocks().front().pos +
                   sf::Vector2f{Cfg::SHIELD_BLOCK / 2.f, Cfg::SHIELD_BLOCK / 2.f};
        }

    public:
        TEST_METHOD(Init_CriaBlocosComVidaCheia)
        {
            Shield s;
            s.init({100.f, 500.f});
            Assert::IsFalse(s.getBlocks().empty());
            for (const auto &b : s.getBlocks())
            {
                Assert::AreEqual(3, b.hp);
                Assert::IsTrue(b.alive);
            }
        }

        TEST_METHOD(Init_LimitesCorrespondemAoTamanhoConfigurado)
        {
            Shield s;
            s.init({100.f, 500.f});
            auto b = s.getBounds();
            Assert::AreEqual(100.f, b.position.x, 0.001f);
            Assert::AreEqual(500.f, b.position.y, 0.001f);
            Assert::AreEqual(Cfg::SHIELD_COLS * Cfg::SHIELD_BLOCK, b.size.x, 0.001f);
            Assert::AreEqual(Cfg::SHIELD_ROWS * Cfg::SHIELD_BLOCK, b.size.y, 0.001f);
        }

        TEST_METHOD(CheckBulletHit_NoBloco_RetornaTrueEReduzVida)
        {
            Shield s;
            s.init({0.f, 0.f});
            Assert::IsTrue(s.checkBulletHit(centroDoPrimeiroBloco(s)));
            Assert::AreEqual(2, s.getBlocks().front().hp);
        }

        TEST_METHOD(CheckBulletHit_ForaDoEscudo_RetornaFalse)
        {
            Shield s;
            s.init({0.f, 0.f});
            int antes = vivos(s);
            Assert::IsFalse(s.checkBulletHit({5000.f, 5000.f}));
            Assert::AreEqual(antes, vivos(s));
        }

        TEST_METHOD(TresTiros_DestroemOBloco)
        {
            Shield s;
            s.init({0.f, 0.f});
            int antes = vivos(s);
            for (int i = 0; i < 3; ++i)
                s.checkBulletHit(centroDoPrimeiroBloco(s));
            Assert::IsFalse(s.getBlocks().front().alive);
            Assert::AreEqual(antes - 1, vivos(s));
        }

        TEST_METHOD(Damageable_TakeDamageNoBloco_ReduzVidaERetornaDamaged)
        {
            Shield s;
            s.init({0.f, 0.f});
            Damageable &alvo = s;
            Assert::IsTrue(alvo.takeDamage(1, centroDoPrimeiroBloco(s)) == DamageResult::Damaged);
            Assert::AreEqual(2, s.getBlocks().front().hp);
        }

        TEST_METHOD(Damageable_TakeDamageForaDosBlocos_EhIgnorado)
        {
            Shield s;
            s.init({0.f, 0.f});
            Assert::IsTrue(s.takeDamage(1, {5000.f, 5000.f}) == DamageResult::Ignored);
        }

        TEST_METHOD(Damageable_IsHitBy_ConcordaComOsBlocos)
        {
            Shield s;
            s.init({0.f, 0.f});
            Assert::IsTrue(s.isHitBy(centroDoPrimeiroBloco(s), Cfg::BULLET_RADIUS));
            Assert::IsFalse(s.isHitBy({5000.f, 5000.f}, Cfg::BULLET_RADIUS));
        }

        TEST_METHOD(Damageable_DanoMaiorQueAVida_DestroiOBlocoEmUmGolpe)
        {
            Shield s;
            s.init({0.f, 0.f});
            s.takeDamage(3, centroDoPrimeiroBloco(s));
            Assert::IsFalse(s.getBlocks().front().alive);
        }

        TEST_METHOD(Damageable_DestruirTodosOsBlocos_RetornaDeadEIsAliveFalso)
        {
            Shield s;
            s.init({0.f, 0.f});
            DamageResult r = DamageResult::Ignored;
            for (const auto &b : s.getBlocks())
                r = s.takeDamage(3, b.pos + sf::Vector2f{Cfg::SHIELD_BLOCK / 2.f, Cfg::SHIELD_BLOCK / 2.f});
            Assert::IsTrue(r == DamageResult::Dead);
            Assert::IsFalse(s.isAlive());
        }

        TEST_METHOD(ShieldManager_ResetRestauraEscudos)
        {
            ShieldManager m;
            m.init();
            // Centro do primeiro escudo: sempre cai sobre blocos (formato cheio no meio).
            float x = float(Cfg::W) / (Cfg::SHIELD_COUNT + 1);
            float y = float(Cfg::H) - 160.f + 3.5f * Cfg::SHIELD_BLOCK;

            // Destroi todos os blocos que o ponto alcanca.
            int acertos = 0;
            while (m.checkBulletHit({x, y}, 2.f) && acertos < 1000)
                ++acertos;
            Assert::IsTrue(acertos > 0);
            Assert::IsFalse(m.checkBulletHit({x, y}, 2.f));

            m.reset();
            Assert::IsTrue(m.checkBulletHit({x, y}, 2.f));
        }
    };
}

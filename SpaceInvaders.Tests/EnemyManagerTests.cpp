#include "CppUnitTest.h"
#include "EnemyManager.h"
#include "Constants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    TEST_CLASS(EnemyManagerTests)
    {
        EnemyManager em;

    public:
        TEST_METHOD(SpawnFormation_NenhumInimigoNasceMorto)
        {
            em.spawnFormation(1);
            Assert::IsFalse(em.allDead());
        }

        TEST_METHOD(SpawnFormation_LinhaMaisBaixaEstaNaPosicaoEsperada)
        {
            em.spawnFormation(1);
            float esperado = Cfg::FORM_START_Y + (Cfg::FORMATION_ROWS - 1) * Cfg::FORM_SPACING_Y;
            Assert::AreEqual(esperado, em.getLowestEnemyPos().y, 0.01f);
        }

        TEST_METHOD(SpawnFormation_CriaTodosOsInimigosDaFormacao)
        {
            em.spawnFormation(1);
            Assert::AreEqual(static_cast<std::size_t>(Cfg::FORMATION_ROWS * Cfg::FORMATION_COLS), em.getEnemies().size());
        }

        TEST_METHOD(SpawnFormation_UfoSoApareceDepoisDoIntervalo)
        {
            em.spawnFormation(1);
            Assert::IsFalse(em.hasUFO());
        }

        TEST_METHOD(SpawnFormation_NoInicio_NaoChegouAoChao)
        {
            em.spawnFormation(1);
            Assert::IsFalse(em.hasReachedBottom());
        }

        TEST_METHOD(SpawnFormation_ChamarDeNovo_ReiniciaFormacaoELimpaPowerups)
        {
            em.spawnFormation(1);
            float y1 = em.getLowestEnemyPos().y;
            em.spawnFormation(3);
            Assert::AreEqual(y1, em.getLowestEnemyPos().y, 0.01f);
            Assert::IsTrue(em.getPowerups().empty());
        }

        TEST_METHOD(Formacao_DesceComOTempo_EAcabaChegandoAoChao)
        {
            em.spawnFormation(1);
            BulletManager bullets;
            ParticleSystem fx;
            sf::Vector2f jogador{float(Cfg::W) / 2.f, float(Cfg::H) - 70.f};

            bool chegou = false;
            // ate 10 minutos simulados a 60 FPS
            for (int i = 0; i < 60 * 600 && !chegou; ++i)
            {
                em.update(1.f / 60.f, jogador, bullets, fx, [](sf::Vector2f, sf::Color, int) {});
                bullets.killAll(); // ignora tiros inimigos: so interessa o avanco da formacao
                chegou = em.hasReachedBottom();
            }
            Assert::IsTrue(chegou);
        }
    };

    TEST_CLASS(ConstantesTests)
    {
    public:
        TEST_METHOD(PontuacaoCrescePorTipoDeInimigo)
        {
            Assert::IsTrue(Cfg::SCORE_DRONE < Cfg::SCORE_INVADER);
            Assert::IsTrue(Cfg::SCORE_INVADER < Cfg::SCORE_BRUTE);
            Assert::IsTrue(Cfg::SCORE_BRUTE < Cfg::SCORE_KAMIKAZE);
            Assert::IsTrue(Cfg::SCORE_KAMIKAZE < Cfg::SCORE_ELITE);
            Assert::IsTrue(Cfg::SCORE_ELITE < Cfg::UFO_SCORE);
        }

        TEST_METHOD(IntervalosDeSorteioSaoValidos)
        {
            Assert::IsTrue(Cfg::ENEMY_SHOOT_MIN < Cfg::ENEMY_SHOOT_MAX);
            Assert::IsTrue(Cfg::UFO_INTERVAL_MIN < Cfg::UFO_INTERVAL_MAX);
            Assert::IsTrue(Cfg::POWERUP_DROP_CHANCE > 0.f && Cfg::POWERUP_DROP_CHANCE < 1.f);
        }
    };
}

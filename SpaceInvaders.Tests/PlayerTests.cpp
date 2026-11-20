#include "CppUnitTest.h"
#include "Player.h"
#include "Constants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Os graficos/audio nao sao carregados nos testes (SFX.play volta cedo quando o
// som nao foi carregado e GFX.player tem tamanho 0), entao a logica de
// dano/pontuacao do Player pode ser exercitada sem abrir janela.
namespace SpaceInvadersTests
{
    TEST_CLASS(PlayerTests)
    {
        Player player;
        ParticleSystem fx;
        BulletManager bullets;

        // Avanca o tempo o suficiente para a invulnerabilidade acabar.
        void esperaInvulnerabilidade()
        {
            player.update(Cfg::PLAYER_INVUL + 0.1f, bullets, fx);
        }

    public:
        TEST_METHOD_INITIALIZE(Prepara)
        {
            player.init();
        }

        TEST_METHOD(Init_EstadoInicial)
        {
            Assert::AreEqual(Cfg::PLAYER_LIVES, player.getLives());
            Assert::AreEqual(0, player.getScore());
            Assert::IsTrue(player.isAlive());
            Assert::IsFalse(player.isInvul());
            Assert::IsFalse(player.hasShield());
            Assert::AreEqual(float(Cfg::W) / 2.f, player.getPos().x, 0.001f);
        }

        TEST_METHOD(AddScore_Acumula)
        {
            player.addScore(10);
            player.addScore(250);
            Assert::AreEqual(260, player.getScore());
        }

        TEST_METHOD(AddLife_IncrementaVidasENotifica)
        {
            player.addLife();
            Assert::AreEqual(Cfg::PLAYER_LIVES + 1, player.getLives());
            Assert::IsTrue(player.hasLifeNotif());
        }

        TEST_METHOD(TakeDamage_PrimeiroDano_RetornaDamagedEFicaInvulneravel)
        {
            auto r = player.takeDamage(1, {});
            Assert::IsTrue(r == DamageResult::Damaged);
            Assert::AreEqual(Cfg::PLAYER_LIVES - 1, player.getLives());
            Assert::IsTrue(player.isInvul());
        }

        TEST_METHOD(Damageable_JogadorTratadoPelaInterface)
        {
            Damageable &alvo = player;
            Assert::IsTrue(alvo.isAlive());
            Assert::IsTrue(alvo.takeDamage(1, {}) == DamageResult::Damaged);
            Assert::AreEqual(Cfg::PLAYER_LIVES - 1, player.getLives());
        }

        TEST_METHOD(IsHitBy_PontoNoCentro_Acerta)
        {
            Assert::IsTrue(player.isHitBy(player.getPos(), 1.f));
            Assert::IsFalse(player.isHitBy({0.f, 0.f}, 0.f));
        }

        TEST_METHOD(TakeDamage_ValorNaoPositivo_EhIgnorado)
        {
            Assert::IsTrue(player.takeDamage(0, {}) == DamageResult::Ignored);
            Assert::AreEqual(Cfg::PLAYER_LIVES, player.getLives());
        }

        TEST_METHOD(TakeDamage_DuranteInvulnerabilidade_EhIgnorado)
        {
            player.takeDamage(1, {});
            auto r = player.takeDamage(1, {});
            Assert::IsTrue(r == DamageResult::Ignored);
            Assert::AreEqual(Cfg::PLAYER_LIVES - 1, player.getLives());
        }

        TEST_METHOD(TakeDamage_ComEscudo_EhIgnorado)
        {
            player.applyPowerup(PowerupType::Shield);
            Assert::IsTrue(player.hasShield());
            auto r = player.takeDamage(1, {});
            Assert::IsTrue(r == DamageResult::Ignored);
            Assert::AreEqual(Cfg::PLAYER_LIVES, player.getLives());
        }

        TEST_METHOD(TakeDamage_UltimaVida_RetornaDead)
        {
            DamageResult r = DamageResult::Ignored;
            for (int i = 0; i < Cfg::PLAYER_LIVES; ++i)
            {
                r = player.takeDamage(1, {});
                if (i < Cfg::PLAYER_LIVES - 1)
                    esperaInvulnerabilidade();
            }
            Assert::IsTrue(r == DamageResult::Dead);
            Assert::IsFalse(player.isAlive());
            Assert::AreEqual(0, player.getLives());
        }

        TEST_METHOD(Invulnerabilidade_Expira)
        {
            player.takeDamage(1, {});
            esperaInvulnerabilidade();
            Assert::IsFalse(player.isInvul());
        }

        TEST_METHOD(ApplyPowerup_Rapid_AtivaTemporizador)
        {
            player.applyPowerup(PowerupType::Rapid);
            Assert::AreEqual(Cfg::POWERUP_DURATION, player.getRapidTimer(), 0.001f);
        }

        TEST_METHOD(ApplyPowerup_Triple_AtivaTemporizador)
        {
            player.applyPowerup(PowerupType::Triple);
            Assert::AreEqual(Cfg::POWERUP_DURATION, player.getTripleTimer(), 0.001f);
        }

        TEST_METHOD(ApplyPowerup_None_NaoAlteraNada)
        {
            player.applyPowerup(PowerupType::None);
            Assert::AreEqual(0.f, player.getRapidTimer(), 0.001f);
            Assert::AreEqual(0.f, player.getTripleTimer(), 0.001f);
            Assert::IsFalse(player.hasShield());
        }

        TEST_METHOD(Escudo_Expira_ComOTempo)
        {
            player.applyPowerup(PowerupType::Shield);
            player.update(Cfg::POWERUP_DURATION + 0.1f, bullets, fx);
            Assert::IsFalse(player.hasShield());
        }

        TEST_METHOD(Init_ReiniciaAposPartida)
        {
            player.addScore(500);
            player.takeDamage(1, {});
            player.applyPowerup(PowerupType::Shield);
            player.init();
            Assert::AreEqual(0, player.getScore());
            Assert::AreEqual(Cfg::PLAYER_LIVES, player.getLives());
            Assert::IsFalse(player.hasShield());
            Assert::IsFalse(player.isInvul());
        }
    };
}

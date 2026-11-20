#include "CppUnitTest.h"
#include "EnemyFactory.h"
#include "Constants.h"
#include <memory>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    TEST_CLASS(EnemyFactoryTests)
    {
    public:
        TEST_METHOD(CreateFormation_GeraLinhasVezesColunas)
        {
            auto formation = EnemyFactory::createFormation(1);
            Assert::AreEqual(static_cast<std::size_t>(Cfg::FORMATION_ROWS * Cfg::FORMATION_COLS), formation.size());
        }

        TEST_METHOD(CreateFormation_CadaLinhaTemOTipoDaFabrica)
        {
            auto formation = EnemyFactory::createFormation(1);
            for (int r = 0; r < Cfg::FORMATION_ROWS; ++r)
                for (int c = 0; c < Cfg::FORMATION_COLS; ++c)
                    Assert::IsTrue(formation[r * Cfg::FORMATION_COLS + c]->type() == EnemyFactory::typeForRow(r));
        }

        TEST_METHOD(CreateFormation_PosicionaNaGrade)
        {
            auto formation = EnemyFactory::createFormation(1);
            auto last = formation.back()->pos();
            Assert::AreEqual(Cfg::FORM_START_X + (Cfg::FORMATION_COLS - 1) * Cfg::FORM_SPACING_X, last.x, 0.01f);
            Assert::AreEqual(Cfg::FORM_START_Y + (Cfg::FORMATION_ROWS - 1) * Cfg::FORM_SPACING_Y, last.y, 0.01f);
        }

        TEST_METHOD(CreateFormation_NenhumInimigoNasceMorto)
        {
            for (const auto &e : EnemyFactory::createFormation(1))
                Assert::IsTrue(e->isAlive());
        }

        TEST_METHOD(Create_DevolveASubclasseCorrespondenteAoTipo)
        {
            Assert::IsTrue(dynamic_cast<Drone *>(EnemyFactory::create(EnemyType::Drone, {}, 1).get()) != nullptr);
            Assert::IsTrue(dynamic_cast<Invader *>(EnemyFactory::create(EnemyType::Invader, {}, 1).get()) != nullptr);
            Assert::IsTrue(dynamic_cast<Brute *>(EnemyFactory::create(EnemyType::Brute, {}, 1).get()) != nullptr);
            Assert::IsTrue(dynamic_cast<Kamikaze *>(EnemyFactory::create(EnemyType::Kamikaze, {}, 1).get()) != nullptr);
            Assert::IsTrue(dynamic_cast<Elite *>(EnemyFactory::create(EnemyType::Elite, {}, 1).get()) != nullptr);
            Assert::IsTrue(dynamic_cast<UFO *>(EnemyFactory::create(EnemyType::UFO, {}, 1).get()) != nullptr);
        }

        TEST_METHOD(Create_TipoInvalido_LancaInvalidArgument)
        {
            bool lancou = false;
            try
            {
                EnemyFactory::create(static_cast<EnemyType>(99), {}, 1);
            }
            catch (const std::invalid_argument &)
            {
                lancou = true;
            }
            Assert::IsTrue(lancou);
        }

        TEST_METHOD(CreateUfo_NasceForaDaTelaIndoParaDentro)
        {
            auto ufo = EnemyFactory::createUfo(1);
            Assert::IsTrue(ufo->pos().x < 0.f || ufo->pos().x > float(Cfg::W));
            Assert::IsFalse(ufo->isGone());
        }
    };

    TEST_CLASS(EnemyHierarchyTests)
    {
        FormationState form;

        std::unique_ptr<Enemy> make(EnemyType t)
        {
            return EnemyFactory::create(t, {100.f, 100.f}, 1);
        }

    public:
        TEST_METHOD(PontuacaoPorSubclasse)
        {
            Assert::AreEqual(Cfg::SCORE_DRONE, make(EnemyType::Drone)->scoreValue());
            Assert::AreEqual(Cfg::SCORE_INVADER, make(EnemyType::Invader)->scoreValue());
            Assert::AreEqual(Cfg::SCORE_BRUTE, make(EnemyType::Brute)->scoreValue());
            Assert::AreEqual(Cfg::SCORE_KAMIKAZE, make(EnemyType::Kamikaze)->scoreValue());
            Assert::AreEqual(Cfg::SCORE_ELITE, make(EnemyType::Elite)->scoreValue());
            Assert::AreEqual(Cfg::UFO_SCORE, make(EnemyType::UFO)->scoreValue());
        }

        TEST_METHOD(VidaPorSubclasse)
        {
            Assert::AreEqual(1, make(EnemyType::Drone)->maxHp());
            Assert::AreEqual(1, make(EnemyType::Invader)->maxHp());
            Assert::AreEqual(2, make(EnemyType::Brute)->maxHp());
            Assert::AreEqual(1, make(EnemyType::Kamikaze)->maxHp());
            Assert::AreEqual(3, make(EnemyType::Elite)->maxHp());
        }

        TEST_METHOD(CorPorSubclasse)
        {
            Assert::IsTrue(make(EnemyType::Drone)->color() == Cfg::COL_DRONE);
            Assert::IsTrue(make(EnemyType::Elite)->color() == Cfg::COL_ELITE);
            Assert::IsTrue(make(EnemyType::UFO)->color() == Cfg::COL_UFO);
        }

        TEST_METHOD(TakeDamage_Brute_PrecisaDeDoisAcertos)
        {
            auto e = make(EnemyType::Brute);
            Assert::IsTrue(e->takeDamage(1, {}) == DamageResult::Damaged);
            Assert::IsTrue(e->isAlive());
            Assert::IsTrue(e->takeDamage(1, {}) == DamageResult::Dead);
            Assert::IsFalse(e->isAlive());
        }

        TEST_METHOD(TakeDamage_Elite_PrecisaDeTresAcertos)
        {
            auto e = make(EnemyType::Elite);
            Assert::IsTrue(e->takeDamage(1, {}) == DamageResult::Damaged);
            Assert::IsTrue(e->takeDamage(1, {}) == DamageResult::Damaged);
            Assert::IsTrue(e->takeDamage(1, {}) == DamageResult::Dead);
        }

        TEST_METHOD(TakeDamage_EmInimigoMorto_EhIgnorado)
        {
            auto e = make(EnemyType::Drone);
            e->takeDamage(1, {});
            Assert::IsTrue(e->takeDamage(1, {}) == DamageResult::Ignored);
        }

        TEST_METHOD(TakeDamage_ValorNaoPositivo_EhIgnorado)
        {
            auto e = make(EnemyType::Drone);
            Assert::IsTrue(e->takeDamage(0, {}) == DamageResult::Ignored);
            Assert::IsTrue(e->isAlive());
        }

        TEST_METHOD(Polimorfismo_DamageableTrataInimigoDeFormaUniforme)
        {
            std::unique_ptr<Enemy> e = make(EnemyType::Invader);
            Damageable &alvo = *e;
            Assert::IsTrue(alvo.isAlive());
            Assert::IsTrue(alvo.isHitBy(e->pos(), 0.f));
            Assert::IsTrue(alvo.takeDamage(1, e->pos()) == DamageResult::Dead);
            Assert::IsFalse(alvo.isAlive());
        }

        TEST_METHOD(IsHitBy_PontoDistante_NaoAcerta)
        {
            auto e = make(EnemyType::Drone);
            Assert::IsFalse(e->isHitBy({5000.f, 5000.f}, 0.f));
        }

        TEST_METHOD(Morte_IniciaAnimacaoDeExplosaoQueTermina)
        {
            auto e = make(EnemyType::Drone);
            e->takeDamage(1, {});
            Assert::IsTrue(e->isExploding());
            for (int i = 0; i < 10; ++i)
                e->updateExplosion(0.11f);
            Assert::IsFalse(e->isExploding());
        }

        TEST_METHOD(Invader_NaoSeMoveForaDaFormacao)
        {
            auto e = make(EnemyType::Invader);
            BulletManager bullets;
            e->update(0.016f, {0.f, 0.f}, form, bullets);
            Assert::AreEqual(100.f, e->pos().x, 0.001f);
            Assert::AreEqual(100.f, e->pos().y, 0.001f);
        }

        TEST_METHOD(Formacao_AcompanhaDeslocamentoLateralEVertical)
        {
            auto e = make(EnemyType::Invader);
            BulletManager bullets;
            form.offsetX = 30.f;
            form.stepY = 8.f;
            e->update(0.016f, {0.f, 0.f}, form, bullets);
            Assert::AreEqual(130.f, e->pos().x, 0.001f);
            Assert::AreEqual(108.f, e->pos().y, 0.001f);
        }

        TEST_METHOD(Drone_OscilaHorizontalmenteNoMaximoDoLimite)
        {
            auto e = make(EnemyType::Drone);
            BulletManager bullets;
            for (int i = 0; i < 200; ++i)
            {
                e->update(0.016f, {0.f, 0.f}, form, bullets);
                Assert::IsTrue(std::abs(e->pos().x - 100.f) <= 14.01f);
            }
        }

        TEST_METHOD(Elite_Atira_EmTresDirecoes)
        {
            auto e = make(EnemyType::Elite);
            BulletManager bullets;
            for (int i = 0; i < 60 * 30 && bullets.getAll().empty(); ++i)
                e->update(1.f / 60.f, {0.f, 0.f}, form, bullets);
            Assert::AreEqual(static_cast<std::size_t>(3), bullets.getAll().size());
        }

        TEST_METHOD(Drone_Atira_UmTiroReto)
        {
            auto e = make(EnemyType::Drone);
            BulletManager bullets;
            for (int i = 0; i < 60 * 30 && bullets.getAll().empty(); ++i)
                e->update(1.f / 60.f, {0.f, 0.f}, form, bullets);
            Assert::AreEqual(static_cast<std::size_t>(1), bullets.getAll().size());
            Assert::AreEqual(0.f, bullets.getAll().front().vel.x, 0.001f);
        }

        TEST_METHOD(Kamikaze_ForaDoMergulho_NaoAbalroa)
        {
            auto e = make(EnemyType::Kamikaze);
            Assert::IsFalse(e->rams(e->pos()));
        }

        TEST_METHOD(OutrosInimigos_NuncaAbalroam)
        {
            Assert::IsFalse(make(EnemyType::Drone)->rams({100.f, 100.f}));
            Assert::IsFalse(make(EnemyType::Elite)->rams({100.f, 100.f}));
        }
    };

    TEST_CLASS(UfoTests)
    {
        FormationState form;

    public:
        TEST_METHOD(Ufo_AtravessaATelaENaoAtira)
        {
            auto ufo = EnemyFactory::create(EnemyType::UFO, {-60.f, 80.f}, 1);
            BulletManager bullets;
            float x0 = ufo->pos().x;
            for (int i = 0; i < 60 * 30; ++i)
                ufo->update(1.f / 60.f, {0.f, 0.f}, form, bullets);
            Assert::IsTrue(ufo->pos().x > x0);
            Assert::IsTrue(ufo->isGone());
            Assert::IsTrue(bullets.getAll().empty());
        }

        TEST_METHOD(Ufo_VindoDaDireita_MoveParaEsquerda)
        {
            auto ufo = EnemyFactory::create(EnemyType::UFO, {float(Cfg::W) + 60.f, 80.f}, 1);
            BulletManager bullets;
            float x0 = ufo->pos().x;
            ufo->update(0.1f, {0.f, 0.f}, form, bullets);
            Assert::IsTrue(ufo->pos().x < x0);
        }

        TEST_METHOD(Ufo_AcertoPorProximidade)
        {
            auto ufo = EnemyFactory::create(EnemyType::UFO, {400.f, 80.f}, 1);
            Assert::IsTrue(ufo->isHitBy({410.f, 85.f}, 0.f));
            Assert::IsFalse(ufo->isHitBy({480.f, 80.f}, 0.f));
        }
    };
}

#include "CppUnitTest.h"
#include "Storage.h"
#include "Exceptions.h"
#include <filesystem>
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    namespace
    {
        // Arquivo temporario removido automaticamente ao fim de cada teste.
        struct TempFile
        {
            std::string path;
            explicit TempFile(const std::string &name)
                : path((std::filesystem::temp_directory_path() / ("si_test_" + name)).string())
            {
                std::error_code ec;
                std::filesystem::remove(path, ec);
            }
            ~TempFile()
            {
                std::error_code ec;
                std::filesystem::remove(path, ec);
            }
            void write(const std::string &content) const
            {
                std::ofstream f(path, std::ios::trunc);
                f << content;
            }
        };

        // true se f() lancar uma excecao do tipo Ex.
        template <class Ex, class F>
        bool lanca(F &&f)
        {
            try
            {
                f();
            }
            catch (const Ex &)
            {
                return true;
            }
            catch (...)
            {
            }
            return false;
        }
    }

    TEST_CLASS(StorageHighScoreTests)
    {
    public:
        TEST_METHOD(ArquivoInexistente_RetornaZero)
        {
            TempFile f("hs_inexistente.dat");
            Assert::AreEqual(0, Storage::readHighScore(f.path));
        }

        TEST_METHOD(GravaELe_IdaEVolta)
        {
            TempFile f("hs_roundtrip.dat");
            Storage::writeHighScore(f.path, 12345);
            Assert::AreEqual(12345, Storage::readHighScore(f.path));
        }

        TEST_METHOD(GravarNovamente_SobrescreveValorAntigo)
        {
            TempFile f("hs_sobrescreve.dat");
            Storage::writeHighScore(f.path, 99999);
            Storage::writeHighScore(f.path, 5);
            Assert::AreEqual(5, Storage::readHighScore(f.path));
        }

        TEST_METHOD(ConteudoNaoNumerico_LancaInvalidData)
        {
            TempFile f("hs_lixo.dat");
            f.write("abc");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readHighScore(f.path); }));
        }

        TEST_METHOD(ArquivoVazio_LancaInvalidData)
        {
            TempFile f("hs_vazio.dat");
            f.write("");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readHighScore(f.path); }));
        }

        TEST_METHOD(ValorNegativo_LancaInvalidData)
        {
            TempFile f("hs_negativo.dat");
            f.write("-50");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readHighScore(f.path); }));
        }

        TEST_METHOD(ValorGigante_LancaInvalidData)
        {
            TempFile f("hs_gigante.dat");
            f.write("99999999999999");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readHighScore(f.path); }));
        }

        TEST_METHOD(GravarNegativo_LancaInvalidArgument)
        {
            TempFile f("hs_grava_neg.dat");
            Assert::IsTrue(lanca<std::invalid_argument>([&] { Storage::writeHighScore(f.path, -1); }));
        }

        TEST_METHOD(GravarEmDiretorioInexistente_LancaPersistence)
        {
            std::string caminho = (std::filesystem::temp_directory_path() /
                                   "si_dir_que_nao_existe" / "hiscore.dat")
                                      .string();
            Assert::IsTrue(lanca<PersistenceException>([&] { Storage::writeHighScore(caminho, 10); }));
        }

        TEST_METHOD(ErroDeLeitura_PodeSerCapturadoComoGameException)
        {
            TempFile f("hs_game_exception.dat");
            f.write("xyz");
            Assert::IsTrue(lanca<GameException>([&] { Storage::readHighScore(f.path); }));
        }
    };

    TEST_CLASS(StorageVolumeTests)
    {
    public:
        TEST_METHOD(ArquivoInexistente_RetornaVolumePadrao)
        {
            TempFile f("vol_inexistente.dat");
            Assert::AreEqual(Storage::DEFAULT_VOLUME, Storage::readVolume(f.path), 0.001f);
        }

        TEST_METHOD(GravaELe_IdaEVolta)
        {
            TempFile f("vol_roundtrip.dat");
            Storage::writeVolume(f.path, 42.5f);
            Assert::AreEqual(42.5f, Storage::readVolume(f.path), 0.001f);
        }

        TEST_METHOD(LimitesZeroECem_SaoAceitos)
        {
            TempFile f("vol_limites.dat");
            Storage::writeVolume(f.path, 0.f);
            Assert::AreEqual(0.f, Storage::readVolume(f.path), 0.001f);
            Storage::writeVolume(f.path, 100.f);
            Assert::AreEqual(100.f, Storage::readVolume(f.path), 0.001f);
        }

        TEST_METHOD(ForaDaFaixa_LancaInvalidData)
        {
            TempFile f("vol_fora.dat");
            f.write("150");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readVolume(f.path); }));
            f.write("-3");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readVolume(f.path); }));
        }

        TEST_METHOD(NaoNumerico_LancaInvalidData)
        {
            TempFile f("vol_lixo.dat");
            f.write("alto");
            Assert::IsTrue(lanca<InvalidDataException>([&] { Storage::readVolume(f.path); }));
        }

        TEST_METHOD(GravarForaDaFaixa_LancaInvalidArgument)
        {
            TempFile f("vol_grava_fora.dat");
            Assert::IsTrue(lanca<std::invalid_argument>([&] { Storage::writeVolume(f.path, 101.f); }));
            Assert::IsTrue(lanca<std::invalid_argument>([&] { Storage::writeVolume(f.path, -0.5f); }));
        }
    };
}

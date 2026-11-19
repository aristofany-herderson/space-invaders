#include "CppUnitTest.h"
#include "Exceptions.h"
#include <cstring>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    TEST_CLASS(ExceptionsTests)
    {
    public:
        TEST_METHOD(AssetLoadException_GuardaCaminhoEMensagem)
        {
            AssetLoadException e("assets/fonts/font.ttf");
            Assert::AreEqual(std::string("assets/fonts/font.ttf"), e.path());
            Assert::IsNotNull(std::strstr(e.what(), "assets/fonts/font.ttf"));
        }

        TEST_METHOD(PersistenceException_GuardaCaminhoEMotivo)
        {
            PersistenceException e("hiscore.dat", "disco cheio");
            Assert::AreEqual(std::string("hiscore.dat"), e.path());
            Assert::IsNotNull(std::strstr(e.what(), "disco cheio"));
        }

        TEST_METHOD(InvalidDataException_EhPersistenceException)
        {
            bool capturada = false;
            try
            {
                throw InvalidDataException("settings.dat", "lixo");
            }
            catch (const PersistenceException &e)
            {
                capturada = true;
                Assert::AreEqual(std::string("settings.dat"), e.path());
            }
            Assert::IsTrue(capturada);
        }

        TEST_METHOD(TodasDerivamDeGameException)
        {
            auto lanca = [](int qual)
            {
                if (qual == 0)
                    throw AssetLoadException("a");
                if (qual == 1)
                    throw PersistenceException("b", "x");
                throw InvalidDataException("c", "y");
            };
            for (int i = 0; i < 3; ++i)
            {
                bool capturada = false;
                try
                {
                    lanca(i);
                }
                catch (const GameException &)
                {
                    capturada = true;
                }
                Assert::IsTrue(capturada);
            }
        }

        TEST_METHOD(GameException_EhStdRuntimeError)
        {
            bool capturada = false;
            try
            {
                throw GameException("falha");
            }
            catch (const std::runtime_error &e)
            {
                capturada = true;
                Assert::AreEqual(0, std::strcmp("falha", e.what()));
            }
            Assert::IsTrue(capturada);
        }
    };
}

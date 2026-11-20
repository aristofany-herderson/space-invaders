#include "CppUnitTest.h"
#include "RankingClient.h"
#include "Exceptions.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpaceInvadersTests
{
    TEST_CLASS(RankingClientTests)
    {
    public:
        TEST_METHOD(ParseScores_ListaVazia)
        {
            Assert::AreEqual(size_t(0), RankingClient::parseScores("[]").size());
        }

        TEST_METHOD(ParseScores_LeNomeEScoreNaOrdem)
        {
            auto r = RankingClient::parseScores(
                R"([{"name":"BOB","score":1500,"created_at":"x"},{"name":"ANA","score":500,"created_at":"y"}])");
            Assert::AreEqual(size_t(2), r.size());
            Assert::AreEqual(std::string("BOB"), r[0].playerName);
            Assert::AreEqual(1500, r[0].score);
            Assert::AreEqual(std::string("ANA"), r[1].playerName);
            Assert::AreEqual(500, r[1].score);
        }

        TEST_METHOD(ParseScores_NomeComAspasEscapadas)
        {
            auto r = RankingClient::parseScores(R"([{"name":"a\"b\\c","score":7}])");
            Assert::AreEqual(std::string("a\"b\\c"), r[0].playerName);
            Assert::AreEqual(7, r[0].score);
        }

        TEST_METHOD(ParseScores_SemScoreLancaNetworkException)
        {
            Assert::ExpectException<NetworkException>([]
                                                      { RankingClient::parseScores(R"([{"name":"X"}])"); });
        }
    };
}

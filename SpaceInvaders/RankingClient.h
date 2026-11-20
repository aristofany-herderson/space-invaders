#pragma once
#include <string>
#include <vector>

struct ScoreEntry
{
    std::string playerName;
    int score = 0;
};

// Cliente REST do ranking online (servidor em server/). Usa WinHTTP, nativo do Windows.
// Falha de rede ou resposta de erro -> NetworkException.
namespace RankingClient
{
    void submitScore(const ScoreEntry &entry);
    std::vector<ScoreEntry> fetchTop(int n = 10);

    // Separado da rede para poder ser testado: le o JSON devolvido por GET /scores.
    std::vector<ScoreEntry> parseScores(const std::string &json);
}

#pragma once
#include <string>

// Leitura/gravacao dos arquivos de dados locais (hiscore.dat e settings.dat).
// Nao depende de SFML, entao pode ser testada sem janela/audio.
//
// Convencao:
//  - arquivo inexistente NAO e erro (primeira execucao): devolve o valor padrao;
//  - arquivo ilegivel, vazio, nao numerico ou fora da faixa -> excecao
//    (PersistenceException / InvalidDataException), cabendo a quem chama decidir
//    o fallback.
namespace Storage
{
    constexpr float DEFAULT_VOLUME = 80.f;

    int readHighScore(const std::string &path);
    void writeHighScore(const std::string &path, int score);

    float readVolume(const std::string &path);
    void writeVolume(const std::string &path, float volume);
}

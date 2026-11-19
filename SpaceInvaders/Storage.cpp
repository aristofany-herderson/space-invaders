#include "Storage.h"
#include "Exceptions.h"
#include <filesystem>
#include <fstream>
#include <cmath>

namespace
{
    bool exists(const std::string &path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec) && !ec;
    }
}

int Storage::readHighScore(const std::string &path)
{
    if (!exists(path))
        return 0;

    std::ifstream f(path);
    if (!f.is_open())
        throw PersistenceException(path, "nao foi possivel abrir para leitura");

    long long value = 0;
    if (!(f >> value))
        throw InvalidDataException(path, "conteudo nao e um numero inteiro");
    if (value < 0 || value > 2147483647LL)
        throw InvalidDataException(path, "pontuacao fora da faixa valida");

    return static_cast<int>(value);
}

void Storage::writeHighScore(const std::string &path, int score)
{
    if (score < 0)
        throw std::invalid_argument("a pontuacao nao pode ser negativa");

    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open())
        throw PersistenceException(path, "nao foi possivel abrir para escrita");

    f << score;
    f.flush();
    if (!f)
        throw PersistenceException(path, "erro durante a escrita");
}

float Storage::readVolume(const std::string &path)
{
    if (!exists(path))
        return DEFAULT_VOLUME;

    std::ifstream f(path);
    if (!f.is_open())
        throw PersistenceException(path, "nao foi possivel abrir para leitura");

    float value = 0.f;
    if (!(f >> value) || !std::isfinite(value))
        throw InvalidDataException(path, "conteudo nao e um numero valido");
    if (value < 0.f || value > 100.f)
        throw InvalidDataException(path, "volume fora da faixa 0-100");

    return value;
}

void Storage::writeVolume(const std::string &path, float volume)
{
    if (!std::isfinite(volume) || volume < 0.f || volume > 100.f)
        throw std::invalid_argument("o volume deve estar entre 0 e 100");

    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open())
        throw PersistenceException(path, "nao foi possivel abrir para escrita");

    f << volume;
    f.flush();
    if (!f)
        throw PersistenceException(path, "erro durante a escrita");
}

#pragma once
#include <stdexcept>
#include <string>

// Hierarquia de excecoes do jogo.
//
//   std::runtime_error
//     └── GameException              (base: qualquer falha prevista do jogo)
//           ├── AssetLoadException   (fonte/sprite/som obrigatorio nao carregou)
//           └── PersistenceException (falha ao ler/gravar arquivo de dados)
//                 └── InvalidDataException (arquivo existe mas o conteudo e invalido)
//           └── NetworkException     (servidor de ranking offline ou com erro)
//
// Quem captura GameException trata qualquer falha "esperada"; erros de
// programacao continuam sendo std::logic_error / std::invalid_argument.

class GameException : public std::runtime_error
{
public:
    explicit GameException(const std::string &msg) : std::runtime_error(msg) {}
};

class AssetLoadException : public GameException
{
public:
    explicit AssetLoadException(const std::string &path)
        : GameException("Nao foi possivel carregar o recurso: " + path), m_path(path) {}

    const std::string &path() const noexcept { return m_path; }

private:
    std::string m_path;
};

class PersistenceException : public GameException
{
public:
    PersistenceException(const std::string &path, const std::string &reason)
        : GameException("Falha de persistencia em '" + path + "': " + reason), m_path(path) {}

    const std::string &path() const noexcept { return m_path; }

private:
    std::string m_path;
};

class InvalidDataException : public PersistenceException
{
public:
    InvalidDataException(const std::string &path, const std::string &reason)
        : PersistenceException(path, reason) {}
};

// Falha ao falar com o servidor de ranking (offline, timeout, resposta de erro).
class NetworkException : public GameException
{
public:
    explicit NetworkException(const std::string &msg) : GameException("Ranking: " + msg) {}
};

#include "Game.h"
#include "Exceptions.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    try
    {
        Game game;
        game.run();
    }
    catch (const AssetLoadException &e)
    {
        std::cerr << "[Erro] " << e.what()
                  << "\nVerifique se a pasta assets/ esta ao lado do executavel.\n";
        return 2;
    }
    catch (const GameException &e)
    {
        std::cerr << "[Erro] " << e.what() << "\n";
        return 3;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Erro inesperado] " << e.what() << "\n";
        return 1;
    }
    return 0;
}

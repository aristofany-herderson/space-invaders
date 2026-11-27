# Testes unitários

Os testes usam o **Microsoft C++ Unit Test Framework** (`CppUnitTest.h`), que já vem com o componente
*Desenvolvimento para desktop com C++* do Visual Studio. O projeto fica em [`SpaceInvaders.Tests/`](../SpaceInvaders.Tests)
e já está na solução (`SpaceInvaders.slnx`).

## Como configurar (uma vez)

1. Abra `SpaceInvaders.slnx` no Visual Studio 2026.
2. O projeto de testes procura o SFML na variável `SFML_DIR`. Se ela não existir, usa o mesmo caminho do projeto principal
   (`C:\Users\arist\source\SFML-3.1.0`). Para usar outro caminho, defina a variável de ambiente e reinicie o Visual Studio:

   ```powershell
   setx SFML_DIR "C:\caminho\para\SFML-3.1.0"
   ```

3. Na barra de menus, escolha **Teste → Gerenciador de Testes** (*Test Explorer*) e compile a solução na plataforma **x64**.
   Os testes aparecem agrupados por classe.
4. **Executar Tudo** (`Ctrl+R, A`).

O projeto de testes é uma DLL que compila junto os `.cpp` do jogo que **não dependem de janela** (`Player`, `Shield`, `Bullet`,
`Enemy`, `EnemyFactory`, `EnemyManager`, `Particle`, `Spritegen`, `SoundManager`, `Storage` e `RankingClient`). `Game.cpp` e a `main` ficam de fora.
Um evento pós-build copia as DLLs do SFML para a pasta de saída dos testes.

> **Ao criar um `.cpp` novo no jogo**, acrescente-o também em `SpaceInvaders.Tests.vcxproj` (e em `SpaceInvaders.vcxproj` e no
> `CMakeLists.txt`). Se faltar, o projeto de testes não linka ("símbolo externo não resolvido").

### Cuidados por ser uma DLL

O Test Explorer carrega e descarrega a DLL de testes dentro do próprio processo. Por isso:

- **Nada do SFML pode ser criado como variável global.** `sf::Texture` cria um contexto OpenGL no construtor; como global, isso roda
  enquanto a DLL é carregada e trava o Test Explorer antes do primeiro teste. Por isso o banco de texturas é obtido por `textureBank()`
  (a macro `GFX` continua funcionando) e só é criado no primeiro uso.
- **O áudio só é ligado em `SoundManager::loadAll()`.** Cada `sf::Sound` liga o dispositivo de áudio, que tem uma thread própria; se ele
  for ligado nos testes, o Test Explorer trava ou fecha quando a DLL é descarregada. Os testes nunca chamam `loadAll()`, então
  `SFX.play(...)` simplesmente não toca nada.

## O que é testado

| Arquivo | Classe testada | O que verifica |
|---|---|---|
| `ExceptionsTests.cpp` | hierarquia de exceções | herança (`InvalidDataException` → `PersistenceException` → `GameException`, `NetworkException` → `GameException`), caminho/mensagem guardados, falha de rede não é capturada como falha de arquivo |
| `StorageTests.cpp` | `Storage` | leitura/gravação de `hiscore.dat` e `settings.dat`, arquivo inexistente, conteúdo inválido, valores fora da faixa, diretório inexistente |
| `PlayerTests.cpp` | `Player` | estado inicial, pontuação, vidas, dano (`Damaged`/`Ignored`/`Dead`), invulnerabilidade, escudo, power-ups, uso pela interface `Damageable` |
| `ShieldTests.cpp` | `Shield`, `ShieldManager` | blocos com vida cheia, acerto, destruição em 3 tiros, `reset()`, `Damageable` (`takeDamage`, `isHitBy`, `isAlive`) |
| `BulletTests.cpp` | `BulletManager` | tiro simples/triplo/inimigo, movimento, remoção fora da tela e de balas mortas |
| `EnemyManagerTests.cpp` | `EnemyManager` | formação inicial, reinício por onda, formação desce até o chão; consistência das constantes de pontuação |
| `RankingTests.cpp` | `RankingClient` | leitura do JSON devolvido por `GET /scores` (lista vazia, ordem, aspas escapadas) e `NetworkException` para resposta inválida |
| `EnemyHierarchyTests.cpp` | `EnemyFactory`, `Enemy` e subclasses, `UFO` | quantidade e tipo por linha, subclasse certa por `EnemyType`, tipo inválido, pontuação/vida/cor por subclasse, dano e morte, polimorfismo via `Damageable`, tiros do `Drone` e do `Elite`, movimento do `UFO` |

### Tratamento de exceções nos testes

Os testes de `Storage` conferem o comportamento definido para cada falha:

| Situação | Resultado |
|---|---|
| arquivo não existe (primeira execução) | valor padrão (0 ponto / volume 80), **sem** exceção |
| arquivo vazio, não numérico ou fora da faixa | `InvalidDataException` |
| não consegue abrir/gravar o arquivo | `PersistenceException` |
| argumento inválido ao gravar (pontuação negativa, volume fora de 0–100) | `std::invalid_argument` |

O `Game` captura `GameException` ao carregar e usa o valor padrão (registrando a falha no `std::cerr`), então um arquivo corrompido não derruba o jogo.

No ranking, `RankingClient::parseScores` lança `NetworkException` quando a resposta do servidor não tem o formato esperado
(`RankingTests.cpp`). A parte de rede (`submitScore`/`fetchTop`) não é testada aqui, porque depende do servidor estar no ar.

## Pendências

- `submitScore`/`fetchTop` só são testados manualmente, com o servidor de `server/` rodando.
- Os testes de `Player`/`Shield`/`Bullet`/`Enemy`/`EnemyManager` usam as classes do SFML; se o ambiente de testes não tiver as DLLs do SFML ao lado do binário,
  o carregamento falha antes de qualquer teste rodar (o evento pós-build resolve isso na máquina de desenvolvimento).
- Em Linux/macOS o Microsoft C++ Unit Test Framework não existe: os testes rodam no Visual Studio (Windows). O jogo em si compila nas três plataformas com CMake.

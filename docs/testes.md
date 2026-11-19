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
`Enemy`, `Particle`, `Spritegen`, `SoundManager` e `Storage`). `Game.cpp` e a `main` ficam de fora. Um evento pós-build copia as DLLs do SFML
para a pasta de saída dos testes.

## O que é testado

| Arquivo | Classe testada | O que verifica |
|---|---|---|
| `ExceptionsTests.cpp` | hierarquia de exceções | herança (`InvalidDataException` → `PersistenceException` → `GameException`), caminho/mensagem guardados |
| `StorageTests.cpp` | `Storage` | leitura/gravação de `hiscore.dat` e `settings.dat`, arquivo inexistente, conteúdo inválido, valores fora da faixa, diretório inexistente |
| `PlayerTests.cpp` | `Player` | estado inicial, pontuação, vidas, dano (`Damaged`/`Ignored`/`Dead`), invulnerabilidade, escudo, power-ups |
| `ShieldTests.cpp` | `Shield`, `ShieldManager` | blocos com vida cheia, acerto, destruição em 3 tiros, `reset()` |
| `BulletTests.cpp` | `BulletManager` | tiro simples/triplo/inimigo, movimento, remoção fora da tela e de balas mortas |
| `EnemyManagerTests.cpp` | `EnemyManager` | formação inicial, reinício por onda, formação desce até o chão; consistência das constantes de pontuação |

### Tratamento de exceções nos testes

Os testes de `Storage` conferem o comportamento definido para cada falha:

| Situação | Resultado |
|---|---|
| arquivo não existe (primeira execução) | valor padrão (0 ponto / volume 80), **sem** exceção |
| arquivo vazio, não numérico ou fora da faixa | `InvalidDataException` |
| não consegue abrir/gravar o arquivo | `PersistenceException` |
| argumento inválido ao gravar (pontuação negativa, volume fora de 0–100) | `std::invalid_argument` |

O `Game` captura `GameException` ao carregar e usa o valor padrão (registrando a falha no `std::cerr`), então um arquivo corrompido não derruba o jogo.

## Pendências

- Quando a hierarquia `Enemy` / `EnemyFactory` for integrada, acrescentar `EnemyHierarchyTests.cpp`:
  pontuação, vida e movimento de cada subclasse, a Factory (quantidade e tipo de inimigos por linha) e o polimorfismo via `Damageable`.
- Quando o ranking existir, testar a (de)serialização JSON do `ScoreEntry` e o `RankingClient` com uma resposta de rede simulada.
- Os testes de `Player`/`Shield`/`Bullet`/`EnemyManager` usam as classes do SFML; se o ambiente de testes não tiver as DLLs do SFML ao lado do binário,
  o carregamento falha antes de qualquer teste rodar (o evento pós-build resolve isso na máquina de desenvolvimento).

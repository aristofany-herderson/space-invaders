<p id="title" align="center">
  <a href="#title">
    <img align="center" width="96" height="96" src="./SpaceInvaders/assets/icon.png">
    <h1 align="center">Space Invaders</h1>
  </a>
</p>

<p align="center">
  <a aria-label="Made By Aristofany" href="https://github.com/aristofany-herderson/">
    <img src="https://img.shields.io/badge/MADE%20BY%20Aristofany-000000.svg?style=for-the-badge&labelColor=000&logo=starship&logoColor=fff&logoWidth=20">
  </a>
  <a aria-label="License" href="./LICENSE">
    <img src="https://img.shields.io/github/license/aristofany-herderson/space-invaders?style=for-the-badge&color=000&labelColor=000">
  </a>
  <a aria-label="Enjoy My Repositories" href="https://github.com/aristofany-herderson?tab=repositories">
    <img src="https://img.shields.io/badge/Enjoy%20My%20Projects-000000.svg?style=for-the-badge&color=000&logo=github&labelColor=000&logoColor=fff">
  </a>
</p>

<p align="center">
  👾 Um clone moderno de Space Invaders feito em C++ e SFML, com ranking online.
</p>

<br/>

# 🎓 Sobre o projeto

Trabalho da disciplina **Linguagem de Programação II**. Partimos de um jogo jogável (power-ups, escudos, efeitos sonoros e recorde em arquivo)
e o reorganizamos para aplicar os conceitos da disciplina:

- **Classe abstrata e herança:** `Enemy` com as subclasses `Drone`, `Invader`, `Brute`, `Kamikaze`, `Elite` e `UFO`.
- **Interface e polimorfismo:** `Damageable`, implementada por `Player`, `Enemy` e `Shield`.
- **Padrões de projeto:** `EnemyFactory` (Factory) e `SoundManager` (Singleton), justificados na seção "Padrões de projeto".
- **Tratamento de exceções:** hierarquia `GameException` (veja a seção "Tratamento de exceções").
- **Extras:** GUI (SFML), banco de dados e API REST (libcurl) para o ranking online.
- **Testes unitários:** Microsoft C++ Unit Test Framework.

**Equipe:** Aristofany Herderson Gomes de Souto Menezes, Leonardo Rosendo Faustino e Mairon Lima de Melo.

| Integrante | Responsabilidade |
|---|---|
| Aristofany | base do jogo, hierarquia de inimigos, Factory, revisão dos pull requests |
| Mairon | banco de dados e API, tela de ranking, entrada do nome do jogador |
| Leonardo | diagramas UML, testes unitários, tratamento de exceções, README |

<br/>

# 🧪 Tecnologias

- C++20
- SFML 3.1 (gráficos, janela e áudio)
- CMake 3.21+ (Linux, Windows e macOS)
- Visual Studio 2026 (opcional, também usado para os testes)
- Microsoft C++ Unit Test Framework (testes)
- libcurl + API REST (Firebase/Supabase), previsto para o ranking

<br/>

# 📂 Estrutura do projeto

```
CMakeLists.txt                   build multiplataforma
SpaceInvaders.slnx               solução do Visual Studio
SpaceInvaders/                   código do jogo
    assets/                      fontes, sons e sprites
    Damageable.h                 interface de objetos que recebem dano
    Enemy, EnemyFactory,         hierarquia de inimigos, Factory e gerenciador da formação
    EnemyManager
    Background, Bullet, Game,    demais entidades e sistemas do jogo
    Particle, Player, Powerup,
    Shield, SoundManager,
    Spritegen, Random.h
    Exceptions.h                 hierarquia de exceções
    Storage                      leitura/gravação de hiscore.dat e settings.dat
SpaceInvaders.Tests/             projeto de testes unitários
docs/
    uml/                         diagramas de classes e de sequência
    testes.md                    como configurar e rodar os testes
```

<br/>

# 🧑🏻‍💻 Como executar

Clone o repositório:

```bash
git clone https://github.com/aristofany-herderson/space-invaders.git

cd space-invaders
```

## Linux, macOS e Windows (CMake)

Requisitos: compilador C++20 (GCC 11+, Clang 13+ ou MSVC 2022+), CMake 3.21+ e Git. Se o SFML 3 não estiver instalado, o CMake baixa e compila o SFML 3.1.0 sozinho.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

No Linux, o SFML compilado a partir do código-fonte precisa das bibliotecas de desenvolvimento do sistema. Em Debian/Ubuntu:

```bash
sudo apt install libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev libfreetype-dev libharfbuzz-dev libopenal-dev libvorbis-dev libflac-dev
```

Para usar um SFML já instalado, passe `-DCMAKE_PREFIX_PATH=/caminho/do/SFML`. A pasta `SpaceInvaders/assets/` é copiada para ao lado do executável após o build.

## Visual Studio (Windows)

1. Baixe o **SFML 3.1** (Visual C++, 64 bits) e extraia em uma pasta.
2. Ajuste os diretórios de include e lib do SFML nas propriedades do projeto `SpaceInvaders` (hoje apontam para `C:\Users\arist\source\SFML-3.1.0`).
3. Copie as DLLs do SFML (`sfml-*-d.dll` no Debug, `sfml-*.dll` no Release) para a pasta do executável.
4. Abra `SpaceInvaders.slnx` no **Visual Studio 2026**, escolha **x64** e compile/execute.

O executável precisa encontrar a pasta `assets/` no diretório de trabalho.

<br/>

# 🎮 O jogo

O jogador controla uma nave e deve sobreviver a ondas sucessivas de inimigos, buscando a maior pontuação.

| Ação | Tecla |
|---|---|
| Mover | `←` `→` ou `A` `D` |
| Atirar | `Espaço` ou `Z` |
| Pausar | `P`, `Esc` ou o botão de pausa no HUD |

<br/>

# 🚀 Funcionalidades

- ✅ Vários tipos de inimigos
- ✅ Explosões animadas e partículas
- ✅ Efeitos sonoros e música de fundo
- ✅ UFO bônus
- ✅ Escudos destrutíveis
- ✅ Power-ups (tiro rápido, tiro triplo, escudo)
- ✅ Recorde salvo em arquivo
- ✅ Dificuldade progressiva
- ✅ Botão de pausa e controle de volume
- 🚧 Ranking online com atualização automática (em desenvolvimento)

<br/>

# 🧠 Arquitetura

Os diagramas ficam em [`docs/uml/`](docs/uml) e são escritos em Mermaid, então o GitHub os desenha direto na página:

| Diagrama | Conteúdo |
|---|---|
| [Classes: versão atual](docs/uml/diagrama-classes-atual.md) | levantamento do código antes da refatoração |
| [Classes: arquitetura-alvo](docs/uml/diagrama-classes-alvo.md) | `Enemy` abstrata, subclasses, `Damageable`, `EnemyFactory`, ranking, exceções |
| [Sequência](docs/uml/diagramas-sequencia.md) | quadro de jogo, colisão polimórfica, montagem de onda e envio de pontuação ao ranking |

Módulos principais:

- **Game**: laço principal, estados e colisões
- **Player**: movimento, tiro, vidas e power-ups
- **Damageable**: interface implementada por `Player`, `Enemy` e `Shield`
- **Enemy**: classe abstrata com `Drone`, `Invader`, `Brute`, `Kamikaze`, `Elite` e `UFO`
- **EnemyFactory**: monta a formação de cada onda e cria o UFO
- **EnemyManager**: movimento da formação, tiros do jogador contra inimigos e power-ups
- **Bullet**: projéteis
- **Shield**: escudos destrutíveis
- **Particle / Background**: efeitos visuais
- **SoundManager**: áudio (Singleton)
- **Spritegen**: carregamento e geração de sprites
- **Storage**: persistência local do recorde e do volume

<br/>

# 🧩 Padrões de projeto

| Padrão | Onde | Por que |
|---|---|---|
| Factory | `EnemyFactory` | a formação de cada onda é uma tabela de tipos por linha. A Factory concentra esse conhecimento e o `EnemyManager` só recebe `unique_ptr<Enemy>`, sem `switch` por tipo. Um inimigo novo exige uma subclasse e uma linha na Factory. |
| Singleton | `SoundManager::instance()` | existe um único dispositivo de áudio e um pool de vozes compartilhado. O construtor é privado e a cópia é proibida, então não há como criar uma segunda instância. |
| Template Method | `Enemy::update` | a base define a ordem (movimento, depois tiro) e cada subclasse preenche `move`, `attack` e `canAttack`. |

Princípios SOLID: `Game` depende da abstração `Damageable`, não de `Player`, `Shield` ou de cada inimigo (Dependency Inversion e Liskov); adicionar um tipo de inimigo não altera `EnemyManager` nem `Game` (Open/Closed); cada classe tem uma responsabilidade (Single Responsibility).

<br/>

# 🛡️ Tratamento de exceções

As falhas previstas usam a hierarquia definida em [`Exceptions.h`](SpaceInvaders/Exceptions.h):

```
std::runtime_error
└── GameException
    ├── AssetLoadException        recurso obrigatório não carregou (ex.: fonte)
    └── PersistenceException      falha ao ler/gravar arquivo
        └── InvalidDataException  arquivo existe, mas o conteúdo é inválido
```

| Situação | Comportamento |
|---|---|
| `hiscore.dat` ou `settings.dat` não existe | usa o valor padrão (primeira execução não é erro) |
| arquivo corrompido, vazio ou fora da faixa | `InvalidDataException`; o jogo registra no `std::cerr` e usa o valor padrão |
| não é possível gravar o recorde/volume | `PersistenceException`; o jogo registra o erro e continua |
| fonte `assets/fonts/font.ttf` ausente | `AssetLoadException`; `main` mostra a mensagem e encerra com código 2 |
| argumento inválido (pontuação negativa, volume fora de 0–100) | `std::invalid_argument` |

Sprites e sons ausentes não derrubam o jogo: os sprites têm *fallback* procedural e os sons são ignorados.

<br/>

# ✅ Testes

Projeto [`SpaceInvaders.Tests`](SpaceInvaders.Tests) com o Microsoft C++ Unit Test Framework.
Para rodar: abra a solução, compile em **x64** e use **Teste → Gerenciador de Testes → Executar Tudo**.
Passo a passo e lista do que é coberto em [`docs/testes.md`](docs/testes.md).

<br/>

# 📈 Próximos passos

- Ranking online (API REST + banco), tela de ranking e entrada do nome
- Modos de dificuldade, controle e conquistas

<br/>

# 🧑🏻 Autor do jogo original

<p align="center">
  <img width="20%" src="https://github.com/aristofany-herderson.png">

  <p align="center">
    Aristofany Herderson
  </p>

  <p align="center">
    <a href="https://www.linkedin.com/in/aristofany-herderson/" target="_blank">
    <img src="https://img.shields.io/badge/LINKEDIN-000000.svg?style=for-the-badge&labelColor=0a66c2&logo=inspire&logoColor=fff">
    </a>
    <a href="https://www.instagram.com/aristo.dev/" target="_blank">
    <img src="https://img.shields.io/badge/INSTAGRAM-000000.svg?style=for-the-badge&labelColor=dd326f&logo=instagram&logoColor=fff">
    </a>
  </p>
</p>

<br/>

# 📜 Licença

Este projeto é distribuído sob a licença MIT.

Fique à vontade para fazer fork, estudar e melhorar.

<br/>

# 👾 Arcade Never Dies

_"The only thing standing between humanity and the invaders is you."_

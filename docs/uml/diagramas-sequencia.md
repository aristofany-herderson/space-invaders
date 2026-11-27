# Diagramas de sequência

Quatro cenários, todos descrevendo o **código atual**: quadro de jogo, colisão polimórfica, montagem de onda e ranking online.

## 1. Um quadro de jogo

`Game::run` → `update(dt)`, como está hoje em `Game.cpp`.

```mermaid
sequenceDiagram
    autonumber
    participant Run as Game::run
    participant G as Game
    participant P as Player
    participant EM as EnemyManager
    participant BM as BulletManager
    participant SM as ShieldManager
    participant FX as ParticleSystem

    loop enquanto a janela está aberta
        Run->>G: processEvents()
        Run->>G: update(dt)
        G->>P: update(dt, bullets, fx)
        P->>BM: spawnPlayer / spawnTriple (se atirou)
        G->>EM: update(dt, playerPos, bullets, fx, onKill)
        EM->>BM: spawnEnemy (inimigos atiram)
        EM->>FX: spawnHit / spawnExplosion (tiro acertou)
        EM-->>G: onKill(pos, cor, pontos)
        G->>P: addScore(pontos)
        G->>BM: update(dt)
        G->>SM: collide(bullet, shield) (tiros do jogador e dos inimigos)
        G->>P: collide(bullet, player) (tiro inimigo no jogador)
        P-->>G: DamageResult
        alt Dead
            G->>G: saveHighScore() / estado = GameOver
        end
        alt EnemyManager.allDead()
            G->>G: nextWave() ou estado = Win
        end
        Run->>G: render()
    end
```

## 2. Tiro contra um alvo (`Damageable`)

Mostra o polimorfismo: `Game` não precisa saber se o alvo é `Drone`, `Brute`, `Shield`…

```mermaid
sequenceDiagram
    autonumber
    participant G as Game
    participant BM as BulletManager
    participant T as Damageable (Enemy, Shield ou Player)
    participant FX as ParticleSystem
    participant SFX as SoundManager

    G->>BM: getAll()
    BM-->>G: bullets
    loop para cada Bullet viva
        loop para cada Damageable vivo do outro lado
            G->>T: isHitBy(bullet.pos, raio)
            T-->>G: bool
            alt tiro atingiu o alvo
                G->>T: takeDamage(1, bullet.pos)
                T-->>G: DamageResult
                G->>G: bullet.alive = false
                alt Damaged
                    G->>FX: spawnHit(pos, cor)
                    G->>SFX: play(Hit)
                else Dead
                    G->>FX: spawnExplosion(pos, cor)
                    G->>SFX: play(Explosion)
                    G->>G: soma scoreValue() ao jogador
                else Ignored (invulnerável / escudo ativo)
                    Note over G,T: nenhuma reação extra
                end
            end
        end
    end
```

## 3. Montagem de uma onda (`EnemyFactory`)

```mermaid
sequenceDiagram
    autonumber
    participant G as Game
    participant EM as EnemyManager
    participant F as EnemyFactory
    participant E as Enemy (Drone, Invader, Brute...)

    G->>EM: spawnFormation(wave)
    EM->>EM: limpa inimigos e power-ups anteriores
    EM->>F: createFormation(wave)
    loop para cada linha e coluna da formação
        F->>F: escolhe o EnemyType da linha
        F->>E: new Tipo(pos, wave)
        E-->>F: unique_ptr~Enemy~
    end
    F-->>EM: vector de unique_ptr~Enemy~
    EM-->>G: formação pronta
    Note over G,EM: Em cada update, EM chama enemy->update(), que usa move() e attack() da subclasse
```

## 4. Fim de partida e ranking online

Fluxo de `Game.cpp` (estados `GameOver` → `NameEntry` → `Ranking`), `RankingClient.cpp` e `server/server.js`.
A rede roda em outra thread (`std::async`), então a janela continua desenhando enquanto espera; o `Game` só olha o resultado
em `pollRanking()`, chamado a cada quadro.

```mermaid
sequenceDiagram
    autonumber
    actor J as Jogador
    participant G as Game
    participant ST as Storage
    participant T as Thread do ranking
    participant RC as RankingClient
    participant API as server.js
    participant DB as PostgreSQL

    G->>G: jogador sem vidas, estado = GameOver
    G->>ST: writeHighScore("hiscore.dat", score) se for recorde
    alt falha ao gravar
        ST--xG: PersistenceException
        G->>G: registra no std::cerr e continua
    end
    J->>G: Enter
    G->>G: estado = NameEntry (se fez pontos)
    J->>G: digita o nome (até 16 letras) e Enter
    G->>G: openRanking(ScoreEntry), estado = Ranking, "SENDING..."
    G-)T: std::async(submitScore + fetchTop(10))
    T->>RC: submitScore(entry)
    RC->>API: POST /scores {"name", "score"}
    API->>DB: INSERT INTO scores
    API-->>RC: 201 Created
    T->>RC: fetchTop(10)
    RC->>API: GET /scores?limit=10
    API->>DB: SELECT ... ORDER BY score DESC
    API-->>RC: JSON
    RC->>RC: parseScores(json)
    RC-->>T: vector de ScoreEntry
    loop a cada quadro
        G->>T: future pronto?
    end
    alt sucesso
        T-->>G: future.get() devolve a lista
        G->>G: desenha o top 10 destacando o jogador
    else servidor fora do ar, timeout de 3 s, HTTP de erro ou JSON inválido
        T--xG: future.get() relança NetworkException
        G->>G: registra no std::cerr e mostra "RANKING OFFLINE"
    end
    Note over API: após cada POST, o servidor avisa a página web do placar (Server-Sent Events em /scores/stream)
```

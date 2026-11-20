# Diagramas de sequência

Quatro cenários: os três primeiros descrevem o **código atual** (quadro de jogo, colisão polimórfica e montagem de onda) e o último descreve o **comportamento-alvo** do ranking online.

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

## 4. Fim de partida e ranking online (alvo, ainda não implementado)

Fluxo previsto para o diferencial do projeto, com o tratamento de falhas de rede.
O envio deve acontecer em outra thread, para não travar o jogo.

```mermaid
sequenceDiagram
    autonumber
    actor J as Jogador
    participant G as Game
    participant ST as Storage
    participant N as NameInputScreen
    participant RC as RankingClient
    participant API as API REST (Firebase ou Supabase)
    participant RS as RankingScreen

    G->>G: Player.isAlive() == false, estado = GameOver
    G->>ST: writeHighScore("hiscore.dat", score)
    alt falha ao gravar
        ST--xG: PersistenceException
        G->>G: registra no log e continua
    end
    G->>N: abre a tela de nome
    J->>N: digita o nome e confirma
    N-->>G: ScoreEntry(nome, pontos, onda)
    G->>RC: submitScore(entry) em thread separada
    RC->>API: POST /scores (JSON)
    alt sucesso (HTTP 2xx)
        API-->>RC: 201 Created
        RC-->>G: ok
    else sem rede, timeout ou HTTP 4xx/5xx
        API--xRC: erro
        RC--xG: NetworkException
        G->>G: avisa "ranking indisponível" e segue sem travar
    end
    J->>G: escolhe "Ranking"
    G->>RS: abre a tela
    loop a cada N segundos enquanto a tela estiver aberta
        RS->>RC: fetchTop(10)
        RC->>API: GET /scores?order=score.desc&limit=10
        API-->>RC: JSON
        RC-->>RS: vector de ScoreEntry
        RS->>RS: atualiza a lista na tela
    end
```

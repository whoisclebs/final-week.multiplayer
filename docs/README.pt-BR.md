<a id="readme-top"></a>

<p align="center">
  <a href="../README.md">English</a> · <b>Português</b> · <a href="README.es.md">Español</a>
</p>

[![Build][build-shield]][build-url]
[![Release][release-shield]][release-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<br />
<div align="center">
  <h3 align="center">Final Week Multiplayer</h3>

  <p align="center">
    Aumente o limite de jogadores das salas cooperativas de Finals Week. Um clique para instalar, um clique para remover.
    <br />
    <a href="https://github.com/whoisclebs/final-week.multiplayer/releases/latest"><strong>Baixar a versão mais recente »</strong></a>
    <br />
    <br />
    <a href="https://github.com/whoisclebs/final-week.multiplayer/issues/new?labels=bug">Reportar bug</a>
    &middot;
    <a href="https://github.com/whoisclebs/final-week.multiplayer/issues/new?labels=enhancement">Sugerir recurso</a>
  </p>
</div>

<details>
  <summary>Sumário</summary>
  <ol>
    <li>
      <a href="#sobre-o-projeto">Sobre o projeto</a>
      <ul>
        <li><a href="#como-funciona">Como funciona</a></li>
        <li><a href="#feito-com">Feito com</a></li>
      </ul>
    </li>
    <li>
      <a href="#primeiros-passos">Primeiros passos</a>
      <ul>
        <li><a href="#pré-requisitos">Pré-requisitos</a></li>
        <li><a href="#instalação">Instalação</a></li>
      </ul>
    </li>
    <li><a href="#uso">Uso</a></li>
    <li><a href="#desenvolvimento">Desenvolvimento</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contribuindo">Contribuindo</a></li>
    <li><a href="#licença">Licença</a></li>
    <li><a href="#contato">Contato</a></li>
    <li><a href="#agradecimentos">Agradecimentos</a></li>
  </ol>
</details>

## Sobre o projeto

Finals Week limita as salas multiplayer a **4 jogadores**. Esse limite é uma única constante passada para `CreateAdvancedSession` no blueprint `BP_GameInstance` do jogo. Este projeto altera essa constante e nada mais.

Ele entrega três coisas:

* **`fwm-installer`**: uma pequena ferramenta de linha de comando que gera o mod a partir dos *seus próprios* arquivos do jogo e o instala. Continua funcionando após atualizações do jogo enquanto essa chamada de sessão mantiver o mesmo formato.
* **Um `_P.pak` pronto** com limite de 8 jogadores, anexado a cada release, para quem prefere copiar um arquivo manualmente.
* **Um plugin C++ para Unreal Engine 5** ([`plugin/`](../plugin/FinalWeekMultiplayer/README.md)) com um subsistema de sessões reutilizável em projetos UE. O jogo publicado não carrega plugins C++, então ele é um recurso para desenvolvedores, não faz parte do mod.

Os arquivos do jogo nunca são modificados. A Unreal Engine carrega arquivos `_P.pak` por cima do pak do jogo, então apagar o arquivo do mod restaura o jogo original.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

### Como funciona

1. O instalador encontra o jogo nas suas bibliotecas da Steam.
2. Ele lê o `BP_GameInstance` de `CopyGirls-Windows.pak`. Esse pak é comprimido com Oodle; o instalador o descomprime com o [oozextract](https://github.com/lvlvllvlvllvlvl/oozextract), um descompressor open source, então ele nunca se conecta à internet.
3. Ele procura no bytecode a chamada `CreateAdvancedSession(PublicConnections, PrivateConnections = 0, …)` e troca `PublicConnections`. A chamada precisa ser encontrada **exatamente uma vez**; caso contrário ele para sem gravar nada, então uma versão desconhecida do jogo nunca é alterada às cegas.
4. Ele grava `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` em `CopyGirls/Content/Paks`. O arquivo é escrito primeiro em um caminho temporário e só é renomeado quando está completo.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

### Feito com

* [![Rust][rust-shield]][rust-url]
* [![Unreal Engine][unreal-shield]][unreal-url]
* [repak](https://github.com/trumank/repak) para ler e gravar paks da Unreal
* [oozextract](https://github.com/lvlvllvlvllvlvl/oozextract) para descompressão Oodle
* [clap](https://github.com/clap-rs/clap) para a linha de comando

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Primeiros passos

### Pré-requisitos

* Finals Week instalado pela Steam, no Windows ou no Linux / Steam Deck (Proton).
* Só o **host** (quem cria a sala) precisa do mod. É recomendado que todos tenham.

### Instalação

#### Opção A: instalador (recomendado)

1. Baixe `fwm-installer.exe` (Windows) ou `fwm-installer-linux` da [release mais recente][release-url].
2. Feche o jogo.
3. Dê dois cliques em `fwm-installer.exe`. Ele instala o mod com limite de 8 jogadores. Se a Steam estiver numa pasta personalizada, coloque o instalador na pasta do jogo (a que tem o `CopyGirls.exe`) antes.

Para outro limite, execute pelo terminal:

```sh
fwm-installer install --players 12
```

#### Opção B: pak pronto

1. Na Steam, clique com o botão direito em **Finals Week** → **Gerenciar** → **Ver arquivos locais**.
2. Abra `CopyGirls\Content\Paks`.
3. Copie `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` da release para essa pasta. Não substitua nem apague nada.
4. Confira se o arquivo não foi adulterado comparando o hash com o `SHA256SUMS.txt` da release:

   ```powershell
   Get-FileHash .\CopyGirls-Windows_FinalWeekMultiplayer_P.pak
   ```

> [!IMPORTANT]
> O pak pronto contém o arquivo do jogo da versão em que foi gerado. Depois de uma atualização do jogo, use o instalador ou aguarde uma nova release.

#### Desinstalar

Execute `fwm-installer uninstall` ou apague `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` da pasta `Paks`.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Uso

```text
fwm-installer [COMANDO]
```

| Comando | Descrição |
|---|---|
| *(nenhum)* | Instala com as configurações padrão e espera Enter (dois cliques) |
| `install [--players N] [--game-dir DIR]` | Instala o mod; execute de novo para mudar o limite |
| `uninstall [--game-dir DIR]` | Remove o mod |
| `pack [--from PAK] [--output ARQUIVO] [--players N]` | Gera o pak do mod sem instalar |

* `--players` aceita de **2 a 64** (padrão **8**).
* `--game-dir` é a pasta que contém `CopyGirls.exe`. Sem ele, o instalador procura primeiro na própria pasta e depois nas bibliotecas da Steam.

### Limitações conhecidas

* Testado pela comunidade com até 4 jogadores. Com mais, as salas de prova podem não ter cadeiras para todos, e nomes em quadros e listas podem ficar cortados.
* Outro mod `_P.pak` que também substitui o `BP_GameInstance` (por exemplo, um mod antigo de "mais jogadores") entra em conflito com este. O instalador avisa quando encontra outros arquivos `_P.pak`.
* Se uma atualização do jogo mudar a chamada de sessão, o instalador se recusa a instalar. Remova o mod e aguarde uma atualização.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Desenvolvimento

```text
.
├── .github/workflows/   CI: lint, testes, build cruzado e release (runners Linux)
├── assets/              BP_GameInstance original da versão suportada do jogo
├── docs/                READMEs traduzidos
├── installer/           Instalador e empacotador em Rust
│   ├── oodle-shim/      Descompressor Oodle offline usado pelo repak
│   └── src/
│       ├── main.rs      Linha de comando
│       ├── game.rs      Localização da Steam e da pasta do jogo
│       ├── pak.rs       Leitura e gravação de pak
│       └── patch.rs     Alteração do bytecode
└── plugin/              Plugin C++ para Unreal Engine 5
```

Compilar e testar o instalador:

```sh
cd installer
cargo test
cargo build --release
```

Cada push e pull request executa o [workflow Build][build-url] no Ubuntu: `cargo fmt`, `clippy`, testes, builds para Linux e Windows (compilação cruzada com MinGW) e o pak de 8 jogadores gerado a partir de `assets/`. Enviar uma tag `v*` publica uma release no GitHub com os instaladores, o pak, o código do plugin e o `SHA256SUMS.txt`.

Depois de uma atualização do jogo, atualize o asset versionado a partir do jogo instalado. Use `--players 4`, o valor original do jogo, para que o asset fique sem alteração:

```sh
fwm-installer pack --players 4 --output assets/BP_GameInstance.pak
```

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Roadmap

- [x] Instalador com limite de jogadores configurável
- [x] Pak pronto gerado pelo CI
- [x] Plugin de sessões para Unreal Engine 5
- [x] Instalador offline, sem downloads
- [ ] Confirmar cadeiras e interface com mais de 4 jogadores

Veja as [issues abertas][issues-url] para a lista completa de recursos propostos e problemas conhecidos.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Contribuindo

Contribuições são bem-vindas.

1. Faça um fork do projeto
2. Crie sua branch (`git checkout -b feat/novo-recurso`)
3. Faça commits seguindo [Conventional Commits](https://www.conventionalcommits.org/pt-br/) (`git commit -m 'feat: add new feature'`)
4. Envie a branch (`git push origin feat/novo-recurso`)
5. Abra um pull request

Código, comentários e mensagens de commit são escritos em inglês.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Licença

Distribuído sob a licença MIT. Veja [`LICENSE`](../LICENSE) para mais informações.

Este é um projeto não oficial de fãs, sem vínculo nem suporte dos desenvolvedores do jogo; não reporte bugs a eles enquanto estiver usando o mod. `assets/` contém um arquivo original do jogo (build Steam 26650291), mantido apenas para que o CI possa gerar o pak; todos os direitos pertencem aos seus donos.

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Contato

whoisclebs: [github.com/whoisclebs](https://github.com/whoisclebs)

Link do projeto: [github.com/whoisclebs/final-week.multiplayer](https://github.com/whoisclebs/final-week.multiplayer)

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

## Agradecimentos

* [rafaalmeida1/Final-Week-Game---More-Players](https://github.com/rafaalmeida1/Final-Week-Game---More-Players), o mod original que encontrou o limite de jogadores no `BP_GameInstance`
* [trumank/repak](https://github.com/trumank/repak)
* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)
* [Shields.io](https://shields.io)

<p align="right">(<a href="#readme-top">voltar ao topo</a>)</p>

[build-shield]: https://img.shields.io/github/actions/workflow/status/whoisclebs/final-week.multiplayer/build.yml?branch=main&style=for-the-badge
[build-url]: https://github.com/whoisclebs/final-week.multiplayer/actions/workflows/build.yml
[release-shield]: https://img.shields.io/github/v/release/whoisclebs/final-week.multiplayer?style=for-the-badge
[release-url]: https://github.com/whoisclebs/final-week.multiplayer/releases/latest
[issues-shield]: https://img.shields.io/github/issues/whoisclebs/final-week.multiplayer.svg?style=for-the-badge
[issues-url]: https://github.com/whoisclebs/final-week.multiplayer/issues
[license-shield]: https://img.shields.io/github/license/whoisclebs/final-week.multiplayer.svg?style=for-the-badge
[license-url]: https://github.com/whoisclebs/final-week.multiplayer/blob/main/LICENSE
[rust-shield]: https://img.shields.io/badge/Rust-000000?style=for-the-badge&logo=rust&logoColor=white
[rust-url]: https://www.rust-lang.org/
[unreal-shield]: https://img.shields.io/badge/Unreal%20Engine%205-0E1128?style=for-the-badge&logo=unrealengine&logoColor=white
[unreal-url]: https://www.unrealengine.com/

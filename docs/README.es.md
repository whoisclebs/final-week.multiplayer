<a id="readme-top"></a>

<p align="center">
  <a href="../README.md">English</a> · <a href="README.pt-BR.md">Português</a> · <b>Español</b>
</p>

[![Build][build-shield]][build-url]
[![Release][release-shield]][release-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<br />
<div align="center">
  <h3 align="center">Final Week Multiplayer</h3>

  <p align="center">
    Aumenta el límite de jugadores de las salas cooperativas de Finals Week. Un clic para instalar, un clic para quitar.
    <br />
    <a href="https://github.com/whoisclebs/final-week.multiplayer/releases/latest"><strong>Descargar la última versión »</strong></a>
    <br />
    <br />
    <a href="https://github.com/whoisclebs/final-week.multiplayer/issues/new?labels=bug">Reportar un error</a>
    &middot;
    <a href="https://github.com/whoisclebs/final-week.multiplayer/issues/new?labels=enhancement">Solicitar una función</a>
  </p>
</div>

<details>
  <summary>Índice</summary>
  <ol>
    <li>
      <a href="#acerca-del-proyecto">Acerca del proyecto</a>
      <ul>
        <li><a href="#cómo-funciona">Cómo funciona</a></li>
        <li><a href="#construido-con">Construido con</a></li>
      </ul>
    </li>
    <li>
      <a href="#primeros-pasos">Primeros pasos</a>
      <ul>
        <li><a href="#requisitos-previos">Requisitos previos</a></li>
        <li><a href="#instalación">Instalación</a></li>
      </ul>
    </li>
    <li><a href="#uso">Uso</a></li>
    <li><a href="#desarrollo">Desarrollo</a></li>
    <li><a href="#hoja-de-ruta">Hoja de ruta</a></li>
    <li><a href="#contribuir">Contribuir</a></li>
    <li><a href="#licencia">Licencia</a></li>
    <li><a href="#contacto">Contacto</a></li>
    <li><a href="#agradecimientos">Agradecimientos</a></li>
  </ol>
</details>

## Acerca del proyecto

Finals Week limita las salas multijugador a **4 jugadores**. Ese límite es una única constante que se pasa a `CreateAdvancedSession` en el blueprint `BP_GameInstance` del juego. Este proyecto cambia esa constante y nada más.

Incluye tres cosas:

* **`fwm-installer`**: una pequeña herramienta de línea de comandos que genera el mod a partir de *tus propios* archivos del juego y lo instala. Sigue funcionando tras las actualizaciones del juego mientras esa llamada de sesión conserve su forma.
* **Un `_P.pak` listo** con límite de 8 jugadores, adjunto a cada release, para quien prefiera copiar un archivo a mano.
* **Un plugin C++ para Unreal Engine 5** ([`plugin/`](../plugin/FinalWeekMultiplayer/README.md)) con un subsistema de sesiones reutilizable en proyectos UE. El juego publicado no puede cargar plugins C++, así que es un recurso para desarrolladores, no forma parte del mod.

Los archivos del juego nunca se modifican. Unreal Engine carga los archivos `_P.pak` encima del pak del juego, así que borrar el archivo del mod restaura el juego original.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

### Cómo funciona

1. El instalador encuentra el juego en tus bibliotecas de Steam.
2. Lee `BP_GameInstance` de `CopyGirls-Windows.pak`. Ese pak está comprimido con Oodle; el instalador lo descomprime con [oozextract](https://github.com/lvlvllvlvllvlvl/oozextract), un descompresor de código abierto, así que nunca se conecta a internet.
3. Busca en el bytecode la llamada `CreateAdvancedSession(PublicConnections, PrivateConnections = 0, …)` y reemplaza `PublicConnections`. La llamada debe encontrarse **exactamente una vez**; si no, se detiene sin escribir nada, así que una versión desconocida del juego nunca se modifica a ciegas.
4. Escribe `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` en `CopyGirls/Content/Paks`. El archivo se escribe primero en una ruta temporal y solo se renombra cuando está completo.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

### Construido con

* [![Rust][rust-shield]][rust-url]
* [![Unreal Engine][unreal-shield]][unreal-url]
* [repak](https://github.com/trumank/repak) para leer y escribir paks de Unreal
* [oozextract](https://github.com/lvlvllvlvllvlvl/oozextract) para la descompresión Oodle
* [clap](https://github.com/clap-rs/clap) para la línea de comandos

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Primeros pasos

### Requisitos previos

* Finals Week instalado desde Steam, en Windows o en Linux / Steam Deck (Proton).
* Solo el **anfitrión** (quien crea la sala) necesita el mod. Se recomienda que todos lo tengan.

### Instalación

#### Opción A: instalador (recomendado)

1. Descarga `fwm-installer.exe` (Windows) o `fwm-installer-linux` de la [última release][release-url].
2. Cierra el juego.
3. Haz doble clic en `fwm-installer.exe`. Instala el mod con un límite de 8 jugadores. Si Steam está en una carpeta personalizada, coloca antes el instalador en la carpeta del juego (la que tiene `CopyGirls.exe`).

Para otro límite, ejecútalo desde una terminal:

```sh
fwm-installer install --players 12
```

#### Opción B: pak listo

1. En Steam, haz clic derecho en **Finals Week** → **Administrar** → **Ver archivos locales**.
2. Abre `CopyGirls\Content\Paks`.
3. Copia `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` de la release en esa carpeta. No reemplaces ni borres nada.
4. Comprueba que el archivo no fue alterado comparando su hash con `SHA256SUMS.txt` de la release:

   ```powershell
   Get-FileHash .\CopyGirls-Windows_FinalWeekMultiplayer_P.pak
   ```

> [!IMPORTANT]
> El pak listo contiene el archivo del juego de la versión con la que se generó. Tras una actualización del juego, usa el instalador o espera una nueva release.

#### Desinstalar

Ejecuta `fwm-installer uninstall` o borra `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` de la carpeta `Paks`.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Uso

```text
fwm-installer [COMANDO]
```

| Comando | Descripción |
|---|---|
| *(ninguno)* | Instala con la configuración por defecto y espera Enter (doble clic) |
| `install [--players N] [--game-dir DIR]` | Instala el mod; ejecútalo de nuevo para cambiar el límite |
| `uninstall [--game-dir DIR]` | Quita el mod |
| `pack [--from PAK] [--output ARCHIVO] [--players N]` | Genera el pak del mod sin instalarlo |

* `--players` acepta de **2 a 64** (por defecto **8**).
* `--game-dir` es la carpeta que contiene `CopyGirls.exe`. Sin él, el instalador busca primero en su propia carpeta y luego en las bibliotecas de Steam.

### Limitaciones conocidas

* Probado por la comunidad con hasta 4 jugadores. Con más, puede que las salas de examen no tengan sillas para todos y que los nombres en pizarras y listas aparezcan cortados.
* Otro mod `_P.pak` que también reemplace `BP_GameInstance` (por ejemplo, un mod antiguo de "más jugadores") entra en conflicto con este. El instalador avisa cuando encuentra otros archivos `_P.pak`.
* Si una actualización del juego cambia la llamada de sesión, el instalador se niega a instalar. Quita el mod y espera una actualización.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Desarrollo

```text
.
├── .github/workflows/   CI: lint, pruebas, compilación cruzada y release (runners Linux)
├── assets/              BP_GameInstance original de la versión soportada del juego
├── docs/                READMEs traducidos
├── installer/           Instalador y empaquetador en Rust
│   ├── oodle-shim/      Descompresor Oodle sin conexión usado por repak
│   └── src/
│       ├── main.rs      Línea de comandos
│       ├── game.rs      Búsqueda de Steam y de la carpeta del juego
│       ├── pak.rs       Lectura y escritura de paks
│       └── patch.rs     Modificación del bytecode
└── plugin/              Plugin C++ para Unreal Engine 5
```

Compilar y probar el instalador:

```sh
cd installer
cargo test
cargo build --release
```

Cada push y pull request ejecuta el [workflow Build][build-url] en Ubuntu: `cargo fmt`, `clippy`, pruebas, compilaciones para Linux y Windows (compilación cruzada con MinGW) y el pak de 8 jugadores generado desde `assets/`. Subir una etiqueta `v*` publica una release en GitHub con los instaladores, el pak, el código del plugin y `SHA256SUMS.txt`.

Tras una actualización del juego, actualiza el asset versionado desde el juego instalado. Usa `--players 4`, el valor original del juego, para que el asset quede sin cambios:

```sh
fwm-installer pack --players 4 --output assets/BP_GameInstance.pak
```

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Hoja de ruta

- [x] Instalador con límite de jugadores configurable
- [x] Pak listo generado por el CI
- [x] Plugin de sesiones para Unreal Engine 5
- [x] Instalador sin conexión, sin descargas
- [ ] Confirmar sillas e interfaz con más de 4 jugadores

Consulta los [issues abiertos][issues-url] para ver la lista completa de funciones propuestas y problemas conocidos.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Contribuir

Las contribuciones son bienvenidas.

1. Haz un fork del proyecto
2. Crea tu rama (`git checkout -b feat/nueva-funcion`)
3. Haz commits siguiendo [Conventional Commits](https://www.conventionalcommits.org/es/) (`git commit -m 'feat: add new feature'`)
4. Sube la rama (`git push origin feat/nueva-funcion`)
5. Abre un pull request

El código, los comentarios y los mensajes de commit se escriben en inglés.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Licencia

Distribuido bajo la licencia MIT. Consulta [`LICENSE`](../LICENSE) para más información.

Este es un proyecto no oficial de fans, sin relación ni soporte de los desarrolladores del juego; no les reportes errores mientras uses el mod. `assets/` contiene un archivo original del juego (build de Steam 26650291), guardado solo para que el CI pueda generar el pak; todos los derechos pertenecen a sus dueños.

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Contacto

whoisclebs: [github.com/whoisclebs](https://github.com/whoisclebs)

Enlace del proyecto: [github.com/whoisclebs/final-week.multiplayer](https://github.com/whoisclebs/final-week.multiplayer)

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

## Agradecimientos

* [rafaalmeida1/Final-Week-Game---More-Players](https://github.com/rafaalmeida1/Final-Week-Game---More-Players), el mod original que encontró el límite de jugadores en `BP_GameInstance`
* [trumank/repak](https://github.com/trumank/repak)
* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)
* [Shields.io](https://shields.io)

<p align="right">(<a href="#readme-top">volver arriba</a>)</p>

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

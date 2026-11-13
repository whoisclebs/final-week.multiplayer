//! Final Week Multiplayer installer: raises the Finals Week player limit.
//!
//! The mod is a `_P.pak` holding a copy of the game's `BP_GameInstance` with a
//! different `PublicConnections` value. The game files are never modified.

mod game;
mod pak;
mod patch;

use std::path::{Path, PathBuf};
use std::process::ExitCode;

use anyhow::{Result, ensure};
use clap::{Args, Parser, Subcommand};

use game::{Game, MOD_PAK};
use pak::GameInstance;

const DEFAULT_PLAYERS: u8 = 8;

#[derive(Parser)]
#[command(
    version,
    about,
    after_help = "Run without a command to install with the default settings."
)]
struct Cli {
    #[command(subcommand)]
    command: Option<Command>,
}

#[derive(Subcommand)]
enum Command {
    /// Install the mod into the game's Paks folder (run again to change the limit)
    Install {
        #[command(flatten)]
        game: GameArgs,
        #[command(flatten)]
        limit: LimitArgs,
    },
    /// Remove the mod from the game
    Uninstall {
        #[command(flatten)]
        game: GameArgs,
    },
    /// Build the mod pak without installing it
    Pack {
        /// Pak to read BP_GameInstance from [default: the installed game pak]
        #[arg(long)]
        from: Option<PathBuf>,
        /// Output pak file
        #[arg(long, short, default_value = MOD_PAK)]
        output: PathBuf,
        #[command(flatten)]
        game: GameArgs,
        #[command(flatten)]
        limit: LimitArgs,
    },
}

#[derive(Args)]
struct GameArgs {
    /// Game folder, the one that contains CopyGirls.exe [default: found through Steam]
    #[arg(long)]
    game_dir: Option<PathBuf>,
}

#[derive(Args)]
struct LimitArgs {
    /// Maximum players per session
    #[arg(long, default_value_t = DEFAULT_PLAYERS,
        value_parser = clap::value_parser!(u8).range(i64::from(patch::MIN_PLAYERS)..=i64::from(patch::MAX_PLAYERS)))]
    players: u8,
}

fn main() -> ExitCode {
    let cli = Cli::parse();
    // Launched without arguments means a double click: install and keep the window open.
    let interactive = cli.command.is_none();
    let command = cli.command.unwrap_or(Command::Install {
        game: GameArgs { game_dir: None },
        limit: LimitArgs {
            players: DEFAULT_PLAYERS,
        },
    });

    let code = match run(command) {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("error: {error:#}");
            ExitCode::FAILURE
        }
    };

    if interactive {
        println!("Press Enter to close.");
        let _ = std::io::stdin().read_line(&mut String::new());
    }
    code
}

fn run(command: Command) -> Result<()> {
    match command {
        Command::Install { game, limit } => {
            let game = Game::locate(game.game_dir.as_deref())?;
            warn_about_other_mods(game.paks_dir());
            build(&game.game_pak(), &game.mod_pak(), limit.players)?;
            println!(
                "Installed: up to {} players. Only the host needs the mod.",
                limit.players
            );
        }
        Command::Uninstall { game } => {
            let mod_pak = Game::locate(game.game_dir.as_deref())?.mod_pak();
            if mod_pak.exists() {
                std::fs::remove_file(&mod_pak)?;
                println!("Removed {}", mod_pak.display());
            } else {
                println!("The mod is not installed.");
            }
        }
        Command::Pack {
            from,
            output,
            game,
            limit,
        } => {
            let source = match from {
                Some(path) => path,
                None => Game::locate(game.game_dir.as_deref())?.game_pak(),
            };
            build(&source, &output, limit.players)?;
        }
    }
    Ok(())
}

/// Reads `BP_GameInstance` from `source`, patches it and writes the mod pak.
fn build(source: &Path, output: &Path, players: u8) -> Result<()> {
    ensure!(
        std::path::absolute(source)? != std::path::absolute(output)?,
        "the output must not overwrite the source pak"
    );

    let mut asset = GameInstance::read(source)?;
    patch::set_player_limit(&mut asset.uexp, players)?;
    asset.write(output)?;
    println!("Wrote {}", output.display());
    Ok(())
}

/// Other `_P` paks may replace the same asset and undo or break this mod.
fn warn_about_other_mods(paks_dir: &Path) {
    let Ok(entries) = std::fs::read_dir(paks_dir) else {
        return;
    };
    for name in entries
        .flatten()
        .map(|entry| entry.file_name().to_string_lossy().into_owned())
    {
        if name.ends_with("_P.pak") && name != MOD_PAK {
            eprintln!(
                "warning: another mod is installed ({name}); remove it if it also changes the player limit"
            );
        }
    }
}

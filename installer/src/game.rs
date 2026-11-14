//! Locates the Finals Week installation through Steam.

use std::path::{Path, PathBuf};

use anyhow::{Result, bail};

/// Steam `installdir` of Finals Week.
const INSTALL_DIR: &str = "FinalsWeek";
// Components, not a "/" path, so printed paths use the native separator.
const PAKS_DIR: [&str; 3] = ["CopyGirls", "Content", "Paks"];
const GAME_PAK: &str = "CopyGirls-Windows.pak";

/// The `_P` suffix makes the engine load this pak on top of the game pak.
pub const MOD_PAK: &str = "CopyGirls-Windows_FinalWeekMultiplayer_P.pak";

pub struct Game {
    paks_dir: PathBuf,
}

impl Game {
    /// Uses `game_dir` when given; otherwise the installer's own folder (when it
    /// was extracted into the game folder), then every Steam library.
    pub fn locate(game_dir: Option<&Path>) -> Result<Self> {
        let candidates: Vec<PathBuf> = match game_dir {
            Some(dir) => vec![dir.to_path_buf()],
            None => std::env::current_exe()
                .ok()
                .and_then(|exe| exe.parent().map(Path::to_path_buf))
                .into_iter()
                .chain(
                    steam_libraries()
                        .into_iter()
                        .map(|library| library.join("steamapps").join("common").join(INSTALL_DIR)),
                )
                .collect(),
        };

        for dir in &candidates {
            let paks_dir = dir.join(PathBuf::from_iter(PAKS_DIR));
            if paks_dir.join(GAME_PAK).is_file() {
                return Ok(Self { paks_dir });
            }
        }

        bail!(
            "Finals Week not found (searched: {}); pass --game-dir with the folder that contains CopyGirls.exe",
            candidates
                .iter()
                .map(|dir| dir.display().to_string())
                .collect::<Vec<_>>()
                .join(", ")
        )
    }

    pub fn paks_dir(&self) -> &Path {
        &self.paks_dir
    }

    pub fn game_pak(&self) -> PathBuf {
        self.paks_dir.join(GAME_PAK)
    }

    pub fn mod_pak(&self) -> PathBuf {
        self.paks_dir.join(MOD_PAK)
    }
}

/// Default Steam roots plus every library listed in their `libraryfolders.vdf`.
fn steam_libraries() -> Vec<PathBuf> {
    // shortcut: default install paths only, read the Windows registry if users report a custom Steam folder
    let roots: Vec<PathBuf> = if cfg!(windows) {
        let program_files = std::env::var_os("ProgramFiles(x86)")
            .map_or_else(|| PathBuf::from(r"C:\Program Files (x86)"), PathBuf::from);
        vec![program_files.join("Steam")]
    } else {
        let home = std::env::var_os("HOME")
            .map(PathBuf::from)
            .unwrap_or_default();
        vec![home.join(".steam/steam"), home.join(".local/share/Steam")]
    };

    let mut libraries = Vec::new();
    for root in roots {
        let vdf_path = root.join("steamapps").join("libraryfolders.vdf");
        if let Ok(vdf) = std::fs::read_to_string(vdf_path) {
            libraries.extend(library_paths(&vdf));
        }
        libraries.push(root);
    }
    libraries.dedup();
    libraries
}

/// Extracts the `"path"` values from a `libraryfolders.vdf` file.
fn library_paths(vdf: &str) -> Vec<PathBuf> {
    vdf.lines()
        .filter_map(|line| line.trim().strip_prefix("\"path\""))
        .filter_map(|rest| rest.trim().strip_prefix('"')?.strip_suffix('"'))
        .map(|path| PathBuf::from(path.replace(r"\\", r"\")))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reads_library_paths_from_vdf() {
        let vdf = r#"
"libraryfolders"
{
	"0"
	{
		"path"		"C:\\Program Files (x86)\\Steam"
		"label"		""
	}
	"1"
	{
		"path"		"/home/user/Games/SteamLibrary"
	}
}"#;
        assert_eq!(
            library_paths(vdf),
            [
                PathBuf::from(r"C:\Program Files (x86)\Steam"),
                PathBuf::from("/home/user/Games/SteamLibrary"),
            ]
        );
    }
}

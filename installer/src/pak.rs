//! Reads `BP_GameInstance` from a pak and writes it to a new pak.

use std::fs::File;
use std::io::{BufReader, BufWriter, Write};
use std::path::Path;

use anyhow::{Context, Result};
use repak::{PakBuilder, Version};

const UASSET: &str = "CopyGirls/Content/CopyGirls/Core/BP_GameInstance.uasset";
const UEXP: &str = "CopyGirls/Content/CopyGirls/Core/BP_GameInstance.uexp";

/// The game instance asset plus the pak layout it was read from.
pub struct GameInstance {
    pub uasset: Vec<u8>,
    pub uexp: Vec<u8>,
    version: Version,
    mount_point: String,
    path_hash_seed: Option<u64>,
}

impl GameInstance {
    /// Reads the asset from the game pak or from any pak that contains it.
    pub fn read(pak_path: &Path) -> Result<Self> {
        let mut reader = BufReader::new(
            File::open(pak_path).with_context(|| format!("cannot open {}", pak_path.display()))?,
        );
        let pak = PakBuilder::new()
            .reader(&mut reader)
            .with_context(|| format!("{} is not a supported pak", pak_path.display()))?;

        // The game pak is Oodle-compressed; repak downloads the Oodle library on first use.
        let mut get = |path: &str| {
            pak.get(path, &mut reader)
                .with_context(|| format!("cannot read {path} from {}", pak_path.display()))
        };

        Ok(Self {
            uasset: get(UASSET)?,
            uexp: get(UEXP)?,
            version: pak.version(),
            mount_point: pak.mount_point().to_owned(),
            path_hash_seed: pak.path_hash_seed(),
        })
    }

    /// Writes an uncompressed pak, replacing `output` only once it is complete.
    pub fn write(&self, output: &Path) -> Result<()> {
        let partial = output.with_extension("pak.partial");
        let result = self.write_to(&partial).and_then(|()| {
            std::fs::rename(&partial, output)
                .with_context(|| format!("cannot write {}", output.display()))
        });
        if result.is_err() {
            let _ = std::fs::remove_file(&partial);
        }
        result
    }

    fn write_to(&self, path: &Path) -> Result<()> {
        let file =
            File::create(path).with_context(|| format!("cannot create {}", path.display()))?;
        let mut pak = PakBuilder::new().writer(
            BufWriter::new(file),
            self.version,
            self.mount_point.clone(),
            self.path_hash_seed,
        );
        pak.write_file(UASSET, false, &self.uasset)?;
        pak.write_file(UEXP, false, &self.uexp)?;
        pak.write_index()?.flush()?;
        Ok(())
    }
}

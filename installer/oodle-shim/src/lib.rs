//! Replaces repak's `oodle_loader`, which downloads the proprietary Oodle
//! library at runtime, with the open-source `oozextract` decompressor. The
//! installer then never connects to the internet. Only decompression is
//! supported; the installer writes uncompressed paks.

use std::fmt;

/// Kept for API compatibility with repak; compression is not supported.
#[derive(Debug, Clone, Copy)]
pub enum Compressor {
    Mermaid,
}

/// Kept for API compatibility with repak; compression is not supported.
#[derive(Debug, Clone, Copy)]
pub enum CompressionLevel {
    Normal,
}

#[derive(Debug)]
pub struct Error(String);

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&self.0)
    }
}

impl std::error::Error for Error {}

pub struct Oodle;

impl Oodle {
    /// Returns the number of bytes written, or 0 on failure, like Oodle's `OodleLZ_Decompress`.
    pub fn decompress(&self, input: &[u8], output: &mut [u8]) -> isize {
        oozextract::Extractor::new()
            .read_from_slice(input, output)
            .ok()
            .and_then(|written| isize::try_from(written).ok())
            .unwrap_or(0)
    }

    pub fn compress(
        &self,
        _input: &[u8],
        _compressor: Compressor,
        _level: CompressionLevel,
    ) -> Result<Vec<u8>, Error> {
        Err(Error("Oodle compression is not supported".into()))
    }
}

pub fn oodle() -> Result<&'static Oodle, Error> {
    Ok(&Oodle)
}

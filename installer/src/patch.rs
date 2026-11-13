//! Patches the player limit inside the cooked `BP_GameInstance` bytecode.

use anyhow::{Result, bail};

/// Smallest and largest player limit the mod accepts.
pub const MIN_PLAYERS: u8 = 2;
pub const MAX_PLAYERS: u8 = 64;

/// Kismet opcode `EX_IntConst`, followed by a little-endian `i32`.
const EX_INT_CONST: u8 = 0x1D;

/// Arguments that follow `PublicConnections` in the game's `CreateAdvancedSession`
/// call: `PrivateConnections = 0` and its seven boolean flags.
#[rustfmt::skip]
const TRAILER: [u8; 12] = [EX_INT_CONST, 0, 0, 0, 0, 0x28, 0x27, 0x28, 0x27, 0x27, 0x28, 0x28];

/// Length of `EX_IntConst` plus its `i32` operand.
const INT_CONST_LEN: usize = 5;

/// Sets `PublicConnections` of the `CreateAdvancedSession` call to `players`.
///
/// Fails unless the call is found exactly once, so an unknown game build is
/// rejected instead of patched in the wrong place.
pub fn set_player_limit(uexp: &mut [u8], players: u8) -> Result<()> {
    let window = INT_CONST_LEN + TRAILER.len();
    let mut matches = uexp
        .windows(window)
        .enumerate()
        .filter(|(_, w)| w[0] == EX_INT_CONST && w[INT_CONST_LEN..] == TRAILER)
        .map(|(offset, _)| offset);

    let (Some(offset), None) = (matches.next(), matches.next()) else {
        bail!("player limit not found in BP_GameInstance; this game build is not supported");
    };

    let value = i32::from(players).to_le_bytes();
    uexp[offset + 1..offset + INT_CONST_LEN].copy_from_slice(&value);
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn call_with(players: u8) -> Vec<u8> {
        let mut bytes = vec![0xAA, 0xBB, EX_INT_CONST, players, 0, 0, 0];
        bytes.extend_from_slice(&TRAILER);
        bytes.push(0xCC);
        bytes
    }

    #[test]
    fn patches_only_the_public_connections_operand() {
        let mut uexp = call_with(4);
        set_player_limit(&mut uexp, 8).unwrap();
        assert_eq!(uexp, call_with(8));
    }

    #[test]
    fn rejects_missing_call() {
        let mut uexp = vec![EX_INT_CONST, 4, 0, 0, 0];
        assert!(set_player_limit(&mut uexp, 8).is_err());
    }

    #[test]
    fn rejects_ambiguous_call() {
        let mut uexp = call_with(4);
        uexp.extend(call_with(4));
        let original = uexp.clone();
        assert!(set_player_limit(&mut uexp, 8).is_err());
        assert_eq!(uexp, original, "nothing is written on failure");
    }
}

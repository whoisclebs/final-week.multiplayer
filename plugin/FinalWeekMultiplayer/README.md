# Final Week Multiplayer — Unreal Engine plugin

C++ plugin for **Unreal Engine 5** that provides cooperative sessions with a configurable player limit, session search and Blueprint events.

> [!NOTE]
> This plugin is for **Unreal Engine projects**. The shipped Finals Week game cannot load C++ plugins; to raise its player limit use the installer described in the [main README](../../README.md).

All session logic lives in a single `GameInstance` subsystem. Settings live in Project Settings, the player limit is always validated, and every asynchronous operation reports its result through a delegate.

## Features

- **Configurable limit** from 2 to 64 players, validated when edited and when used.
- **Session creation** over LAN or an online provider, with optional advertising and a start map.
- **Safe replacement**: an existing session is destroyed before the new one is created.
- **Session search** with a result limit and a lobby filter.
- **Join error handling**: full session, missing session, unavailable address and already-in-session each get their own message.
- **Automatic `ClientTravel`** after joining.
- **Delegates** for create, join, leave and search.
- **Dedicated log category** (`LogFWMSession`).

## Requirements

- Unreal Engine **5.1 or newer**, C++ project.
- An online subsystem configured in the project (Steam, Epic Online Services, LAN or the Null subsystem for testing).

## Installation

1. Copy this folder to `Plugins/FinalWeekMultiplayer` in your project:

   ```
   MyProject/
   └── Plugins/
       └── FinalWeekMultiplayer/
           ├── FinalWeekMultiplayer.uplugin
           └── Source/
   ```

2. Regenerate the project files (right-click the `.uproject` → **Generate Visual Studio project files**).
3. Build the project.
4. Enable **Final Week Multiplayer** in **Edit → Plugins** and restart the editor.

## Configuration

Settings are in **Project Settings → Game → Final Week Multiplayer** and are saved to `Config/DefaultGame.ini`:

```ini
[/Script/FinalWeekMultiplayer.FWMSettings]
MaxPlayers=12
SessionName=Final Week
bUseLANMatch=False
bAdvertiseSession=True
MapName=
```

| Property | Default | Description |
|---|---|---|
| `MaxPlayers` | `8` | Players per session (2 to 64) |
| `SessionName` | `Final Week` | Name shown in the server list |
| `bUseLANMatch` | `False` | Host on the local network instead of the online provider |
| `bAdvertiseSession` | `True` | Advertise the session to other players |
| `MapName` | empty | Map loaded when the session is created; empty keeps the current map |

The limit can also be changed at runtime:

```cpp
UFWMSettings* Settings = GetMutableDefault<UFWMSettings>();
Settings->SetMaxPlayers(16); // always clamped to 2..64
```

## Usage

### Blueprint

1. Get the subsystem with **Get Game Instance Subsystem → FWMSessionSubsystem**.
2. Bind `On Session Created`, `On Sessions Found`, `On Session Joined` and `On Session Left`.
3. Call `Host Session` to create, `Find Sessions` to search and `Join Found Session` (with the result index) to join.

Every delegate delivers `bSuccessful` and a display-ready `Message`.

### C++

```cpp
#include "FWMSessionSubsystem.h"

void AMyGameMode::StartCoop()
{
    UFWMSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UFWMSessionSubsystem>();
    Sessions->OnSessionCreated.AddDynamic(this, &AMyGameMode::HandleSessionCreated);
    Sessions->HostSession();
}

void AMyGameMode::HandleSessionCreated(bool bSuccessful, const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("Session: %s"), *Message);
}
```

## API

### `UFWMSessionSubsystem`

| Method | Returns | Description |
|---|---|---|
| `HostSession()` | — | Creates a session with the configured limit |
| `FindSessions(int32 MaxResults)` | — | Searches sessions (1 to 200 results) |
| `JoinFoundSession(int32 Index)` | `bool` | Joins the session at the given index |
| `LeaveSession()` | — | Destroys the current session |
| `IsInSession()` | `bool` | Whether a session is active |
| `GetCurrentPlayers()` | `int32` | Connected players |
| `GetMaxPlayers()` | `int32` | Effective configured limit |
| `GetFoundSessionCount()` | `int32` | Results of the last search |

### Events

| Event | Parameters | Fired when |
|---|---|---|
| `OnSessionCreated` | `bool bSuccessful`, `FString Message` | Creation finished |
| `OnSessionJoined` | `bool bSuccessful`, `FString Message` | Join finished |
| `OnSessionLeft` | `bool bSuccessful`, `FString Message` | Leave finished |
| `OnSessionsFound` | `bool bSuccessful`, `FString Message` | Search finished |

### `UFWMSettings`

| Member | Type | Description |
|---|---|---|
| `MaxPlayers` | `int32` | Configured limit |
| `SessionName` | `FString` | Session name |
| `bUseLANMatch` | `bool` | LAN session |
| `bAdvertiseSession` | `bool` | Advertise the session |
| `MapName` | `FString` | Start map |
| `GetEffectiveMaxPlayers()` | `int32` | Limit clamped to the valid range |
| `SetMaxPlayers(int32)` | — | Sets and clamps the limit |

## Behavior

- **Always-valid limit**: `MaxPlayers` is clamped to 2..64 when edited, when saved and when a session is created.
- **No duplicate sessions**: hosting while a session exists destroys it first and creates the new one from the destroy callback.
- **No calls without a subsystem**: if the online provider is unavailable, the operation fails with a clear message.
- **LAN and online**: on LAN, presence and lobbies are disabled; online searches filter by lobbies.
- **Joining**: the engine result is translated to a readable message; on success the player travels to the server with `ClientTravel`.

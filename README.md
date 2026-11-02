# ChillSpawn

**English** · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

A server-side DayZ mod. Players type `!car` in chat to spawn a ready-to-drive red Olga 24 six metres north of them: four wheels, spark plug, radiator, charged battery, headlights, doors, hood and trunk, with full fuel, coolant, oil and brake fluid. If that spot is blocked or in water, it tries nearby spots; if none is clear, the player is told to move and the attempt doesn't count against their limit. They get a private message saying it despawns within 15 minutes. Each player can spawn one Olga every 15 minutes of playtime (time spent connected, not wall-clock time). Spawned cars take no damage, and nobody inside is hurt when they crash.

Players can also type `!survive` once every 2 real hours for a small survival kit. Admins have no limits and get an `!adminkit`.

Clients don't need the mod. Load it with `-servermod=@ChillSpawn`. Written for DayZ 1.30.

> **Status: tested on DayZ 1.30 experimental.** Every command, admins, kits and car protection work on a live server with no client mod. See [Known limits / test first](#known-limits--test-first). [docs/PROMPTING.md](docs/PROMPTING.md) shows how it was made.

## Layout

```
ChillSpawn/                    -> packs to @ChillSpawn/addons/ChillSpawn.pbo
  config.cpp                   CfgPatches/CfgMods, script modules
  scripts/3_Game/ChillSpawn/   CS_Config (JSON config), CS_PlayerTracker (playtime + rate limit)
  scripts/4_World/ChillSpawn/  CS_Core, CS_Commands, CS_VehicleSpawner, CS_KitSpawner,
                               CS_CarProtection, CS_Messenger
  scripts/5_Mission/ChillSpawn/ MissionServer hooks (chat, connect, disconnect)
config.example.json            the default config, for reference
docs/PROMPTING.md              how this mod was made with Claude, and prompts to make your own
tools/build-linux.sh           builds the PBO on Linux (see Building)
```

## Configuration

On first start the mod writes `$profile:ChillSpawn/config.json` (the server's `-profiles` folder). Every entry in `Presets` and `Kits` becomes its own chat command, so server owners can add more without touching code. The defaults:

| Command | Spawns | Limit |
|---|---|---|
| `!car` | Red Olga 24, ready to drive, no damage, crew protected | 1 every 15 min of play |
| `!ada` | Green Ada 4x4, the same (`OffroadHatchback_Blue` / `_White` for other colours, with matching `_Blue` / `_White` door, hood and trunk classes) | 1 every 15 min of play |
| `!hatchet` | A hatchet on the ground 1 m away | 3 every 60 min of play |
| `!survive` | Into the player's inventory: a can of bacon, a full bottle of water, purification tablets, a bandage and tetracycline | 1 every 2 real hours |
| `!adminkit` | Admins only. On the ground in front of them: Alice backpack, M14 with 3 full 20-round magazines and 3 boxes of .308, M4A1 with 3 full 60-round magazines and 2 suppressors, full plate carrier, MICH helmet with night vision and batteries, TTsKO camo jacket, trousers and boots, gloves and a hunting knife | none |

Two built-in commands are always available:

| Command | Does |
|---|---|
| `!list` | Lists everything the player can spawn, one per line, with how many they have left and, when none are left, how long until the next |
| `!help` | Lists the commands the player can use |

A preset or kit can't be called `list` or `help`; the built-ins win and the server log warns.

### Admins

Put admins' Steam64 IDs in `Admins`:

```json
"Admins": ["76561198000000000"],
```

Admins have no limits and can use commands marked `AdminOnly`. Other players don't see admin-only commands in `!help` or `!list`, and nothing happens if they type one.

### Presets

Each preset has its own limit (one Olga and one Ada every 15 minutes are separate allowances). To add a preset, copy a block in `Presets`, give it a new `Command`, and restart the server. Fields:

| Field | Meaning |
|---|---|
| `Command` | Chat command, without the prefix (`car` → `!car`) |
| `DisplayName` | Name shown in the message |
| `ClassName` | Vehicle or item class to spawn |
| `Attachments` | Classes attached to it. Repeat a class for each slot it fills (four wheels) |
| `DistanceMetres`, `BearingDegrees` | Where it spawns relative to the player. 0° = north, 90° = east |
| `DespawnSeconds` | 0 = never despawn |
| `MaxSpawns`, `LimitMinutes` | Each player can spawn it `MaxSpawns` times every `LimitMinutes` minutes of playtime |
| `AdminOnly` | Only admins can use it |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutes until despawn |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Vehicles only: how full each fluid starts, 0 to 1 |
| `Invulnerable` | It and its attachments take no damage at all: crashes, bullets, explosions |
| `ProtectCrew` | Vehicles only: nobody inside is hurt or killed when it crashes |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Clear space (metres) needed at the spawn point. The default fits an Olga; raise it for trucks |

### Kits

A kit is a set of items for one command. Fields:

| Field | Meaning |
|---|---|
| `Command`, `DisplayName` | As for presets |
| `MaxSpawns`, `LimitMinutes` | Limit per player |
| `RealTimeLimit` | Count `LimitMinutes` in real-world time instead of playtime (survives restarts) |
| `AdminOnly` | Only admins can use it |
| `ToInventory` | Put items on the player where there's room; the rest go on the ground. Otherwise everything goes on the ground in front of them |
| `Items` | `ClassName`, `Count` and `Contents`. Contents go into the item: attached where they fit (magazines, suppressors, batteries, holsters), otherwise into its cargo, and can have contents of their own |

Kit items spawn pristine, magazines and ammo full, batteries charged and bottles full of water. A magazine listed in a gun's contents is loaded with a round chambered.

The default config is only written when no `config.json` exists. After updating the mod, delete the old file (or add the new fields to it) to pick up new defaults; the server log warns when the file is from an older version.

Per-player playtime and spawn history are saved in `$profile:ChillSpawn/players/`.

## Adding commands (expansion)

Subclass `CS_ChatCommand` and register it from another mod, or in this one:

```c
class MyRulesCommand : CS_ChatCommand
{
	override string GetName() { return "rules"; }

	override void Execute(PlayerBase player, array<string> args)
	{
		CS_Messenger.Send(player, "No KOS in the safe zone.");
	}
}

modded class CS_Core
{
	override protected void RegisterCommands()
	{
		super.RegisterCommands();
		Commands.Register(new MyRulesCommand());
	}
}
```

## Known limits / test first

The basics work on DayZ 1.30 experimental. Check these on a test server when changing the config or updating the game:

1. **Chat hook: works.** The mod reads chat through `MissionServer.OnEvent` with `ChatMessageEventTypeID`, so players don't need to install anything. The [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) lists `ChatMessageEventTypeID` as a client-side event, but on a DayZ 1.30 experimental server the server receives it too. The event only gives the sender's name, so players are matched by name. If two players share a name, the command is ignored.
2. **Attachment and kit class names.** Check every class in `Attachments` and every kit item spawns (the server log prints `Could not attach …`, `Could not create …` or `Could not put …` for any that don't), and that the car starts and drives.
3. **Car protection.** `Invulnerable` stops damage to the car and the parts it spawned with; parts fitted later aren't protected. `ProtectCrew` stops crash damage to the people inside. Neither protects people the car hits. Both are kept in memory, so after a server restart a car that hasn't despawned yet is an ordinary car again.
4. **Obstacle check.** Uses `IsBoxColliding` with a box the size of the car, lifted 0.3 m off the ground, plus a sea/pond check. It tries 10 directions at 6, 9 and 12 m. Check it doesn't wrongly treat bushes as obstacles, and that it catches walls and fences. It doesn't check slope, so a car can still land on a steep hillside.
5. **Despawn timers live in memory.** If the server restarts before a car's timer runs out, the car stays in the world as a normal persistent vehicle. If a player is in the car when the timer runs out, the despawn is retried every minute until they get out.

## Building

### Linux

```bash
tools/build-linux.sh
```

This packs `dist/@ChillSpawn/addons/ChillSpawn.pbo` with the PBO builder from [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools is freeware under its own licence and isn't included here: the script clones it into `~/.cache/rag-dayz-tools` on first run, or uses the path you pass (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) or set in `RAG_TOOLS_DIR`. Needs `python3` 3.10+ and `git`.

The script runs RaG's preflight checks and packs the scripts as they are. It skips binarising, `config.cpp` conversion and signing, which need Windows DayZ Tools programs. A scripts-only server mod doesn't need any of them. Nothing on Linux compiles Enforce Script, so script errors only show up in the server's `.rpt` log when it starts.

### Windows

Pack the `ChillSpawn/` folder into `@ChillSpawn/addons/ChillSpawn.pbo` with DayZ Tools' AddonBuilder or the RaG PBO Builder. Server-only mods don't strictly need signing, but if the server checks signatures, sign the PBO and put the `.bikey` in the server's `keys/` folder.

## Installing

1. Copy the `@ChillSpawn` folder into the server's root folder, next to `DayZServer_x64.exe`.
2. Load it as a server mod:
   - **Plain server:** add `-servermod=@ChillSpawn` to the start line.
   - **CFTools Architect:** use **Add Local Mod**, choose `@ChillSpawn` and set its type to **Server**.
3. Start the server. It writes `profiles/ChillSpawn/config.json` and logs `[ChillSpawn] Loaded … spawn presets, … kits and … admins.`
4. Add your admins' Steam64 IDs to `Admins` in that file and restart.

**Updating:** stop the server fully before replacing `ChillSpawn.pbo`. A running server locks the file, and a quick restart may not leave time to swap it.

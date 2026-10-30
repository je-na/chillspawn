# ChillSpawn

**English** · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

A server-side DayZ mod. Players type `!car` in chat to spawn a ready-to-drive red Olga 24 six metres north of them: four wheels, spark plug, radiator, charged battery, headlights, doors, hood and trunk, with full fuel, coolant, oil and brake fluid. If that spot is blocked or in water, it tries nearby spots; if none is clear, the player is told to move and the attempt doesn't count against their limit. They get a private message saying it despawns within 15 minutes. Each player can spawn 3 per hour of playtime (time spent connected, not wall-clock time).

Clients don't need the mod. Load it with `-servermod=@ChillSpawn`. Written for DayZ 1.30.

> **Status: untested.** The code was written with Claude and has not yet been compiled or run on a server. See [Known limits / test first](#known-limits--test-first) before using it on a live server. [docs/PROMPTING.md](docs/PROMPTING.md) shows how it was made.

## Layout

```
ChillSpawn/                    -> packs to @ChillSpawn/addons/ChillSpawn.pbo
  config.cpp                   CfgPatches/CfgMods, script modules
  scripts/3_Game/ChillSpawn/   CS_Config (JSON config), CS_PlayerTracker (playtime + rate limit)
  scripts/4_World/ChillSpawn/  CS_Core, CS_Commands, CS_VehicleSpawner, CS_Messenger
  scripts/5_Mission/ChillSpawn/ MissionServer hooks (chat, connect, disconnect)
config.example.json            the default config, for reference
docs/PROMPTING.md              how this mod was made with Claude, and prompts to make your own
tools/build-linux.sh           builds the PBO on Linux (see Building)
```

## Configuration

On first start the mod writes `$profile:ChillSpawn/config.json` (the server's `-profiles` folder). Every preset in `Presets` becomes its own chat command, so server owners can add more vehicles or items without touching code. The default has three:

| Command | Spawns |
|---|---|
| `!car` | Red Olga 24, ready to drive |
| `!ada` | Green Ada 4x4, ready to drive (`OffroadHatchback_Blue` / `_White` for other colours, with matching `_Blue` / `_White` door, hood and trunk classes) |
| `!hatchet` | A hatchet on the ground 1 m away |

Two built-in commands are always available:

| Command | Does |
|---|---|
| `!list` | Lists every preset, one per line, with how many spawns the player has left this hour of play and, when none are left, how long until the next |
| `!help` | Lists all commands |

A preset can't be called `list` or `help`; the built-ins win and the server log warns.

Each preset has its own limit (3 Olgas and 3 hatchets an hour are separate allowances). To add a preset, copy a block in `Presets`, give it a new `Command`, and restart the server. Fields:

| Field | Meaning |
|---|---|
| `Command` | Chat command, without the prefix (`car` → `!car`) |
| `DisplayName` | Name shown in the message |
| `ClassName` | Vehicle or item class to spawn |
| `Attachments` | Classes attached to it. Repeat a class for each slot it fills (four wheels) |
| `DistanceMetres`, `BearingDegrees` | Where it spawns relative to the player. 0° = north, 90° = east |
| `DespawnSeconds` | 0 = never despawn |
| `MaxPerPlaytimeHour` | Spawn limit per hour of playtime, counted per preset |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutes until despawn |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Vehicles only: how full each fluid starts, 0 to 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Clear space (metres) needed at the spawn point. The default fits an Olga; raise it for trucks |

The default config is only written when no `config.json` exists. After updating the mod, delete the old file (or add the new fields to it) to pick up new defaults.

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

Not yet compiled or tested. Check these on a test server first:

1. **Chat hook (most likely to fail).** The mod reads chat through `MissionServer.OnEvent` with `ChatMessageEventTypeID`, so players don't need to install anything. The [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) lists `ChatMessageEventTypeID` as a client-side event, and its chat-command tutorial uses a small client mod instead. Test this first: if `!help` gets no reply, the server isn't receiving chat events, and the mod will need a client-side part. The event only gives the sender's name, so players are matched by name. If two players share a name, the command is ignored.
2. **Attachment class names.** Check every class in `Attachments` attaches (the server log prints `Could not attach …` for any that don't), and that the car starts and drives.
3. **Obstacle check.** Uses `IsBoxColliding` with a box the size of the car, lifted 0.3 m off the ground, plus a sea/pond check. It tries 10 directions at 6, 9 and 12 m. Check it doesn't wrongly treat bushes as obstacles, and that it catches walls and fences. It doesn't check slope, so a car can still land on a steep hillside.
4. **Despawn timers live in memory.** If the server restarts before a car's timer runs out, the car stays in the world as a normal persistent vehicle. If a player is in the car when the timer runs out, the despawn is retried every minute until they get out.

## Building

### Linux

```bash
tools/build-linux.sh
```

This packs `dist/@ChillSpawn/addons/ChillSpawn.pbo` with the PBO builder from [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools is freeware under its own licence and isn't included here: the script clones it into `~/.cache/rag-dayz-tools` on first run, or uses the path you pass (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) or set in `RAG_TOOLS_DIR`. Needs `python3` 3.10+ and `git`.

The script runs RaG's preflight checks and packs the scripts as they are. It skips binarising, `config.cpp` conversion and signing, which need Windows DayZ Tools programs. A scripts-only server mod doesn't need any of them. Nothing on Linux compiles Enforce Script, so script errors only show up in the server's `.rpt` log when it starts.

Copy `dist/@ChillSpawn` into the server folder and add `-servermod=@ChillSpawn` to the start line.

### Windows

Pack the `ChillSpawn/` folder into `@ChillSpawn/addons/ChillSpawn.pbo` with DayZ Tools' AddonBuilder or the RaG PBO Builder. Server-only mods don't strictly need signing, but if the server checks signatures, sign the PBO and put the `.bikey` in the server's `keys/` folder.

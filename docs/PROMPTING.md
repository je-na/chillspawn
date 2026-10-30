# How ChillSpawn was made with Claude

**Translate (machine):** [Slovenčina](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=sk) · [Deutsch](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=de) · [Українська](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=uk) · [Polski](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=pl) · [Français](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=fr) · [Español](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=es) · [Português](https://translate.google.com/translate?sl=en&u=https://github.com/je-na/chillspawn/blob/main/docs/PROMPTING.md&tl=pt)

ChillSpawn was written in one Claude Code session (Claude Opus 5.5) on a Linux machine with no DayZ install. Claude wrote every file. The person steering it wrote the requirements, asked for changes and compared the result with two local models. Nothing was compiled or run in game during the session, so treat the code as a strong first draft that still needs a test server.

This page walks through the requests in order (reworded and tidied, not quoted word for word), what each one changed and what was learnt. It ends with prompts you can reuse to make a similar mod.

## The prompts, in order

### 1. The first request

> Write a server-side DayZ mod that lets a player spawn a car with a chat command:
>
> - The car is the red Olga, with four wheels, a spark plug and a radiator attached.
> - It spawns 6 metres north of the player.
> - The player gets a private message saying the car will despawn within 15 minutes.
> - Each player can spawn 3 cars per hour of time spent in game.
> - Include a framework so the mod can be extended later.
> - A JSON config lets the server owner choose what to spawn: a car plus its attachments, or an item.

Claude produced the whole mod: `config.cpp`, the 3_Game/4_World/5_Mission script split, a JSON config written to `$profile:` on first start, a command registry (the "framework"), playtime tracking saved per player, and a README. It also flagged what it was unsure of rather than hiding it:

- that the server receives chat through `MissionServer.OnEvent` with `ChatMessageEventTypeID`, and only gets the sender's name;
- that an Olga with only wheels, spark plug and radiator **won't drive** (no battery or fluids);
- that despawn timers are lost on restart;
- that there was no obstacle check.

**Lesson:** a numbered list of concrete requirements (exact vehicle, distance, direction, limits) gets a complete answer in one go. Ask Claude to list its assumptions and anything it couldn't verify.

### 2. Make it drivable and check for obstacles

> Add whatever else the car needs so it spawns ready to drive. Also check for obstacles before spawning it.

Added battery (charged), headlights, doors, hood and trunk, and filled fuel, coolant, oil and brake fluid (all configurable). The obstacle check uses `IsBoxColliding` with a car-sized box plus a sea/pond check. It tries 10 directions at three distances. If nothing is clear, the player is told and the attempt doesn't count against their limit.

**Lesson:** follow-ups that respond to Claude's own "known limits" list are cheap and targeted.

### 3. More than one spawnable thing

> Make the example config show several spawnable things, for example a hatchet and the Ada 4x4.

The config already supported this (each preset is its own chat command). Claude added `!ada` and `!hatchet` presets to both the defaults and `config.example.json`, and documented how to add more.

### 4. Comparing with local models

> Have my local Qwen model write the same mod, then compare its version with yours: how different are they, and does either have errors?

Claude sent the same combined requirements to a local Qwen model, and later reviewed a second Qwen attempt the user supplied. Neither would have worked:

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| Real DayZ API | Mostly invented (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Mostly real |
| Would load | No: no script modules, never hooks `MissionServer` | No: `config.cpp` has no `CfgMods` |
| Would compile | No | No: ternary operator, C++-style iterators, `ToLower()` used as if it returns a string |
| "North" | 6 m up in the air | 6 m south |
| Rate limit | In memory, fixed blocks | Server uptime, lost on restart |

**Lesson:** for Enforce Script, general-purpose models confidently write C++/Java-isms that DayZ doesn't support. Get a second opinion by having one model review another's code against a checklist (load, compile, behaviour), not by trusting either.

### 5. Borrowing a good idea

> Bring the second Qwen version's idea of a vehicle/item list into ChillSpawn.

Qwen #2 had a `/vspawn list` subcommand. Claude added `!list` (each preset with how many spawns the player has left this hour) and `!help` as built-in commands in ChillSpawn's framework.

### 6. Publishing

> Commit this to my public GitHub repo (MIT licence; replace the README if you like). Make sure no credentials go in. Add a doc showing how the prompting was done and what prompts someone else could use to make a similar mod.

That produced this repository and this page.

### 7. Checking against the 1.30 update

> Check the 1.30 patch notes and Bohemia's feedback tracker for anything that could break our logic.

Nothing in the notes touched the APIs the mod uses. 1.30 brought a stricter script compiler, so two constructs it might reject were changed (a method declared without a body, and a float-to-int conversion).

### 8. Building on Linux

> See whether RaG DayZ Tools can build this on Linux.

RaG DayZ Tools is Windows-first, but its PBO builder is plain Python and runs on Linux. `tools/build-linux.sh` uses it to pack the mod. Binarising, config conversion and signing still need Windows programs, and a scripts-only server mod doesn't need them.

## Prompts you can reuse

### Starting a new server-side mod

Copy this, fill in the brackets and paste it into Claude (Claude Code works best, since it writes the files for you):

```text
Write a complete DayZ server-side mod in Enforce Script for DayZ [version].
Players do not install it; it is loaded with -servermod.

Output every file in full:
- config.cpp with CfgPatches and CfgMods, with gameScriptModule /
  worldScriptModule / missionScriptModule pointing at scripts/3_Game,
  scripts/4_World and scripts/5_Mission
- the .c script files in the right module (3_Game cannot use PlayerBase;
  MissionServer hooks go in 5_Mission, using `modded class MissionServer`)
- an example JSON config, loaded with JsonFileLoader from $profile:<ModName>/
- a README covering install, config fields and known limits

Requirements:
1. [what the player does, e.g. types !car in chat]
2. [exactly what spawns: class name if you know it, attachments, fluids]
3. [where: distance and compass direction from the player]
4. [messages to the player, and whether they are private]
5. [limits: how many, per what period, and whether they survive reconnects/restarts]
6. [anything the server owner must be able to configure]
7. A small framework so more chat commands can be added later.

Use only functions that exist in vanilla DayZ scripts. Enforce Script has no
ternary operator, string.ToLower() changes the string in place and returns an
int, and arrays/maps of objects need `ref` elements. North is +Z; Y is height.

After the code, list every assumption you made and anything you could not
verify (class names, API calls, engine behaviour) so I can check it on a
test server.
```

The "Enforce Script has no…" line comes straight from the mistakes the local models made. Leaving it in costs nothing and prevents the most common errors.

### Follow-ups that worked well

- "Make the vehicle spawn fully drivable: every part it needs, charged battery and all fluids full."
- "Add an obstacle check before spawning. If the spot is blocked or in water, try nearby spots; if none is clear, tell the player and don't count it against their limit."
- "Add a `!list` command showing every spawnable preset and how many the player has left."
- "Review this code [paste or attach] for things that won't compile or load in DayZ, and for logic errors against these requirements: [list]."
- "Write a test plan I can follow on a local server to check each requirement."

### Tips

- **Be exact about game objects.** "Red Olga" became `CivilianSedan_Wine`. If you know the class names, give them; if not, ask Claude to list the ones it used so you can check them in the server's `types.xml` or with an admin tool.
- **Say what "per hour" means.** "Per hour of playtime" and "per hour of server time" are different features, and only one of them survives a reconnect.
- **Ask for a "known limits" section** and treat it as your test list.
- **Test on a local server before going live.** Watch the server `.rpt` log for script errors and lines like `[ChillSpawn] Could not attach …`.
- **On Windows with DayZ Tools installed**, the [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) plugin for Claude Code adds DayZ-specific agents and skills that build PBOs and launch a local test server, which closes the "untested" gap.

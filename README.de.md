# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · **Deutsch** · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Maschinelle Übersetzung. Bei Abweichungen gilt das [englische Original](README.md).

Ein serverseitiges DayZ-Mod. Spieler geben `!car` im Chat ein, um eine fahrbereite rote Olga 24 sechs Meter nördlich von sich zu spawnen: vier Räder, Zündkerze, Kühler, geladene Batterie, Scheinwerfer, Türen, Motorhaube und Kofferraum, mit vollem Kraftstoff-, Kühlwasser-, Öl- und Bremsflüssigkeitsstand. Wenn dieser Punkt blockiert ist oder im Wasser liegt, werden nahegelegene Punkte versucht; wenn keiner frei ist, wird der Spieler angewiesen, sich zu bewegen, und der Versuch zählt nicht gegen sein Limit. Er erhält eine private Nachricht, dass das Fahrzeug innerhalb von 15 Minuten verschwindet. Jeder Spieler kann alle 15 Minuten Spielzeit (Zeit im Server, keine echte Uhrzeit) ein Olga spawnen. Gespawnete Fahrzeuge nehmen keinen Schaden, und niemand im Inneren wird bei einem Unfall verletzt.

Spieler können auch einmal alle 2 echten Stunden `!survive` eingeben, um einen kleinen Survival-Kit zu erhalten. Administratoren haben keine Limits und erhalten einen `!adminkit`.

Clients benötigen das Mod nicht. Laden Sie es mit `-servermod=@ChillSpawn`. Entwickelt für DayZ 1.30.

> **Status: getestet auf DayZ 1.30 experimental.** Jeder Befehl, Admins, Kits und der Fahrzeugschutz funktionieren auf einem Live-Server ohne Client-Mod. Siehe [Bekannte Grenzen / zuerst testen](#bekannte-grenzen--zuerst-testen). [docs/PROMPTING.md](docs/PROMPTING.de.md) zeigt, wie es erstellt wurde.

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

## Konfiguration

Beim ersten Start schreibt das Mod `$profile:ChillSpawn/config.json` (im `-profiles`-Ordner des Servers). Jeder Eintrag in `Presets` und `Kits` wird zu einem eigenen Chat-Befehl, sodass Serverbesitzer weitere hinzufügen können, ohne den Code zu berühren. Die Standards:

| Befehl | Spawnt | Limit |
|---|---|---|
| `!car` | Rote Olga 24, fahrbereit, kein Schaden, Besatzung geschützt | 1 alle 15 Min. Spielzeit |
| `!ada` | Grüne Ada 4x4, dasselbe (`OffroadHatchback_Blue` / `_White` für andere Farben, mit passenden `_Blue` / `_White` Tür-, Hauben- und Kofferraumklassen) | 1 alle 15 Min. Spielzeit |
| `!hatchet` | Eine Axt auf dem Boden in 1 m Entfernung | 3 alle 60 Min. Spielzeit |
| `!survive` | In das Inventar des Spielers: Dose Speck, volle Flasche Wasser, Reinigungstabletten, Verband und Tetracyclin | 1 alle 2 echte Stunden |
| `!adminkit` | Nur Admins. Auf dem Boden vor ihnen: Alice-Rucksack, M14 mit 3 vollen 20-Schuss-Magazinen und 3 Dosen .308, M4A1 mit 3 vollen 60-Schuss-Magazinen und 2 Schalldämpfern, voller Plattenpanzer, MICH-Helm mit Nachtsicht und Batterien, TTsKO-Tarnjacke, Hose und Stiefel, Handschuhe und Jagdmesser | keine |

Zwei integrierte Befehle sind immer verfügbar:

| Befehl | Funktion |
|---|---|
| `!list` | Listet alles auf, was der Spieler spawnen kann, eine Zeile pro Eintrag, mit verbleibenden Counts und, wenn keine mehr übrig sind, der Zeit bis zum nächsten Spawn |
| `!help` | Listet die Befehle auf, die der Spieler verwenden kann |

Ein Preset oder Kit darf nicht `list` oder `help` heißen; die integrierten Befehle haben Vorrang, und das Server-Log warnt.

### Admins

Fügen Sie die Steam64-IDs der Admins in `Admins` ein:

```json
"Admins": ["76561198000000000"],
```

Admins haben keine Limits und können Befehle mit `AdminOnly` verwenden. Andere Spieler sehen Admin-only-Befehle weder in `!help` noch in `!list`, und nichts passiert, wenn sie einen eingeben.

### Presets

Jedes Preset hat sein eigenes Limit (eine Olga und eine Ada alle 15 Minuten sind separate allowances). Um ein Preset hinzuzufügen, kopieren Sie einen Block in `Presets`, geben Sie ihm einen neuen `Command` und starten Sie den Server neu. Felder:

| Feld | Bedeutung |
|---|---|
| `Command` | Chat-Befehl, ohne Präfix (`car` → `!car`) |
| `DisplayName` | Name, der in der Nachricht angezeigt wird |
| `ClassName` | Fahrzeug- oder Item-Klasse zum Spawnen |
| `Attachments` | Klassen, die daran angehängt werden. Wiederholen Sie eine Klasse für jeden Slot, den sie füllt (vier Räder) |
| `DistanceMetres`, `BearingDegrees` | Ort des Spawns relativ zum Spieler. 0° = Norden, 90° = Osten |
| `DespawnSeconds` | 0 = nie despawnen |
| `MaxSpawns`, `LimitMinutes` | Jeder Spieler kann es `MaxSpawns`-mal alle `LimitMinutes` Minuten Spielzeit spawnen |
| `AdminOnly` | Nur Admins können es verwenden |
| `SpawnMessage` | `%1` = DisplayName, `%2` = Minuten bis zum Despawn |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Nur Fahrzeuge: Wie voll jede Flüssigkeit zu Beginn ist, 0 bis 1 |
| `Invulnerable` | Es und seine Attachments nehmen überhaupt keinen Schaden: Unfälle, Kugeln, Explosionen |
| `ProtectCrew` | Nur Fahrzeuge: Niemand im Inneren wird bei einem Unfall verletzt oder getötet |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Benötigter Freiraum (Meter) am Spawn-Punkt. Der Standard passt zu einer Olga; erhöhen Sie ihn für LKW |

### Kits

Ein Kit ist eine Gruppe von Items für einen Befehl. Felder:

| Feld | Bedeutung |
|---|---|
| `Command`, `DisplayName` | Wie bei Presets |
| `MaxSpawns`, `LimitMinutes` | Limit pro Spieler |
| `RealTimeLimit` | Zählen Sie `LimitMinutes` in echter Zeit statt Spielzeit (überlebt Neustarts) |
| `AdminOnly` | Nur Admins können es verwenden |
| `ToInventory` | Items auf den Spieler legen, wo Platz ist; der Rest landet auf dem Boden. Andernfalls alles auf dem Boden vor ihnen |
| `Items` | `ClassName`, `Count` und `Contents`. Contents gehen ins Item: angehängt, wo sie passen (Magazine, Schalldämpfer, Batterien, Holster), sonst in den Cargo, und können eigene Contents haben |

Kit-Items spawnen pristine, Magazine und Munition voll, Batterien geladen und Flaschen mit Wasser gefüllt. Ein Magazin in den Contents einer Waffe ist mit einem Patrone im Ladegerät geladen.

Die Standardkonfiguration wird nur geschrieben, wenn keine `config.json` existiert. Nach dem Update des Mods löschen Sie die alte Datei (oder fügen Sie die neuen Felder hinzu), um neue Standards zu übernehmen; das Server-Log warnt, wenn die Datei aus einer älteren Version stammt.

Pro-Spieler-Spielzeit und Spawn-Historie werden in `$profile:ChillSpawn/players/` gespeichert.

## Befehle hinzufügen (Erweiterung)

Subclass `CS_ChatCommand` und registrieren Sie es von einem anderen Mod oder diesem hier:

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

## Bekannte Grenzen / zuerst testen

Die Basics funktionieren auf DayZ 1.30 experimental. Prüfen Sie diese auf einem Testserver beim Ändern der Konfiguration oder Aktualisieren des Spiels:

1. **Chat-Hook: funktioniert.** Das Mod liest Chat über `MissionServer.OnEvent` mit `ChatMessageEventTypeID`, sodass Spieler nichts installieren müssen. Das [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) listet `ChatMessageEventTypeID` als Client-seitiges Event auf, aber auf einem DayZ 1.30 experimental-Server empfängt der Server es ebenfalls. Das Event gibt nur den Namen des Absenders an, daher werden Spieler nach Name abgeglichen. Wenn zwei Spieler denselben Namen haben, wird der Befehl ignoriert.
2. **Attachment- und Kit-Klassennamen.** Prüfen Sie, dass jede Klasse in `Attachments` und jedes Kit-Item spawnen (das Server-Log druckt `Could not attach …`, `Could not create …` oder `Could not put …` für alle, die nicht funktionieren), und dass das Auto startet und fährt.
3. **Fahrzeugschutz.** `Invulnerable` stoppt Schaden am Auto und den Teilen, mit denen es gespawned wurde; später angebrachte Teile sind nicht geschützt. `ProtectCrew` stoppt Unfallschaden an den Insassen. Keiner schützt Personen, die das Auto trifft. Beide werden im Speicher gehalten, sodass nach einem Server-Neustart ein Auto, das noch nicht despawned ist, wieder ein normales Auto ist.
4. **Hindernisprüfung.** Verwendet `IsBoxColliding` mit einer Box in der Größe des Autos, 0,3 m über dem Boden gehoben, plus See/Tümpel-Prüfung. Es versucht 10 Richtungen bei 6, 9 und 12 m. Prüfen Sie, dass es Büsche nicht fälschlich als Hindernisse behandelt und dass es Wände und Zäune erkennt. Es prüft keine Steigung, daher kann ein Auto immer noch auf einem steilen Hang landen.
5. **Despawn-Timer leben im Speicher.** Wenn der Server vor Ablauf des Timers neu startet, bleibt das Auto in der Welt als normales persistentes Fahrzeug. Wenn ein Spieler im Auto ist, wenn der Timer abläuft, wird der Despawn jede Minute wiederholt, bis er aussteigt.

## Building

### Linux

```bash
tools/build-linux.sh
```

Dies packt `dist/@ChillSpawn/addons/ChillSpawn.pbo` mit dem PBO-Build-Tool von [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools ist Freeware unter eigener Lizenz und nicht hier enthalten: das Script klont es beim ersten Lauf in `~/.cache/rag-dayz-tools` oder verwendet den Pfad, den Sie übergeben (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) oder in `RAG_TOOLS_DIR` setzen. Benötigt `python3` 3.10+ und `git`.

Das Script führt RaGs Preflight-Checks aus und packt die Scripts, wie sie sind. Es überspringt Binarisieren, `config.cpp`-Konvertierung und Signieren, was Windows DayZ Tools-Programme erfordert. Ein reines Script-Servermod benötigt keines davon. Nichts auf Linux kompiliert Enforce Script, daher treten Script-Fehler nur im Server-`.rpt`-Log beim Start auf.

### Windows

Packen Sie den Ordner `ChillSpawn/` mit dem AddonBuilder von DayZ Tools oder dem RaG PBO Builder in `@ChillSpawn/addons/ChillSpawn.pbo`. Reine Servermods müssen nicht streng signiert werden, aber wenn der Server Signaturen prüft, signieren Sie das PBO und legen Sie die `.bikey` im Ordner `keys/` des Servers ab.

## Installing

1. Kopieren Sie den Ordner `@ChillSpawn` in den Root-Ordner des Servers, neben `DayZServer_x64.exe`.
2. Laden Sie ihn als Servermod:
   - **Normaler Server:** Fügen Sie `-servermod=@ChillSpawn` zur Startzeile hinzu.
   - **CFTools Architect:** Verwenden Sie **Add Local Mod**, wählen Sie `@ChillSpawn` und setzen Sie den Typ auf **Server**.
3. Starten Sie den Server. Er schreibt `profiles/ChillSpawn/config.json` und loggt `[ChillSpawn] Loaded … spawn presets, … kits and … admins.`
4. Fügen Sie Ihre Admins' Steam64-IDs in `Admins` in dieser Datei hinzu und starten Sie neu.

**Aktualisieren:** Stoppen Sie den Server vollständig, bevor Sie `ChillSpawn.pbo` ersetzen. Ein laufender Server sperrt die Datei, und ein schneller Neustart lässt möglicherweise keine Zeit zum Austauschen.

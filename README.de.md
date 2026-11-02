# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · **Deutsch** · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Maschinelle Übersetzung. Bei Abweichungen gilt das [englische Original](README.md).

Ein serverseitiges DayZ-Mod. Spieler geben `!car` im Chat ein, um eine fahrbereite rote Olga 24 sechs Meter nördlich von ihnen zu spawnen: vier Räder, Zündkerze, Kühler, geladene Batterie, Scheinwerfer, Türen, Motorhaube und Kofferraum, mit vollem Kraftstoff, Kühlmittel, Öl und Bremsflüssigkeit. Wenn dieser Punkt blockiert ist oder im Wasser liegt, werden nahegelegene Punkte versucht; wenn keiner frei ist, wird der Spieler angewiesen, sich zu bewegen, und der Versuch zählt nicht gegen sein Limit. Er erhält eine private Nachricht, dass das Fahrzeug innerhalb von 15 Minuten verschwindet. Jeder Spieler kann alle 15 Minuten Spielzeit (Zeit, die er verbunden verbracht hat, keine reale Uhrzeit) ein Olga spawnen. Gespawnete Fahrzeuge nehmen keinen Schaden, und niemand im Inneren wird bei einem Crash verletzt.

Spieler können auch `!survive` einmal alle 2 realen Stunden eingeben, um einen kleinen Survival-Kits zu erhalten. Admins haben keine Limits und erhalten ein `!adminkit`.

Clients benötigen das Mod nicht. Lade es mit `-servermod=@ChillSpawn`. Entwickelt für DayZ 1.30.

> **Status: getestet auf DayZ 1.30 experimental.** `!help`, `!list`, `!car`, `!ada` und `!hatchet` funktionieren auf einem Live-Server ohne Client-Mod. Admins, Kits, `!survive` und der Autoschutz sind neu und noch nicht getestet. Siehe [Bekannte Grenzen / erst testen](#bekannte-grenzen--erst-testen). [docs/PROMPTING.md](docs/PROMPTING.de.md) zeigt, wie es erstellt wurde.

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

Beim ersten Start schreibt das Mod `$profile:ChillSpawn/config.json` (der `-profiles`-Ordner des Servers). Jeder Eintrag in `Presets` und `Kits` wird zu seinem eigenen Chat-Befehl, sodass Serverbesitzer weitere hinzufügen können, ohne den Code zu berühren. Die Standards:

| Befehl | Spawned | Limit |
|---|---|---|
| `!car` | Rote Olga 24, fahrbereit, kein Schaden, Crew geschützt | 1 alle 15 Min. Spielzeit |
| `!ada` | Grüne Ada 4x4, das Gleiche (`OffroadHatchback_Blue` / `_White` für andere Farben, mit passenden `_Blue` / `_White` Tür-, Motorhauben- und Kofferraum-Klassen) | 1 alle 15 Min. Spielzeit |
| `!hatchet` | Eine Axt am Boden 1 m entfernt | 3 alle 60 Min. Spielzeit |
| `!survive` | In das Inventar des Spielers: eine Dose Bacon, eine volle Flasche Wasser, Reinigungstabletten, ein Verband und Tetracyclin | 1 alle 2 reale Stunden |
| `!adminkit` | Nur Admins. Auf dem Boden vor ihnen: Alice-Rucksack, M14 mit 3 vollen 20-Schuss-Magazinen und 3 Dosen .308, M4A1 mit 3 vollen 60-Schuss-Magazinen und 2 Schalldämpfern, voller Panzerweste, MICH-Helm mit Nachtsicht und Batterien, TTsKO-Tarnjacke, Hose und Stiefel, Handschuhe und ein Jagdmesser | keine |

Zwei integrierte Befehle sind immer verfügbar:

| Befehl | Funktion |
|---|---|
| `!list` | Listet alles auf, was der Spieler spawnen kann, eine pro Zeile, mit der Anzahl der verbleibenden und, wenn keine mehr übrig sind, wie lange bis zum nächsten |
| `!help` | Listet die Befehle auf, die der Spieler verwenden kann |

Ein Preset oder Kit darf nicht `list` oder `help` heißen; die integrierten Befehle haben Vorrang, und das Server-Protokoll warnt.

### Admins

Legen Sie die Steam64-IDs der Admins in `Admins`:

```json
"Admins": ["76561198000000000"],
```

Admins haben keine Limits und können Befehle verwenden, die mit `AdminOnly` markiert sind. Andere Spieler sehen Admin-only-Befehle in `!help` oder `!list` nicht, und nichts passiert, wenn sie einen eingeben.

### Presets

Jedes Preset hat sein eigenes Limit (eine Olga und eine Ada alle 15 Minuten sind separate allowances). Um ein Preset hinzuzufügen, kopieren Sie einen Block in `Presets`, geben Sie ihm einen neuen `Command` und starten Sie den Server neu. Felder:

| Feld | Bedeutung |
|---|---|
| `Command` | Chat-Befehl, ohne Präfix (`car` → `!car`) |
| `DisplayName` | Name, der in der Nachricht angezeigt wird |
| `ClassName` | Fahrzeug- oder Item-Klasse zum Spawnen |
| `Attachments` | Klassen, die daran angebracht werden. Wiederholen Sie eine Klasse für jeden Slot, den sie füllt (vier Räder) |
| `DistanceMetres`, `BearingDegrees` | Wo es im Verhältnis zum Spieler spawnet. 0° = Norden, 90° = Osten |
| `DespawnSeconds` | 0 = nie verschwinden |
| `MaxSpawns`, `LimitMinutes` | Jeder Spieler kann es `MaxSpawns`-mal alle `LimitMinutes` Minuten Spielzeit spawnen |
| `AdminOnly` | Nur Admins können es verwenden |
| `SpawnMessage` | `%1` = DisplayName, `%2` = Minuten bis zum Verschwinden |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Nur Fahrzeuge: wie voll jede Flüssigkeit am Anfang ist, 0 bis 1 |
| `Invulnerable` | Es und seine Anbauteile nehmen überhaupt keinen Schaden: Crashes, Kugeln, Explosionen |
| `ProtectCrew` | Nur Fahrzeuge: niemand im Inneren wird bei einem Crash verletzt oder getötet |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Freier Raum (Meter), der am Spawn-Punkt benötigt wird. Der Standard passt zu einer Olga; erhöhen Sie ihn für LKW |

### Kits

Ein Kit ist eine Reihe von Items für einen Befehl. Felder:

| Feld | Bedeutung |
|---|---|
| `Command`, `DisplayName` | Wie bei Presets |
| `MaxSpawns`, `LimitMinutes` | Limit pro Spieler |
| `RealTimeLimit` | Zählen Sie `LimitMinutes` in Echtzeit statt Spielzeit (überlebt Neustarts) |
| `AdminOnly` | Nur Admins können es verwenden |
| `ToInventory` | Legen Sie Items in das Inventar des Spielers, wo Platz ist; der Rest landet auf dem Boden. Andernfalls landet alles auf dem Boden vor ihnen |
| `Items` | `ClassName`, `Count` und `Contents`. Contents gehen in das Item: angebracht, wo sie passen (Magazine, Schalldämpfer, Batterien, Holster), sonst in dessen Ladung, und können eigene Contents haben |

Kit-Items spawnen intakt, Magazine und Munition voll, Batterien geladen und Flaschen mit Wasser gefüllt. Ein Magazin, das im Inhalt einer Waffe aufgeführt ist, ist mit einem Patrone im Lauf geladen.

Die Standardkonfiguration wird nur geschrieben, wenn keine `config.json` existiert. Nach dem Aktualisieren des Mods löschen Sie die alte Datei (oder fügen Sie die neuen Felder hinzu), um neue Standards zu übernehmen; das Server-Protokoll warnt, wenn die Datei von einer älteren Version stammt.

Die Spielzeit und Spawn-Historie pro Spieler werden in `$profile:ChillSpawn/players/` gespeichert.

## Befehle hinzufügen (Erweiterung)

Erstellen Sie eine Unterklasse von `CS_ChatCommand` und registrieren Sie sie aus einem anderen Mod oder in diesem hier:

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

## Bekannte Grenzen / erst testen

Die Grundlagen funktionieren auf DayZ 1.30 experimental. Prüfen Sie diese auf einem Testserver, wenn Sie die Konfiguration ändern oder das Spiel aktualisieren:

1. **Chat-Hook: funktioniert.** Das Mod liest Chat über `MissionServer.OnEvent` mit `ChatMessageEventTypeID`, sodass Spieler nichts installieren müssen. Das [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) listet `ChatMessageEventTypeID` als clientseitiges Event auf, aber auf einem DayZ 1.30 experimental-Server empfängt der Server es ebenfalls. Das Event gibt nur den Namen des Absenders an, daher werden Spieler nach Namen zugeordnet. Wenn zwei Spieler denselben Namen haben, wird der Befehl ignoriert.
2. **Attachment- und Kit-Klassennamen.** Prüfen Sie jede Klasse in `Attachments` und jedes Kit-Item, ob es spawnet (das Server-Protokoll gibt `Could not attach …`, `Could not create …` oder `Could not put …` für diejenigen aus, die nicht funktionieren), und dass das Auto startet und fährt.
3. **Autoschutz (neu).** `Invulnerable` stoppt Schaden am Auto und den Teilen, mit denen es gespawnet wurde; später angebrachte Teile sind nicht geschützt. `ProtectCrew` stoppt Craschschaden an den Personen im Inneren. Keines davon schützt Personen, die das Auto trifft. Beide sind im Speicher gespeichert, daher ist ein Auto nach einem Serverneustart, das noch nicht verschwunden ist, wieder ein normales Auto.
4. **Hindernisprüfung.** Verwendet `IsBoxColliding` mit einer Box in der Größe des Autos, 0,3 m über dem Boden angehoben, plus eine See/Tümpel-Prüfung. Es versucht 10 Richtungen bei 6, 9 und 12 m. Prüfen Sie, dass es Büsche nicht fälschlicherweise als Hindernisse behandelt und dass es Wände und Zäune erkennt. Es prüft keine Steigung, daher kann ein Auto immer noch auf einem steilen Hang landen.
5. **Despawn-Timer leben im Speicher.** Wenn der Server vor Ablauf des Timers eines Autos neu startet, bleibt das Auto in der Welt als normales persistentes Fahrzeug. Wenn ein Spieler im Auto ist, wenn der Timer abläuft, wird das Verschwinden jede Minute wiederholt, bis er aussteigt.

## Building

### Linux

```bash
tools/build-linux.sh
```

Dies packt `dist/@ChillSpawn/addons/ChillSpawn.pbo` mit dem PBO-Builder von [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools ist Freeware unter seiner eigenen Lizenz und nicht hier enthalten: das Script klont es beim ersten Lauf nach `~/.cache/rag-dayz-tools` oder verwendet den Pfad, den Sie übergeben (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) oder in `RAG_TOOLS_DIR` setzen. Benötigt `python3` 3.10+ und `git`.

Das Script führt Rags Preflight-Checks aus und packt die Scripts, wie sie sind. Es überspringt das Binarisieren, die `config.cpp`-Konvertierung und das Signieren, was Windows DayZ Tools Programme benötigt. Ein reines Script-Servermod benötigt keines davon. Auf Linux wird Enforce Script nicht kompiliert, daher erscheinen Script-Fehler nur im `.rpt`-Log des Servers beim Start.

Kopieren Sie `dist/@ChillSpawn` in den Serverordner und fügen Sie `-servermod=@ChillSpawn` zur Startzeile hinzu.

### Windows

Packen Sie den Ordner `ChillSpawn/` mit dem AddonBuilder von DayZ Tools oder dem RaG PBO Builder in `@ChillSpawn/addons/ChillSpawn.pbo`. Server-only-Mods benötigen strikt gesehen keine Signierung, aber wenn der Server Signaturen prüft, signieren Sie die PBO und legen Sie die `.bikey` im `keys/`-Ordner des Servers ab.

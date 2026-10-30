# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · **Deutsch** · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Maschinelle Übersetzung. Bei Abweichungen gilt das [englische Original](README.md).

Ein serverseitiges DayZ-Mod. Spieler geben `!car` im Chat ein, um eine fahrbereite rote Olga 24 sechs Meter nördlich von ihnen zu spawnen: vier Räder, Zündkerze, Kühler, geladene Batterie, Scheinwerfer, Türen, Motorhaube und Kofferraum, mit vollem Kraftstoff-, Kühlwasser-, Öl- und Bremsflüssigkeitsstand. Wenn dieser Ort blockiert ist oder im Wasser liegt, werden nearby gelegene Punkte versucht; wenn keiner frei ist, wird der Spieler aufgefordert zu bewegen und der Versuch zählt nicht gegen ihr Limit. Sie erhalten eine private Nachricht, dass das Fahrzeug nach 15 Minuten wieder verschwindet (despawn). Jeder Spieler kann 3 Fahrzeuge pro Stunde Spielzeit spawnen (Zeit, die mit dem Server verbunden verbracht wurde, keine reale Uhrzeit).

Clients benötigen das Mod nicht. Lade es mit `-servermod=@ChillSpawn`. Entwickelt für DayZ 1.30.

> **Status: ungetestet.** Der Code wurde mit Claude geschrieben und noch nicht kompiliert oder auf einem Server ausgeführt. Siehe [Bekannte Grenzen / zuerst testen](#bekannte-grenzen--zuerst-testen) bevor du es auf einem Live-Server verwendest. [docs/PROMPTING.md](docs/PROMPTING.de.md) zeigt, wie es erstellt wurde.

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

## Konfiguration

Beim ersten Start schreibt das Mod `$profile:ChillSpawn/config.json` (im `-profiles`-Ordner des Servers). Jeder Eintrag in `Presets` wird zu einem eigenen Chatbefehl, sodass Serverbesitzer weitere Fahrzeuge oder Gegenstände hinzufügen können, ohne den Code zu berühren. Die Standardkonfiguration enthält drei:

| Befehl | Spawnt |
|---|---|
| `!car` | Rote Olga 24, fahrbereit |
| `!ada` | Grüne Ada 4x4, fahrbereit (`OffroadHatchback_Blue` / `_White` für andere Farben, mit passenden `_Blue` / `_White` Tür-, Motorhauben- und Kofferraumklassen) |
| `!hatchet` | Eine Axt auf dem Boden in 1 m Entfernung |

Zwei integrierte Befehle sind immer verfügbar:

| Befehl | Funktion |
|---|---|
| `!list` | Listet jeden Preset pro Zeile auf, mit der Anzahl verbleibender Spawns für den Spieler in dieser Spielstunde und, falls keine mehr übrig sind, wie lange bis zum nächsten |
| `!help` | Listet alle Befehle auf |

Ein Preset darf nicht `list` oder `help` heißen; die integrierten Befehle haben Vorrang und das Serverprotokoll warnt.

Jeder Preset hat sein eigenes Limit (3 Olgas und 3 Äxte pro Stunde sind separate Zulassungen). Um einen Preset hinzuzufügen, kopiere einen Block in `Presets`, vergib einen neuen `Command`-Namen und starte den Server neu. Felder:

| Feld | Bedeutung |
|---|---|
| `Command` | Chatbefehl, ohne das Präfix (`car` → `!car`) |
| `DisplayName` | Name, der in der Nachricht angezeigt wird |
| `ClassName` | Fahrzeug- oder Gegenstandsklasse zum Spawnen |
| `Attachments` | Klassen, die daran angebracht werden. Wiederhole eine Klasse für jeden Slot, den sie füllt (vier Räder) |
| `DistanceMetres`, `BearingDegrees` | Position relativ zum Spieler. 0° = Norden, 90° = Osten |
| `DespawnSeconds` | 0 = niemals verschwinden lassen |
| `MaxPerPlaytimeHour` | Spawn-Limit pro Stunde Spielzeit, gezählt pro Preset |
| `SpawnMessage` | `%1` = DisplayName, `%2` = Minuten bis zum Verschwinden |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Nur Fahrzeuge: Wie voll jede Flüssigkeit initial ist, 0 bis 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Benötigter Freiraum (in Metern) am Spawn-Punkt. Der Standard passt zur Olga; erhöhe ihn für LKW |

Die Standardkonfiguration wird nur geschrieben, wenn keine `config.json` existiert. Nach einem Mod-Update lösche die alte Datei (oder füge die neuen Felder hinzu), um neue Standardeinstellungen zu übernehmen.

Die Spielzeit und Spawn-Historie pro Spieler werden in `$profile:ChillSpawn/players/` gespeichert.

## Befehle hinzufügen (Erweiterung)

Erweitere `CS_ChatCommand` und registriere es von einem anderen Mod oder in diesem hier:

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

Noch nicht kompiliert oder getestet. Prüfe diese Punkte zuerst auf einem Testserver:

1. **Chat-Hook (wahrscheinlichster Fehlerpunkt).** Das Mod liest Chat über `MissionServer.OnEvent` mit `ChatMessageEventTypeID`, sodass Spieler nichts installieren müssen. Das [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) listet `ChatMessageEventTypeID` als clientseitiges Event auf, und sein Chat-Befehls-Tutorial verwendet stattdessen ein kleines Client-Mod. Teste dies zuerst: Wenn `!help` keine Antwort gibt, empfängt der Server keine Chat-Events und das Mod benötigt einen clientseitigen Teil. Das Event liefert nur den Absendernamen, daher werden Spieler anhand ihres Namens abgeglichen. Wenn zwei Spieler denselben Namen haben, wird der Befehl ignoriert.
2. **Attachment-Klassennamen.** Prüfe, ob jede Klasse in `Attachments` tatsächlich angebracht wird (das Serverprotokoll druckt `Could not attach …` für alle, die nicht funktionieren), und dass das Fahrzeug startet und fährt.
3. **Hindernisprüfung.** Verwendet `IsBoxColliding` mit einer Box in der Größe des Fahrzeugs, 0,3 m über dem Boden, plus eine See/Tümpel-Prüfung. Es versucht 10 Richtungen in 6, 9 und 12 m Entfernung. Prüfe, dass Büsche nicht fälschlich als Hindernisse behandelt werden und dass Wände und Zäune erkannt werden. Steigungen werden nicht geprüft, daher kann ein Fahrzeug immer noch auf einem steilen Hang landen.
4. **Despawn-Timer leben im Speicher.** Wenn der Server neu startet, bevor der Timer für ein Fahrzeug abgelaufen ist, bleibt das Fahrzeug in der Welt als normales persistentes Fahrzeug erhalten. Wenn ein Spieler im Fahrzeug sitzt, wenn der Timer abläuft, wird das Despawnen jede Minute erneut versucht, bis er aussteigt.

## Building

### Linux

```bash
tools/build-linux.sh
```

Dies packt `dist/@ChillSpawn/addons/ChillSpawn.pbo` mit dem PBO-Builder von [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools ist Freeware unter eigener Lizenz und nicht hier enthalten: Das Skript klont es beim ersten Lauf nach `~/.cache/rag-dayz-tools` oder verwendet den Pfad, den du übergibst (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) oder in `RAG_TOOLS_DIR` festlegst. Benötigt `python3` 3.10+ und `git`.

Das Skript führt Rag's Preflight-Checks aus und packt die Skripte, wie sie sind. Es überspringt das Binarisieren, die `config.cpp`-Konvertierung und das Signieren, was Windows DayZ Tools Programme erfordert. Ein reines Server-Mod benötigt keines davon. Unter Linux wird Enforce Script nicht kompiliert, daher treten Skriptfehler erst im `.rpt`-Log des Servers beim Start auf.

Kopiere `dist/@ChillSpawn` in den Serverordner und füge `-servermod=@ChillSpawn` zur Startzeile hinzu.

### Windows

Packe den Ordner `ChillSpawn/` mit dem AddonBuilder von DayZ Tools oder dem RaG PBO Builder zu `@ChillSpawn/addons/ChillSpawn.pbo`. Server-only-Mods benötigen streng genommen keine Signierung, aber wenn der Server Signaturen überprüft, signiere die PBO und lege die `.bikey` in den `keys/`-Ordner des Servers.

# Wie ChillSpawn mit Claude entstand

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · **Deutsch** · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Maschinelle Übersetzung. Bei Abweichungen gilt das [englische Original](PROMPTING.md).

ChillSpawn wurde in einer einzigen Claude-Code-Sitzung (Claude Opus 5.5) auf einem Linux-Rechner ohne DayZ-Installation erstellt. Claude hat jede Datei geschrieben. Die Person, die die Sitzung leitete, schrieb die Anforderungen, bat um Änderungen und verglich das Ergebnis mit zwei lokalen Modellen. Während der Sitzung wurde kein Code kompiliert oder im Spiel ausgeführt, daher ist der Code als starker erster Entwurf zu betrachten, der noch einen Testserver benötigt.

Diese Seite führt die Anfragen der Reihe nach durch (umformuliert und bereinigt, nicht wörtlich zitiert), was jede einzelne veränderte und was daraus gelernt wurde. Sie endet mit Prompts, die Sie wiederverwenden können, um ein ähnliches Mod zu erstellen.

## Die Prompts, in Reihenfolge

### 1. Die erste Anfrage

> Schreiben Sie ein serverseitiges DayZ-Mod, das es einem Spieler ermöglicht, ein Auto über einen Chat-Befehl zu spawnen:
>
> - Das Auto ist der rote Olga, mit vier Rädern, einer Zündkerze und einem Kühler ausgestattet.
> - Es wird 6 Meter nördlich des Spielers gespawnt.
> - Der Spieler erhält eine private Nachricht, dass das Auto innerhalb von 15 Minuten entfernt wird (despawn).
> - Jeder Spieler kann 3 Autos pro Stunde Spielzeit spawnen.
> - Fügen Sie ein Framework hinzu, damit das Mod später erweitert werden kann.
> - Eine JSON-Konfiguration ermöglicht es dem Serverbesitzer zu wählen, was gespawnt werden soll: ein Auto samt seiner Attachments oder ein Item.

Claude produzierte das gesamte Mod: `config.cpp`, die Aufteilung der Skripte in 3_Game/4_World/5_Mission, eine JSON-Konfiguration, die beim ersten Start in `$profile:` geschrieben wird, eine Befehlsregistrierung (das „Framework“), die Spielzeit-Verfolgung pro Spieler und eine README. Es markierte auch Unsicherheiten anstatt sie zu verbergen:

- dass der Server Chat-Nachrichten über `MissionServer.OnEvent` mit `ChatMessageEventTypeID` empfängt und nur den Namen des Absenders erhält;
- dass ein Olga nur mit Rädern, Zündkerze und Kühler **nicht fährt** (keine Batterie oder Flüssigkeiten);
- dass Despawn-Timer bei einem Neustart verloren gehen;
- dass es keine Hindernisprüfung gab.

**Lektion:** Eine nummerierte Liste konkreter Anforderungen (exaktes Fahrzeug, Entfernung, Richtung, Limits) liefert in einem Schritt eine vollständige Antwort. Bitten Sie Claude, seine Annahmen und alles aufzulisten, was es nicht verifizieren konnte.

### 2. Machen Sie es fahrbar und prüfen Sie auf Hindernisse

> Fügen Sie hinzu, was das Auto noch braucht, damit es startklar gespawnt wird. Prüfen Sie auch vor dem Spawnen auf Hindernisse.

Es wurden Batterie (geladen), Scheinwerfer, Türen, Motorhaube und Kofferraum hinzugefügt sowie Kraftstoff, Kühlmittel, Öl und Bremsflüssigkeit aufgefüllt (alles konfigurierbar). Die Hindernisprüfung nutzt `IsBoxColliding` mit einer auto-großen Box plus einer See/Tümpel-Prüfung. Es werden 10 Richtungen in drei Entfernungen versucht. Wenn nichts frei ist, wird der Spieler informiert und der Versuch zählt nicht gegen sein Limit.

**Lektion:** Follow-ups, die auf Clauses eigene Liste der „bekannten Grenzen“ eingehen, sind kostengünstig und zielgerichtet.

### 3. Mehr als ein spawnbares Objekt

> Zeigen Sie im Beispielkonfig mehrere spawnbare Objekte, zum Beispiel eine Axt und den Ada 4x4.

Die Konfiguration unterstützte dies bereits (jede Vorlage ist ihr eigener Chat-Befehl). Claude fügte `!ada` und `!hatchet` Vorlagen sowohl in die Standards als auch in `config.example.json` ein und dokumentierte, wie man weitere hinzufügt.

### 4. Vergleich mit lokalen Modellen

> Lassen Sie mein lokales Qwen-Modell dasselbe Mod schreiben, dann vergleichen Sie seine Version mit Ihrer: Wie unterschiedlich sind sie und enthält entweder davon Fehler?

Claude sendete die gleichen kombinierten Anforderungen an ein lokales Qwen-Modell und überprüfte später einen zweiten Qwen-Versuch, den der Benutzer bereitstellte. Keines davon hätte funktioniert:

| | Lokales Qwen #1 | Lokales Qwen #2 |
|---|---|---|
| Echte DayZ API | Meist erfunden (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Meist echt |
| Würde laden | Nein: keine Skriptmodule, hookt `MissionServer` nie | Nein: `config.cpp` hat keine `CfgMods` |
| Würde kompilieren | Nein | Nein: Ternäroperator, C++-artige Iteratoren, `ToLower()` verwendet, als würde es einen String zurückgeben |
| „Norden“ | 6 m in der Luft | 6 m südlich |
| Rate Limit | Im Speicher, feste Blöcke | Server-Uptime, verloren bei Neustart |

**Lektion:** Für Enforce Script schreiben allgemeine Modelle selbstbewusst C++/Java-Isms, die DayZ nicht unterstützt. Holen Sie sich eine zweite Meinung, indem Sie ein Modell den Code eines anderen Modells anhand einer Checkliste (laden, kompilieren, Verhalten) überprüfen lassen, anstatt einem von beiden blind zu vertrauen.

### 5. Eine gute Idee übernehmen

> Nehmen Sie die Idee der Fahrzeug-/Item-Liste aus der zweiten Qwen-Version in ChillSpawn auf.

Qwen #2 hatte einen `/vspawn list` Subcommand. Claude fügte `!list` (jede Vorlage mit der Anzahl der verbleibenden Spawns für den Spieler in dieser Stunde) und `!help` als integrierte Befehle in ChillSpawns Framework ein.

### 6. Veröffentlichen

> Committen Sie dies in mein öffentliches GitHub-Repo (MIT-Lizenz; ersetzen Sie die README, wenn Sie möchten). Stellen Sie sicher, dass keine Zugangsdaten enthalten sind. Fügen Sie eine Dokumentation hinzu, die zeigt, wie das Prompting erfolgte und welche Prompts jemand anderes verwenden könnte, um ein ähnliches Mod zu erstellen.

Das führte zu diesem Repository und dieser Seite.

### 7. Prüfung gegen das 1.30 Update

> Prüfen Sie die 1.30 Patch Notes und Bohemias Feedback-Tracker auf alles, was unsere Logik brechen könnte.

In den Notes wurde nichts erwähnt, das die APIs betrifft, die das Mod nutzt. 1.30 brachte einen strikteren Skript-Compiler, sodass zwei Konstrukte, die er möglicherweise ablehnen würde, geändert wurden (eine Methode ohne Körper deklariert und eine Float-zu-Int-Konvertierung).

### 8. Erstellen unter Linux

> Prüfen Sie, ob RaG DayZ Tools dies auf Linux erstellen können.

RaG DayZ Tools ist Windows-first, aber sein PBO-Build-Tool ist reines Python und läuft auf Linux. `tools/build-linux.sh` nutzt es, um das Mod zu packen. Binarisieren, Konvertieren der Config und Signieren benötigen immer noch Windows-Programme, und ein Skript-nur Server-Mod braucht diese nicht.

## Wiederverwendbare Prompts

### Starten eines neuen serverseitigen Mods

Kopieren Sie dies, füllen Sie die Klammern aus und fügen Sie es in Claude ein (Claude Code funktioniert am besten, da er die Dateien für Sie schreibt):

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

Die Zeile „Enforce Script hat kein…“ stammt direkt aus den Fehlern, die die lokalen Modelle gemacht haben. Das Beibehalten kostet nichts und verhindert die häufigsten Fehler.

### Follow-ups, die gut funktioniert haben

- "Machen Sie das Fahrzeug vollständig fahrbar gespawnt: jeden Teil, den es braucht, geladene Batterie und volle Flüssigkeiten."
- "Fügen Sie eine Hindernisprüfung vor dem Spawnen hinzu. Wenn der Ort blockiert oder im Wasser ist, versuchen Sie nahegelegene Orte; wenn keiner frei ist, teilen Sie es dem Spieler mit und zählen Sie es nicht gegen sein Limit."
- "Fügen Sie einen `!list` Befehl hinzu, der jede spawnbare Vorlage zeigt und wie viele dem Spieler noch bleiben."
- "Überprüfen Sie diesen Code [einfügen oder anhängen] auf Dinge, die in DayZ nicht kompilieren oder laden würden, sowie auf Logikfehler gegenüber diesen Anforderungen: [Liste]."
- "Schreiben Sie einen Testplan, dem ich auf einem lokalen Server folgen kann, um jede Anforderung zu prüfen."

### Tipps

- **Seien Sie genau bei Spielobjekten.** „Roter Olga“ wurde zu `CivilianSedan_Wine`. Wenn Sie die Klassennamen kennen, geben Sie sie an; wenn nicht, bitten Sie Claude, die von ihm verwendeten aufzulisten, damit Sie diese im `types.xml` des Servers oder mit einem Admin-Tool prüfen können.
- **Sagen Sie, was „pro Stunde“ bedeutet.** „Pro Spielzeitstunde“ und „pro Server-Stundenzeit“ sind unterschiedliche Funktionen, und nur eine davon überlebt einen Reconnect.
- **Bitten Sie um einen Abschnitt „bekannte Grenzen“** und behandeln Sie ihn als Ihre Testliste.
- **Testen Sie auf einem lokalen Server, bevor Sie live gehen.** Beobachten Sie das Server `.rpt` Log auf Skriptfehler und Zeilen wie `[ChillSpawn] Could not attach …`.
- **Unter Windows mit installiertem DayZ Tools** fügt das [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) Plugin für Claude Code DayZ-spezifische Agents und Fähigkeiten hinzu, die PBOs erstellen und einen lokalen Testserver starten, was die Lücke „ungetestet“ schließt.

# Wie ChillSpawn mit Claude entstand

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · **Deutsch** · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Maschinelle Übersetzung. Bei Abweichungen gilt das [englische Original](PROMPTING.md).

ChillSpawn wurde in einer einzigen Claude-Code-Sitzung (Claude Opus 5.5) auf einem Linux-Rechner ohne DayZ-Installation geschrieben. Claude hat jede Datei erstellt. Die Person, die das Projekt steuerte, verfasste die Anforderungen, bat um Änderungen und verglich das Ergebnis mit zwei lokalen Modellen. Die erste Version wurde geschrieben, ohne etwas zu kompilieren oder auszuführen. Später in derselben Sitzung wurde ein Windows-Dedicated-Server mit DayZ 1.30 Experimental eingerichtet, Claude greifte über SSH darauf zu, und das Mod wurde bereitgestellt, im Spiel getestet und dort erweitert.

Diese Seite führt die Anfragen der Reihe nach durch (umformuliert und bereinigt, nicht wortwörtlich zitiert), was jede Änderung bewirkte und was dabei gelernt wurde. Sie endet mit Prompts, die Sie wiederverwenden können, um ein ähnliches Mod zu erstellen.

## Die Prompts in der Reihenfolge

### 1. Die erste Anfrage

> Schreiben Sie ein serverseitiges DayZ-Mod, das es einem Spieler erlaubt, ein Auto über einen Chat-Befehl zu spawnen:
>
> - Das Auto ist die rote Olga, mit vier Rädern, einer Zündkerze und einem Kühler ausgestattet.
> - Es wird 6 Meter nördlich des Spielers gespawnt.
> - Der Spieler erhält eine private Nachricht, dass das Auto innerhalb von 15 Minuten verschwindet (despawn).
> - Jeder Spieler kann 3 Autos pro Stunde verbrachter Spielzeit spawnen.
> - Fügen Sie ein Framework hinzu, damit das Mod später erweitert werden kann.
> - Eine JSON-Konfiguration ermöglicht es dem Serverbesitzer zu wählen, was gespawnt wird: ein Auto samt seiner Attachments oder ein Item.

Claude produzierte das gesamte Mod: `config.cpp`, die Aufteilung der Skripte in 3_Game/4_World/5_Mission, eine JSON-Konfiguration, die beim ersten Start nach `$profile:` geschrieben wird, eine Befehlsregistrierung (das „Framework“), die Spielzeitverfolgung pro Spieler und eine README. Es markierte auch Unsicherheiten, anstatt sie zu verschweigen:

- dass der Server Chats über `MissionServer.OnEvent` mit `ChatMessageEventTypeID` empfängt und nur den Namen des Absenders erhält;
- dass eine Olga, die nur aus Rädern, Zündkerze und Kühler besteht, **nicht fährt** (keine Batterie oder Flüssigkeiten);
- dass Despawn-Timer bei einem Neustart verloren gehen;
- dass es keine Hindernisprüfung gab.

**Lektion:** Eine nummerierte Liste konkreter Anforderungen (genaues Fahrzeug, Entfernung, Richtung, Limits) liefert in einem Schritt eine vollständige Antwort. Bitten Sie Claude, seine Annahmen und alles aufzulisten, was es nicht verifizieren konnte.

### 2. Machen Sie es fahrbar und prüfen Sie auf Hindernisse

> Fügen Sie alles hinzu, was das Auto braucht, damit es startklar zum Fahren gespawnt wird. Prüfen Sie auch vor dem Spawnen auf Hindernisse.

Es wurden Batterie (geladen), Scheinwerfer, Türen, Motorhaube und Kofferraum hinzugefügt sowie Kraftstoff, Kühlflüssigkeit, Öl und Bremsflüssigkeit aufgefüllt (alles konfigurierbar). Die Hindernisprüfung verwendet `IsBoxColliding` mit einer auto-großen Box plus einer See-/Teich-Prüfung. Es versucht 10 Richtungen in drei Entfernungen. Wenn nichts frei ist, wird der Spieler informiert und der Versuch zählt nicht gegen sein Limit.

**Lektion:** Follow-ups, die auf Claudes eigene Liste der „bekannten Grenzen“ eingehen, sind kostengünstig und gezielt.

### 3. Mehr als eine spawnbare Sache

> Lassen Sie die Beispielkonfiguration mehrere spawnbare Dinge anzeigen, zum Beispiel ein Beil und den Ada 4x4.

Die Konfiguration unterstützte dies bereits (jedes Preset ist ein eigener Chat-Befehl). Claude fügte `!ada` und `!hatchet` Presets sowohl zu den Standards als auch zu `config.example.json` hinzu und dokumentierte, wie man weitere hinzufügt.

### 4. Vergleich mit lokalen Modellen

> Lassen Sie mein lokales Qwen-Modell dasselbe Mod schreiben, vergleichen Sie dann seine Version mit Ihrer: Wie unterschiedlich sind sie und hat eines Fehler?

Claude sendete die gleichen kombinierten Anforderungen an ein lokales Qwen-Modell und überprüfte später einen zweiten Qwen-Versuch, den der Benutzer bereitstellte. Keines davon hätte funktioniert:

| | Lokales Qwen #1 | Lokales Qwen #2 |
|---|---|---|
| Echte DayZ-API | Meist erfunden (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Meist echt |
| Würde laden | Nein: keine Skriptmodule, hookt `MissionServer` nie | Nein: `config.cpp` hat kein `CfgMods` |
| Würde kompilieren | Nein | Nein: Ternäroperator, C++-Stil Iteratoren, `ToLower()` verwendet als würde es einen String zurückgeben |
| „Norden“ | 6 m in der Luft | 6 m südlich |
| Rate Limit | Im Speicher, feste Blöcke | Server-Uptime, verloren bei Neustart |

**Lektion:** Für Enforce Script schreiben allgemeine Modelle selbstbewusst C++/Java-Isms, die DayZ nicht unterstützt. Holen Sie sich eine zweite Meinung, indem Sie ein Modell den Code eines anderen gegen eine Checkliste (laden, kompilieren, Verhalten) prüfen lassen, statt auf eines von beiden zu vertrauen.

### 5. Eine gute Idee übernehmen

> Nehmen Sie die Idee des zweiten Qwen-Versions einer Fahrzeug-/Item-Liste in ChillSpawn auf.

Qwen #2 hatte einen `/vspawn list` Unterbefehl. Claude fügte `!list` (jedes Preset mit der Anzahl der verbleibenden Spawns, die der Spieler diese Stunde hat) und `!help` als integrierte Befehle im ChillSpawn-Framework hinzu.

### 6. Veröffentlichen

> Committen Sie dies in mein öffentliches GitHub-Repo (MIT-Lizenz; ersetzen Sie die README nach Belieben). Stellen Sie sicher, dass keine Zugangsdaten eingetragen werden. Fügen Sie eine Dokumentation hinzu, die zeigt, wie das Prompting erfolgte und welche Prompts jemand anderes verwenden könnte, um ein ähnliches Mod zu erstellen.

Das produzierte dieses Repository und diese Seite.

### 7. Prüfung gegen das 1.30-Update

> Prüfen Sie die 1.30 Patch Notes und Bohemias Feedback-Tracker auf alles, was unsere Logik brechen könnte.

Nichts in den Notes berührte die APIs, die das Mod verwendet. 1.30 brachte einen strengeren Skriptcompiler, daher wurden zwei Konstrukte geändert, die er ablehnen könnte (eine Methode ohne Körper deklariert und eine Float-zu-Int-Konvertierung).

### 8. Bauen auf Linux

> Prüfen Sie, ob RaG DayZ Tools dies auf Linux bauen können.

RaG DayZ Tools ist Windows-first, aber sein PBO-Builder ist reines Python und läuft auf Linux. `tools/build-linux.sh` verwendet es, um das Mod zu packen. Binarisieren, Konvertieren der Configs und Signieren benötigen immer noch Windows-Programme, und ein Server-mod nur für Skripte benötigt diese nicht.

### 9. Testen auf einem echten Server

> Ich habe einen Windows-Dedicated-Server mit CFTools Architect eingerichtet und SSH für Sie geöffnet. Stellen Sie das Mod dort bereit und prüfen Sie, ob es funktioniert.

Der Server lief DayZ 1.30 Experimental, verwaltet von CFTools Architect. Über SSH tat Claude:

- kopierte die eigene `scripts.pbo` des Spiels vom Server, entpackte sie und prüfte jede Funktion, die das Mod aufruft, gegen die echten 1.30 Experimental-Skripte. Alle existierten mit passenden Parametern;
- baute die PBO auf Linux, kopierte `@ChillSpawn` in den Root-Ordner des Servers und bat den Besitzer, sie in Architect mit **Add Local Mod**, Typ **Server** hinzuzufügen;
- las die Server-`.rpt` und Skriptlogs nach jedem Neustart durch, um zu bestätigen, dass die Skripte kompilierten und das Mod lud.

`!help` und `!car` funktionierten beim ersten Mal. Der Chat-Hook, der am wahrscheinlichsten fehlschlägt, funktioniert auf einem Server ohne Client-Mod.

**Lektionen:**

- **Geben Sie Claude die echten Spielskripte.** Das Entpacken von `scripts.pbo` aus dem installierten Spiel verwandelt „diese API existiert wahrscheinlich“ in eine schnelle Prüfung und klärt Fragen, die Wikis falsch beantworten.
- **Tauschen Sie die PBO bei vollständig gestopptem Server.** Ein laufender Server sperrt die Mod-Datei, und der Neustart von Architect ist zu schnell, um dies zu erkennen.

### 10. Admins, Limits, geschützte Autos und Kits

> Lassen Sie Admins so viel spawnen, wie sie wollen. Beschränken Sie Spieler auf eines von jedem Auto alle 15 Minuten. Können gespawnte Autos schadenunempfindlich gemacht werden, oder stoppen Unfälle das Verletzen der Insassen, ohne ein Client-Mod? Fügen Sie einen Admin-only `!adminkit` mit [einer Liste von Waffen, Rüstung und Kleidung] hinzu, sowie ein `!survive`-Kit, das Spieler einmal alle 2 echten Stunden mit [Essen, Wasser, medizinischen Items] verwenden können.

Claude las zuerst den 1.30 Auto-Code. Craschschäden an der Crew werden durch serverseitige Skripte angewendet (`CarScript.DamageCrew`), daher kann ein Server-mod dies ausschalten. Schäden am Auto selbst werden mit `SetAllowDamage(false)` ausgeschaltet. Dann fügte es hinzu:

- eine `Admins`-Liste von Steam64-IDs, ohne Limits und Zugriff auf Admin-only-Befehle, die andere Spieler nicht sehen können;
- Limits als `MaxSpawns` pro `LimitMinutes`, gezählt in Spielzeit oder in Echtzeit für Kits, die dies verlangen;
- Optionen `Invulnerable` und `ProtectCrew` für Presets;
- Kits: verschachtelte Items (eine Waffe mit Magazin und Schalldämpfer, ein Rucksack mit Inhalt), volle Magazine und Wasser, geliefert ins Inventar oder auf den Boden.

Alles funktionierte im Spiel beim ersten Bereitstellen.

**Lektion:** Fragen Sie „kann das serverseitig gemacht werden?“, bevor Sie nach dem Feature fragen, erhalten Sie eine Antwort, die auf dem tatsächlichen Spielcode basiert, sowie dessen Grenzen: hier, dass Personen, die das Auto trifft, immer noch verletzt werden, und der Schutz bei einem Neustart verloren geht.

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

Die Zeile „Enforce Script hat kein…“ stammt direkt aus den Fehlern, die die lokalen Modelle machten. Das Beibehalten kostet nichts und verhindert die häufigsten Fehler.

### Follow-ups, die gut funktionierten

- „Machen Sie das Fahrzeug startklar zum Fahren spawnen: jeden Teil, den es braucht, volle Batterie und alle Flüssigkeiten voll.“
- „Fügen Sie eine Hindernisprüfung vor dem Spawnen hinzu. Wenn der Ort blockiert oder im Wasser ist, versuchen Sie nahegelegene Orte; wenn keiner frei ist, informieren Sie den Spieler und zählen Sie es nicht gegen sein Limit.“
- „Fügen Sie einen `!list`-Befehl hinzu, der jedes spawnbare Preset und wie viele der Spieler noch hat, anzeigt.“
- „Überprüfen Sie diesen Code [einfügen oder anhängen] auf Dinge, die in DayZ nicht kompilieren oder laden werden, und auf Logikfehler gegen diese Anforderungen: [Liste].“
- „Schreiben Sie einen Testplan, dem ich auf einem lokalen Server folgen kann, um jede Anforderung zu prüfen.“
- „Hier ist SSH-Zugang zu meinem Server. Prüfen Sie jede API, die das Mod verwendet, gegen die eigenen Skripte des Spiels, bevor Sie bereitstellen.“
- „Kann [Feature] nur serverseitig gemacht werden? Prüfen Sie die Skripte des Spiels und sagen Sie mir, was es nicht abdecken kann.“

### Tipps

- **Seien Sie genau bei Spielobjekten.** „Rote Olga“ wurde zu `CivilianSedan_Wine`. Wenn Sie die Klassennamen kennen, geben Sie sie an; wenn nicht, bitten Sie Claude, die von ihm verwendeten aufzulisten, damit Sie diese in der serverseitigen `types.xml` oder mit einem Admin-Tool prüfen können.
- **Sagen Sie, was „pro Stunde“ bedeutet.** „Pro Stunde Spielzeit“ und „pro Stunde Serverzeit“ sind unterschiedliche Features, und nur eines überlebt einen Reconnect.
- **Bitten Sie um einen Abschnitt „bekannte Grenzen“** und behandeln Sie ihn als Ihre Testliste.
- **Testen Sie auf einem lokalen Server, bevor Sie live gehen.** Beobachten Sie das Server-`.rpt`-Log auf Skriptfehler und Zeilen wie `[ChillSpawn] Could not attach …`.
- **Geben Sie Claude SSH-Zugang zu einem Testserver**, wenn Sie können. Er kann dann bereitstellen, die Logs lesen und die eigenen Skripte des Spiels prüfen, was Vermutungen in Überprüfungen verwandelt.
- **Unter Windows mit installiertem DayZ Tools** fügt das [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z)-Plugin für Claude Code DayZ-spezifische Agents und Fähigkeiten hinzu, die PBOs bauen und einen lokalen Testserver starten, was die Lücke „ungetestet“ schließt.

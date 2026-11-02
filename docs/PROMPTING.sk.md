# Ako vznikol ChillSpawn s pomocou Claude

[English](PROMPTING.md) · **Slovenčina** · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Strojový preklad. Pri rozdieloch platí [anglický originál](PROMPTING.md).

ChillSpawn bol napísaný v jednej relácii Claude Code (Claude Opus 5.5) na Linuxe bez nainštalovaného DayZ. Claude napísal každý súbor. Človek, ktorý ho riadil, formuloval požiadavky, žiadal zmeny a porovnával výsledok s dvoma lokálnymi modelmi. Prvá verzia bola napísaná bez kompilácie alebo spúšťania čohokoľvek. Neskôr v tej istej relácii bol nastavený Windows dedicated server s DayZ 1.30 experimental, Claude sa k nemu pripojil cez SSH a modbol nasadený, otestovaný vo hre a následne rozšírený.

Táto stránka prechádza požiadavkami v poradí (preformulované a upravené, nie doslovné citácie), čo každá zmenila a čo sa naučili. Končí výzvami, ktoré môžete znova použiť na vytvorenie podobného modu.

## Výzvy v poradí

### 1. Prvá požiadavka

> Napíš serverový DayZ mod, ktorý umožní hráčovi spawnovať auto pomocou chatovej príkazu:
>
> - Auto je červená Olga, so štyrmi kolesami, sviečkou a chladičom.
> - Spawnuje sa 6 metrov severne od hráča.
> - Hráč dostane súkromnú správu, že auto sa zruší do 15 minút.
> - Každý hráč môže spawnovať 3 autá za hodinu času stráveného vo hre.
> - Zapoj rámec (framework), aby mohol byť mod neskôr rozšírený.
> - JSON konfigurácia umožňuje majiteľovi servera vybrať, čo sa má spawnovať: auto plus jeho príslušenstvo, alebo predmet.

Claude vyprodukoval celý mod: `config.cpp`, rozdelenie skriptov 3_Game/4_World/5_Mission, JSON konfiguráciu napísanú do `$profile:` pri prvom spustení, registráciu príkazov (tzv. "framework"), sledovanie času hry uložené pre každého hráča a README. Tiež upozornil na to, si nie je istý, namiesto toho, aby to skryl:

- že server prijíma chat cez `MissionServer.OnEvent` s `ChatMessageEventTypeID` a dostáva iba meno odosielateľa;
- že Olga len s kolesami, sviečkou a chladičom **nejde** (žiadna batéria ani tekutiny);
- že časovače zrušenia sa pri reštarte stratia;
- že neexistuje kontrola prekážok.

**Lekcia:** číslovaný zoznam konkrétnych požiadaviek (presné vozidlo, vzdialenosť, smer, limity) prináša kompletnú odpoveď na jeden pokus. Požiadaj Claudea, aby vypísal svoje predpoklady a všetko, čo nemohol overiť.

### 2. Urob ho jazdným a skontroluj prekážky

> Pridaj všetko ostatné, čo auto potrebuje, aby sa spawnulo pripravené na jazdu. Tiež skontroluj prekážky pred spawnom.

Pridaná batéria (nabitá), svetlomety, dvere, kapota a kufor, naplnené palivo, chladivo, olej a brzdová kvapalina (všetko konfigurovateľné). Kontrola prekážok používa `IsBoxColliding` s boxom veľkosti auta plus kontrolu mora/rybníka. Skúša 10 smerov na troch vzdialenostiach. Ak nie je nič jasné, hráčovi sa to oznámi a pokus sa nezapočítava do jeho limitu.

**Lekcia:** nadväzujúce požiadavky, ktoré reagujú na vlastný zoznam "známych obmedzení" Claudea, sú lacné a cielené.

### 3. Viac ako jedna spawnovateľná vec

> Urob v ukážkovej konfigurácii niekoľko spawnovateľných vecí, napríklad sekeru a Ada 4x4.

Konfigurácia to už podporovala (každý preset je vlastný chatový príkaz). Claude pridal predvolené `!ada` a `!hatchet` do oboch štandardných hodnôt aj do `config.example.json` a zdokumentoval, ako pridať ďalšie.

### 4. Porovnanie s lokálnymi modelmi

> Nechaj môj lokálny model Qwen napísať ten istý mod, potom porovnajte jeho verziu s mojou: ako sa líšia a má niektorá chyby?

Claude poslal rovnaké kombinované požiadavky lokálnemu modelu Qwen a neskôr skontroloval druhý pokus Qwen, ktorý používateľ dodal. Žiadny z nich by nefungoval:

| | Lokálny Qwen #1 | Lokálny Qwen #2 |
|---|---|---|
| Skutočné DayZ API | Väčšinou vymyslené (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Väčšinou reálne |
| Načítalo by sa | Nie: žiadne skriptové moduly, nikdy neviaže `MissionServer` | Nie: `config.cpp` nemá `CfgMods` |
| Skompilovalo by sa | Nie | Nie: ternárny operátor, C++-štýl iterátory, `ToLower()` používané tak, ako keby vracalo reťazec |
| "Sever" | 6 m nad hlavou | 6 m južne |
| Limit rýchlosti | V pamäti, pevné bloky | Uptime servera, stratí sa pri reštarte |

**Lekcia:** pre Enforce Script všeobecné modely sebavedomo píšu C++/Java-izmy, ktoré DayZ nepodporuje. Získaj druhý názor tým, že jeden model skontroluje kód iného podľa kontrolného zoznamu (načítanie, kompilácia, správanie), nie tým, že dôveruješ ktorémukoľvek z nich.

### 5. Prevzatie dobrej myšlienky

> Prenes do ChillSpawn nápad druhej verzie Qwen na zoznam vozidiel/predmetov.

Qwen #2 malo podpríkaz `/vspawn list`. Claude pridal `!list` (každý preset s tým, koľko spawnov hráčovi zostáva túto hodinu) a `!help` ako zabudované príkazy v rámci ChillSpawn.

### 6. Zverejnenie

> Ulož toto do môjho verejného GitHub repozitára (licencia MIT; nahraď README, ak chceš). Uisti sa, že sa nedostanú žiadne poverenia. Pridaj dokumentáciu ukazujúcu, ako bolo promptované a aké výzvy by niekto iný mohol použiť na vytvorenie podobného modu.

To vyprodukovalo tento repozitár a túto stránku.

### 7. Kontrola proti aktualizácii 1.30

> Skontroluj poznámky k patchu 1.30 a Bohemia feedback tracker pre všetko, čo by mohlo prerušiť našu logiku.

Nič v poznámkach sa nedotýkalo API, ktoré mod používa. 1.30 priniesol prísnejší skriptový kompilátor, takže boli zmenené dve konštrukcie, ktoré by mohol zamietnuť (metóda deklarovaná bez tela a konverzia float na int).

### 8. Buildovanie na Linuxe

> Skontroluj, či RaG DayZ Tools dokážu toto zbuildovať na Linuxe.

RaG DayZ Tools sú primárne pre Windows, ale ich PBO builder je čisté Python a beží na Linuxe. `tools/build-linux.sh` ho používa na balenie modu. Binárizácia, konverzia konfigurácií a podpis stále potrebujú Windows programy, ale serverový mod len so skriptmi ich nepotrebuje.

### 9. Testovanie na reálnom serveri

> Nastavil som Windows dedicated server s CFTools Architect a otvoril SSH pre teba. Nasad sem mod a skontroluj, či funguje.

Server bežal na DayZ 1.30 experimental, spravovanom CFTools Architect.cez SSH:

- skopíroval `scripts.pbo` hry zo servera, rozbalil ho a skontroloval každú funkciu, ktorú mod volá, proti reálnym skríptom 1.30 experimental. Všetky existovali so zhodnými parametrami;
- zbuildoval PBO na Linuxe, skopíroval `@ChillSpawn` do koreňového priečinka servera a požiadal majiteľa, aby ho pridal v Architect s **Add Local Mod**, typ **Server**;
- čítal `.rpt` a skriptové logy servera po každom reštarte, aby potvrdil, že skripty sa skompilovali a mod sa načítal.

`!help` a `!car` fungovali na prvý pokus. Chat hook, časť, ktorá najpravdepodobnejšie zlyhá, funguje na serveri bez klientského modu.

**Lekcie:**

- **Daj Claudeovi reálne skripty hry.** Rozbalenie `scripts.pbo` z nainštalovanej hry mení "toto API pravdepodobne existuje" na rýchlu kontrolu a vyrieši otázky, ktoré wiky nesprávne informujú.
- **Vymeň PBO so zastaveným serverom.** Bežiaci server zamkne súbor modu a reštart Architect je príliš rýchly na to, aby to zachytil.

### 10. Admini, limity, chránené autá a sady

> Nech admini spawnujú koľko chcú. Omez hráčov na jedno auto každých 15 minút. Dajú sa spawnované autá urobiť imúnne voči poškodeniu alebo nehody prestanú zraniať ľudí vo vnútri, bez klientského modu? Pridaj admin-only `!adminkit` so [zoznamom zbraní, brnenia a oblečenia] a `!survive` sadu, ktorú môžu hráči použiť raz za 2 reálne hodiny so [jedlom, vodou, lekárskymi predmetmi].

Claude najprv preštudoval kód auta v 1.30. Zranenie posádky pri nehode sa aplikuje serverovým skriptom (`CarScript.DamageCrew`), takže serverový mod to môže vypnúť. Poškodenie samotného auta sa vypína pomocou `SetAllowDamage(false)`. Potom pridal:

- zoznam `Admins` Steam64 ID, bez limitov a prístup k admin-only príkazom, ktoré ostatní hráči nevidia;
- limity ako `MaxSpawns` na `LimitMinutes`, počítané v čase hry, alebo v reálnom čase pre sady, ktoré to vyžadujú;
- možnosti `Invulnerable` a `ProtectCrew` pre presety;
- sady: vnorené predmety (zbraň s zásobníkom a tlmičom, batoh s obsahom), plné zásobníky a voda, doručené do inventára alebo na zem.

Všetko fungovalo vo hre pri prvom nasadení.

**Lekcia:** pýtať sa "da sa toto urobiť len serverovo?" pred tým, než požiadaš o funkciu, prináša odpoveď založenú na skutočnom kóde hry, plus jej obmedzeniach: tu, že ľudia, do ktorých auto narazí, sú stále zranení, a ochrana sa stratí pri reštarte.

## Výzvy, ktoré môžete znova použiť

### Začatie nového serverového modu

Skopíruj toto, vyplň zátvorky a vlož do Claude (Claude Code funguje najlepšie, keďže píše súbory za teba):

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

Riadok "Enforce Script has no…" pochádza priamo z chýb, ktoré urobili lokálne modely. Nechanie ho tam nič nestojí a zabraňuje najbežnejším chybám.

### Nadväzujúce výzvy, ktoré dobre fungovali

- "Make the vehicle spawn fully drivable: every part it needs, charged battery and all fluids full."
- "Add an obstacle check before spawning. If the spot is blocked or in water, try nearby spots; if none is clear, tell the player and don't count it against their limit."
- "Add a `!list` command showing every spawnable preset and how many the player has left."
- "Review this code [paste or attach] for things that won't compile or load in DayZ, and for logic errors against these requirements: [list]."
- "Write a test plan I can follow on a local server to check each requirement."
- "Here's SSH access to my server. Check every API the mod uses against the game's own scripts before deploying."
- "Can [feature] be done server-side only? Check the game's scripts and tell me what it can't cover."

### Tipy

- **Buď presný ohľadom herných objektov.** "Red Olga" sa stala `CivilianSedan_Wine`. Ak poznáš názvy tried, daj ich; ak nie, požiдай Claudea, aby vypísal tie, ktoré použil, aby si ich mohol skontrolovať v `types.xml` servera alebo s admin nástrojom.
- **Povedz, čo znamená "za hodinu".** "Za hodinu času hry" a "za hodinu času servera" sú rôzne funkcie a prežije ju len jedna z nich po odpojení.
- **Požiadať o sekciu "známe obmedzenia"** a považuj ju za svoj testovací zoznam.
- **Testuj na lokálnom serveri pred nasadením.** Sleduj log `.rpt` servera na skriptové chyby a riadky ako `[ChillSpawn] Could not attach …`.
- **Daj Claudeovi SSH prístup k testovaciemu serveru**, ak môžeš. Potom ho môže nasadiť, čítať logy a kontrolovať reálne skripty hry, čo mení hádanky na kontroly.
- **Na Windowse s nainštalovaným DayZ Tools** plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) pre Claude Code pridáva agentov a zručnosti špecifické pre DayZ, ktoré buildujú PBO a spúšťajú lokálny testovací server, čím sa zatvára medzera "neotestované".

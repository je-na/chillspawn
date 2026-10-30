# Ako vznikol ChillSpawn s pomocou Claude

[English](PROMPTING.md) · **Slovenčina** · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Strojový preklad. Pri rozdieloch platí [anglický originál](PROMPTING.md).

ChillSpawn bol napísaný v jednej relácii Claude Code (Claude Opus 5.5) na Linuxe bez inštalácie hry DayZ. Claude napísal každý súbor. Osoba, ktorá ho riadila, formulovala požiadavky, žiadala o zmeny a porovnávala výsledok s dvoma lokálnymi modelmi. Počas relácie sa nič nekompilovalo ani nespúšťalo v hre, takže kód považujte za silný prvý návrh, ktorý si stále vyžaduje testovanie na testovacom serveri.

Tento článok prechádza požiadavkami postupne (preformulované a upravené, nie doslovné citácie), čo každá zmenila a čo sa zistilo. Na konci nájdete podnety, ktoré môžete znova použiť na vytvorenie podobného modu.

## Podnety v poradí

### 1. Prvá požiadavka

> Napíšte serverový DayZ mod, ktorý umožní hráčovi spawnovať auto pomocou príkazu do chatu:
>
> - Auto je červená Olga s štyrmi kolesami, sviečkou a chladičom.
> - Spawnuje sa 6 metrov severne od hráča.
> - Hráč dostane súkromnú správu, že auto sa zanikne do 15 minút.
> - Každý hráč môže spawnovať 3 autá za hodinu času stráveného v hre.
> - Zahŕňa rámec, aby bol mod neskôr rozšíriteľný.
> - JSON konfiguračný súbor umožňuje vlastníkovi servera vybrať, čo sa má spawnovať: auto vrátane jeho príslušenstva alebo predmet.

Claude vytvoril celý mod: `config.cpp`, rozdelenie skriptov 3_Game/4_World/5_Mission, JSON konfig napísaný do `$profile:` pri prvom spustení, registráciu príkazov („rámec"), sledovanie času hry uložené pre každého hráča a README. Okrem toho označil, v čom si nie je istý, namiesto toho, aby to skryl:

- že server prijíma chat prostredníctvom `MissionServer.OnEvent` s `ChatMessageEventTypeID` a dostáva len meno odosielateľa;
- že Olga s kolesami, sviečkou a chladičom **nebude jazdiť** (žiadna batéria ani kvapaliny);
- že časy na zaniknutie sa stratia pri reštarte;
- že neexistovala kontrola prekážok.

**Zistenie:** číselný zoznam konkrétnych požiadaviek (presné vozidlo, vzdialenosť, smer, limity) prináša kompletnú odpoveď naraz. Požiadajte Claude, aby vypísal svoje predpoklady a všetko, čo nemohol overiť.

### 2. Urobiť ho jazditeľným a skontrolovať prekážky

> Pridaj všetko ostatné, čo auto potrebuje, aby sa spawnulo pripravené na jazdu. Tiež skontroluj prekážky pred spawnom.

Pridaná batéria (nabitá), svetlomety, dvere, kapota a kufor, naplnené palivo, chladivo, olej a brzdová kvapalina (všetko konfigurovateľné). Kontrola prekážok používa `IsBoxColliding` s boxom veľkosti auta plus kontrolu mora/rybníka. Skúsi 10 smerov na troch vzdialenostiach. Ak nie je nič jasné, hráčovi sa to oznámi a pokus sa nezapočíta do jeho limitu.

**Zistenie:** nadväzujúce požiadavky, ktoré reagujú na vlastný zoznam „známych obmedzení" Claude, sú lacné a cielené.

### 3. Viac ako jedna vec na spawnovanie

> Uprav príkladný konfiguračný súbor tak, aby ukazoval niekoľko vecí, ktoré sa dajú spawnovať, napríklad sekáč a Ada 4x4.

Konfig už toto podporoval (každý preset je vlastným príkazom v chate). Claude pridal predvolené nastavenia `!ada` a `!hatchet` do predvolených hodnôt aj do `config.example.json` a zdokumentoval, ako pridať ďalšie.

### 4. Porovnávanie s lokálnymi modelmi

> Nechaj môj lokálny model Qwen napísať ten istý mod, potom porovnaj jeho verziu s tvojou: ako sa líšia a či niektorá z nich obsahuje chyby?

Claude poslal rovnaké kombinované požiadavky lokálnemu modelu Qwen a neskôr skontroloval druhý pokus Qwen, ktorý používateľ poskytol. Žiadna z nich by nefungovala:

| | Lokálny Qwen #1 | Lokálny Qwen #2 |
|---|---|---|
| Reálna DayZ API | Väčšinou vymyslené (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Väčšinou reálne |
| Načítalo by sa | Nie: žiadne skriptové moduly, nikdy neviaže `MissionServer` | Nie: `config.cpp` nemá `CfgMods` |
| Skompilovalo by sa | Nie | Nie: trojuholníkový operátor, C++-štýl iterátory, `ToLower()` použité tak, akoby vracalo reťazec |
| „Sever" | 6 m vo vzduchu | 6 m na juh |
| Limit rýchlosti | V pamäti, pevné bloky | Uptime servera, stratí sa pri reštarte |

**Zistenie:** pre Enforce Script všeobecné modely sebavedome píšu C++/Java-izmy, ktoré DayZ nepodporuje. Získaj druhý názor tým, že jeden model skontroluje kód iného modelu podľa kontrolného zoznamu (načítanie, kompilácia, správanie), nie tým, že dôveruješ ktorémukoľvek z nich.

### 5. Vypožičanie si dobrej myšlienky

> Prenies myšlienku zo druhej verzie Qwen na zoznam vozidiel/predmetov do ChillSpawnu.

Qwen #2 malo podpríkaz `/vspawn list`. Claude pridal `!list` (každý preset s počtom spawnov, ktoré hráč má túto hodinu ešte k dispozícii) a `!help` ako zabudované príkazy v rámcici ChillSpawnu.

### 6. Publikovanie

> Komituj toto do môjho verejného GitHub repozitára (licencia MIT; ak chceš, nahraď README). Uisti sa, že sa nikde neobjavia žiadne prihlasovacie údaje. Pridaj dokumentáciu vysvetľujúcu, ako sa promptovalo a aké podnety môže niekto iný použiť na vytvorenie podobného modu.

To viedlo k tomuto repozitáru a tejto stránke.

### 7. Kontrola proti aktualizácii 1.30

> Skontroluj poznámky k verzii 1.30 a feedback tracker od Bohemii pre akékoľvek zmeny, ktoré by mohli narušiť našu logiku.

Nič v poznámkach sa nedotýkalo API, ktoré mod používa. Verzia 1.30 priniesla prísnejší skriptový kompilátor, takže dve konštrukcie, ktoré by mohol odmietnuť, boli zmenené (metóda deklarovaná bez tela a konverzia float na int).

### 8. Budovanie na Linuxe

> Pozri sa, či RaG DayZ Tools dokážu toto zostaviť na Linuxe.

RaG DayZ Tools sú primárne pre Windows, ale ich PBO builder je obyčajný Python a beží na Linuxe. `tools/build-linux.sh` ho používa na zabalenie modu. Binarizácia, konverzia konfigurácií a podpis stále potrebujú Windows programy, ale serverový mod len so skriptmi ich nepotrebuje.

## Podnety, ktoré môžete znova použiť

### Začíname nový serverový mod

Skopírujte toto, vyplňte zátvorky a vložte do Claude (Claude Code funguje najlepšie, pretože píše súbory za vás):

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

Riadok „Enforce Script has no…" vychádza priamo z chýb, ktoré urobili lokálne modely. Nechať ho tam nestojí nič a zabráni najbežnejším chybám.

### Nadväzujúce podnety, ktoré fungovali dobre

- „Urob vozidlo plne jazditeľným po spawnnutí: každú časť, ktorú potrebuje, nabitú batériu a všetky kvapaliny plné."
- „Pridaj kontrolu prekážok pred spawnom. Ak je miesto blokované alebo vo vode, skús susedné miesta; ak nie je žiadne jasné, oznám to hráčovi a nezapočítavaj to do jeho limitu."
- „Pridaj príkaz `!list` zobrazujúci každý spawnovateľný preset a koľko má hráč ešte k dispozícii."
- „Skontroluj tento kód [vlož alebo pripoj] na veci, ktoré sa nebudú kompilovať alebo načítať v DayZ, a na logické chyby voči týmto požiadavkám: [zoznam]."
- „Napíš testovací plán, ktorému môžem postupne na lokálnom serveri overiť každú požiadavku."

### Tipy

- **Buď presný ohľadom herných objektov.** „Červená Olga" sa stala `CivilianSedan_Wine`. Ak poznáš názvy tried, poskytni ich; ak nie, požiaj Claude, aby vypísal tie, ktoré použil, aby si ich mohol skontrolovať v serverovom `types.xml` alebo pomocou admin nástroja.
- **Vysvetli, čo znamená „za hodinu".** „Za hodinu herného času" a „za hodinu serverového času" sú rôzne funkcie a len jedna z nich prežije rekonexiu.
- **Požiadať o sekciu „známe limity"** a považuj ju za svoj testovací zoznam.
- **Testuj na lokálnom serveri pred nasadením do prevádzky.** Sleduj serverový log `.rpt` na skriptové chyby a riadky ako `[ChillSpawn] Could not attach …`.
- **Na Windows s nainštalovaným DayZ Tools** plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) pre Claude Code pridáva agentov a zručnosti špecifické pre DayZ, ktoré zostavujú PBO a spúšťajú lokálny testovací server, čím sa zatvára medzera „netestované".

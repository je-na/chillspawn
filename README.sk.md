# ChillSpawn

[English](README.md) · **Slovenčina** · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Strojový preklad. Pri rozdieloch platí [anglický originál](README.md).

Serverový mod pre DayZ. Hráči napíšu do chatu `!car`, aby si spawnuli červenú Olgu 24 pripravenú na jazdu, šesť metrov pred nimi: štyri kolesá, sviečka, chladič, nabité batérie, svetlomety, dvere, kapota a kufor, s plnou nádržou paliva, chladiva, oleja a brzdovej kvapaliny. Ak je toto miesto zablokované alebo pod vodou, skúsi nearby miesta; ak nie je žiadne voľné, hráčovi sa povie, aby sa presunul a pokus sa nezapočíta do jeho limitu. Dostane súkromnú správu, že vozidlo zmizne o 15 minút. Každý hráč môže spawnúť jednu Olgu každých 15 minút herného času (čas pripojenia, nie skutočný čas). Spawnuté autá nedostávajú poškodenie a nikto vo vnútri nie je zranený pri náraze.

Hráči môžu tiež napísať `!survive` raz za 2 reálne hodiny pre malú súpravu na prežitie. Administrátori nemajú žiadne obmedzenia a dostávajú príkaz `!adminkit`.

Klienti nepotrebujú tento mod. Načítajte ho pomocou `-servermod=@ChillSpawn`. Napísané pre DayZ 1.30.

> **Stav: otestované na DayZ 1.30 experimental.** `!help`, `!list`, `!car`, `!ada` a `!hatchet` fungujú na živom serveri bez klientského modu. Administrátori, sady (`kits`), `!survive` a ochrana áut sú nové a ešte nie sú testované. Pozri [Known limits / test first](#known-limits--test-first). [docs/PROMPTING.md](docs/PROMPTING.sk.md) ukazuje, ako bol tento mod vytvorený.

## Štruktúra

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

## Konfigurácia

Pri prvom spustení mod zapíše `$profile:ChillSpawn/config.json` (do priečinka `-profiles` servera). Každý záznam v `Presets` a `Kits` sa stane vlastným príkazom do chatu, takže vlastníci serverov môžu pridať viac bez zásahu do kódu. Predvolené hodnoty:

| Príkaz | Spawnuje | Limit |
|---|---|---|
| `!car` | Červená Olga 24, pripravená na jazdu, žiadne poškodenie, posádka chránená | 1 každých 15 min herného času |
| `!ada` | Zelená Ada 4x4, to isté (`OffroadHatchback_Blue` / `_White` pre iné farby, s prislúchajúcimi triedami `_Blue` / `_White` dverí, kapoty a kufra) | 1 každých 15 min herného času |
| `!hatchet` | Sekerka na zemi 1 m ďaleko | 3 každých 60 min herného času |
| `!survive` | Do inventára hráča: konzva bravčového mäsa, fľaša plná vody, tablety na čistenie vody, obväz a tetracyklín | 1 každé 2 reálne hodiny |
| `!adminkit` | Iba pre administrátorov. Na zemi pred nimi: batoh Alice, M14 s 3 plnými 20-nábojovými zásobníkmi a 3 krabicami .308, M4A1 s 3 plnými 60-nábojovými zásobníkmi a 2 tlmičmi, plný pancierový vest, prilba MICH s nočným videním a batériami, maskáčová bunda TTsKO, nohavice a topánky, rukavice a lovecký nôž | žiadny |

Dva vstavané príkazy sú vždy dostupné:

| Príkaz | Robí |
|---|---|
| `!list` | Vypíše všetko, čo môže hráč spawnúť, jeden na riadok, s počtom zostávajúcich a, keď nie sú žiadne zostávajúce, koľko času do ďalšieho |
| `!help` | Vypíše príkazy, ktoré môže hráč použiť |

Predvoľba alebo sada (`kit`) nemôže byť nazvaná `list` alebo `help`; vstavané majú prednosť a serverový log varuje.

### Administrátori

Umiestnite Steam64 ID administrátorov do `Admins`:

```json
"Admins": ["76561198000000000"],
```

Administrátori nemajú žiadne obmedzenia a môžu používať príkazy označené ako `AdminOnly`. Ostatní hráči nevidia príkazy iba pre administrátorov v `!help` alebo `!list` a nič sa nestane, ak ich napíšu.

### Predvoľby (Presets)

Každá predvoľba má vlastný limit (jedna Olga a jedna Ada každých 15 minút sú samostatné povolenia). Pre pridanie predvoľby skopírujte blok v `Presets`, dajte mu nové `Command` a reštartujte server. Polia:

| Pole | Význam |
|---|---|
| `Command` | Príkaz do chatu, bez prefixu (`car` → `!car`) |
| `DisplayName` | Názov zobrazený v správe |
| `ClassName` | Trieda vozidla alebo predmetu na spawnovanie |
| `Attachments` | Triedy pripojené k objektu. Opakujte triedu pre každý slot, ktorý vyplňuje (štyri kolesá) |
| `DistanceMetres`, `BearingDegrees` | Kde sa spawnuje vzhľadom na hráča. 0° = sever, 90° = východ |
| `DespawnSeconds` | 0 = nikdy nesmizne |
| `MaxSpawns`, `LimitMinutes` | Každý hráč môže spawnúť `MaxSpawns`-krát každých `LimitMinutes` minút herného času |
| `AdminOnly` | Iba administrátori to môžu použiť |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minúty do zmiznutia |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Iba vozidlá: ako plná je každá kvapalina na začiatku, 0 až 1 |
| `Invulnerable` | Vozidlo a jeho príslušenstvo nedostávajú žiadne poškodenie: nárazy, gule, výbuchy |
| `ProtectCrew` | Iba vozidlá: nikto vo vnútri nie je zranený alebo zabitý pri náraze |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Potrebný čistý priestor (metry) na mieste spawnu. Predvolená hodnota vyhovuje Olge; zvýšte ju pre kamióny |

### Sady (Kits)

Sada je súbor predmetov pre jeden príkaz. Polia:

| Pole | Význam |
|---|---|
| `Command`, `DisplayName` | Ako pri predvoľbách |
| `MaxSpawns`, `LimitMinutes` | Limit na hráča |
| `RealTimeLimit` | Počítať `LimitMinutes` v reálnom čase namiesto herného času (prežíva reštarty) |
| `AdminOnly` | Iba administrátori to môžu použiť |
| `ToInventory` | Položiť predmety do inventára hráča, kde je miesto; zvyšok padne na zem. Inak všetko padne na zem pred nimi |
| `Items` | `ClassName`, `Count` a `Contents`. Obsah ide do predmetu: pripojený tam, kam zapadá (zásobníky, tlmiče, batérie, puzdrá), inak do jeho nákladu, a môže mať vlastný obsah |

Predmety v sade sa spawnujú ako nové, zásobníky a munícia plné, batérie nabité a fľaše plné vody. Zásobník uvedený v obsahu zbrane je nabitý s jednou guľkou v komore.

Predvolená konfigurácia sa zapíše len vtedy, ak neexistuje `config.json`. Po aktualizácii modu odstráňte starý súbor (alebo pridajte nové polia do neho), aby ste získali nové predvolené hodnoty; serverový log varuje, keď je súbor zo staršej verzie.

Herný čas a história spawnovania na hráča sa ukladajú v `$profile:ChillSpawn/players/`.

## Pridávanie príkazov (rozšírenie)

Podtrieda `CS_ChatCommand` a zaregistrujte ju z iného modu alebo v tomto:

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

Základné funkcie fungujú na DayZ 1.30 experimental. Skontrolujte tieto body na testovacom serveri pri zmene konfigurácie alebo aktualizácii hry:

1. **Chat hook: funguje.** Mod číta chat cez `MissionServer.OnEvent` s `ChatMessageEventTypeID`, takže hráči nepotrebujú nič inštalovať. [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) uvádza `ChatMessageEventTypeID` ako udalosť na strane klienta, ale na serveri DayZ 1.30 experimental ju prijíma aj server. Udalosť poskytuje iba meno odosielateľa, takže hráči sú spájaní podľa mena. Ak majú dvaja hráči rovnaké meno, príkaz sa ignoruje.
2. **Názvy tried príslušenstva a položiek sady.** Skontrolujte, či každá trieda v `Attachments` a každá položka v sade spawnujú (serverový log vytlačí `Could not attach …`, `Could not create …` alebo `Could not put …` pre tie, ktoré nie sú), a či auto štartuje a jazdí.
3. **Ochrana auta (nové).** `Invulnerable` zastavuje poškodenie auta a dielov, s ktorými bolo spawnuté; diely pripojené neskôr nie sú chránené. `ProtectCrew` zastavuje poškodenie pri náraze pre ľudí vo vnútri. Ani jedno nechráni ľudí, ktorých auto narazí. Obe funkcie sa držia v pamäti, takže po reštarte servera auto, ktoré ešte nesmizlo, je znova obyčajné auto.
4. **Kontrola prekážok.** Používa `IsBoxColliding` s boxom veľkosti auta, zdvihnutým 0,3 m nad zemou, plus kontrolu mora/rybníka. Skúša 10 smerov vo vzdialenostiach 6, 9 a 12 m. Skontrolujte, či správne neklasifikuje kríky ako prekážky a či zachytáva steny a ploty. Nekontroluje sklon, takže auto môže stále dopadnúť na strmý svah.
5. **Časovače zmiznutia žijú v pamäti.** Ak sa server reštartuje pred vypršaním časovača auta, auto zostane vo svete ako bežné trvalé vozidlo. Ak je hráč v aute, keď časovač vyprší, pokus o zmiznutie sa opakuje každú minútu, kým nevyjde von.

## Building

### Linux

```bash
tools/build-linux.sh
```

Toto balí `dist/@ChillSpawn/addons/ChillSpawn.pbo` s PBO builderom z [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools je freeware pod vlastnou licenciou a nie je tu zahrnutý: skript ho naklonuje do `~/.cache/rag-dayz-tools` pri prvom spustení, alebo použije cestu, ktorú zadáte (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) alebo nastavíte v `RAG_TOOLS_DIR`. Vyžaduje `python3` 3.10+ a `git`.

Skript spustí predspúšťacie kontroly RaG a balí skripty tak, ako sú. Preskočí binarizáciu, konverziu `config.cpp` a podpisovanie, ktoré vyžadujú Windows programy DayZ Tools. Serverový mod iba so skriptmi ich nepotrebuje. Na Linuxe sa Enforce Script nekompiluje, takže chyby v skriptoch sa objavia len v serverovom logu `.rpt` pri štarte.

Skopírujte `dist/@ChillSpawn` do priečinka servera a pridajte `-servermod=@ChillSpawn` do riadku spustenia.

### Windows

Zabaľte priečinok `ChillSpawn/` do `@ChillSpawn/addons/ChillSpawn.pbo` pomocou AddonBuilderu z DayZ Tools alebo RaG PBO Buildera. Mod iba pre server nepotrebuje striktne podpisovanie, ale ak server kontroluje podpisy, podejte PBO a umiestnite `.bikey` do priečinka `keys/` servera.

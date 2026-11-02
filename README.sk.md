# ChillSpawn

[English](README.md) · **Slovenčina** · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Strojový preklad. Pri rozdieloch platí [anglický originál](README.md).

Serverový mod pre DayZ. Hráči napíšu `!car` do chatu na spawn pripraveného červeného Olgu 24 šesť metrov pred nimi: štyri kolesá, sviečka, chladič, nabitá batéria, svetlomety, dvere, kapota a kufor, s plnou nádržou paliva, chladiva, oleja a brzdovej kvapaliny. Ak je toto miesto blokované alebo vo vode, skúsi okolité miesta; ak nie je žiadne voľné, hráčovi sa povie, aby sa presunul a pokus sa nepočíta proti jeho limitu. Dostane súkromnú správu, že vozidlo zmizne o 15 minút. Každý hráč môže spawnovať jeden Olga každých 15 minút herného času (čas pripojenia, nie skutočný čas). Spawnované autá neberú žiadne poškodenie a nikto vo vnútri nie je zranený pri náraze.

Hráči môžu tiež napísať `!survive` raz za 2 reálne hodiny pre malú súpravu na prežitie. Administrátori nemajú žiadne obmedzenia a dostanú `!adminkit`.

Klienti nepotrebujú tento mod. Načítajte ho pomocou `-servermod=@ChillSpawn`. Napísané pre DayZ 1.30.

> **Stav: otestované na DayZ 1.30 experimental.** Každý príkaz, administrátori, súpravy a ochrana vozidiel fungujú na živom serveri bez modifikácie klienta. Pozri [Known limits / test first](#známe-obmedzenia--najprv-otestujte). [docs/PROMPTING.md](docs/PROMPTING.sk.md) ukazuje, ako bol tento mod vytvorený.

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

Pri prvom spustení mod zapíše `$profile:ChillSpawn/config.json` (do priečinka `-profiles` servera). Každá položka v `Presets` a `Kits` sa stane vlastným príkazom v chate, takže majitelia serverov môžu pridať viac bez zásahu do kódu. Predvolené hodnoty:

| Príkaz | Spawnuje | Limit |
|---|---|---|
| `!car` | Červený Olga 24, pripravený na jazdu, žiadne poškodenie, posádka chránená | 1 každých 15 min herného času |
| `!ada` | Zelený Ada 4x4, to isté (`OffroadHatchback_Blue` / `_White` pre iné farby, s zodpovedajúcimi triedami `_Blue` / `_White` dverí, kapoty a kufra) | 1 každých 15 min herného času |
| `!hatchet` | Sekera na zemi 1 m ďaleko | 3 každých 60 min herného času |
| `!survive` | Do inventára hráča: konzva s bravčovým mäsom, fľaša vody plná, tablety na čistenie vody, obväz a tetracyklín | 1 každé 2 reálne hodiny |
| `!adminkit` | Len pre administrátorov. Na zemi pred nimi: batoh Alice, M14 s 3 plnými zásobníkmi po 20 nábojoch a 3 krabicami .308, M4A1 s 3 plnými zásobníkmi po 60 nábojoch a 2 tlmičmi, kompletná vesta s pancierovými doskami, prilba MICH s nočným videním a batériami, kamuflážna bunda TTsKO, nohavice a topánky, rukavice a lovecký nôž | žiadny |

Dva zabudované príkazy sú vždy k dispozícii:

| Príkaz | Robí |
|---|---|
| `!list` | Vypíše všetko, čo hráč môže spawnovať, po jednom na riadok, s počtom zostávajúcich a, keď žiadne nezostávajú, ako dlho čaká na ďalší |
| `!help` | Vypíše príkazy, ktoré hráč môže použiť |

Predvoľba alebo súprava nemôže byť nazvaná `list` alebo `help`; zabudované majú prioritu a server loguje varovanie.

### Administrátori

Umiestnite Steam64 ID administrátorov do `Admins`:

```json
"Admins": ["76561198000000000"],
```

Administrátori nemajú žiadne limity a môžu používať príkazy označené ako `AdminOnly`. Ostatní hráči nevidia príkazy iba pre administrátorov v `!help` ani v `!list` a ak jeden napíšu, nestane sa nič.

### Predvoľby (Presets)

Každá predvoľba má vlastný limit (jeden Olga a jedna Ada každých 15 minút sú samostatné povolenia). Na pridanie predvoľby skopírujte blok v `Presets`, dajte mu nový `Command` a reštartujte server. Polia:

| Pole | Význam |
|---|---|
| `Command` | Príkaz do chatu, bez prefixu (`car` → `!car`) |
| `DisplayName` | Názov zobrazený v správe |
| `ClassName` | Trieda vozidla alebo predmetu na spawnovanie |
| `Attachments` | Triedy pripojené k vozidlu. Opakujte triedu pre každý slot, ktorý zapĺňa (štyri kolesá) |
| `DistanceMetres`, `BearingDegrees` | Kde sa spawnuje vzhľadom na hráča. 0° = sever, 90° = východ |
| `DespawnSeconds` | 0 = nikdy nezmizne |
| `MaxSpawns`, `LimitMinutes` | Každý hráč ho môže spawnovať `MaxSpawns`-krát každých `LimitMinutes` minút herného času |
| `AdminOnly` | Len administrátori to môžu použiť |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minúty do zmiznutia |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Len vozidlá: ako plná je každá kvapalina na začiatku, 0 až 1 |
| `Invulnerable` | Vozidlo a jeho príslušenstvo nezoberú žiadne poškodenie: nárazy, gule, explózie |
| `ProtectCrew` | Len vozidlá: nikto vo vnútri nie je zranený alebo zabitý pri náraze |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Voľný priestor (metry) potrebný na mieste spawnu. Predvolená hodnota sedí na Olgu; zvýšte ju pre nákladné autá |

### Súpravy (Kits)

Súprava je sada predmetov pre jeden príkaz. Polia:

| Pole | Význam |
|---|---|
| `Command`, `DisplayName` | Ako pri predvoľbách |
| `MaxSpawns`, `LimitMinutes` | Limit na hráča |
| `RealTimeLimit` | Počítať `LimitMinutes` v reálnom čase namiesto herného času (prežíva reštarty) |
| `AdminOnly` | Len administrátori to môžu použiť |
| `ToInventory` | Položiť predmety do inventára hráča, kde je miesto; zvyšok padne na zem. Inak všetko padne na zem pred nimi |
| `Items` | `ClassName`, `Count` a `Contents`. Obsah ide do predmetu: pripojený tam, kde sa zmestí (zásobníky, tlmiče, batérie, puzdrá), inak do jeho nákladu, a môže mať vlastný obsah |

Predmety v súprave sa spawnujú ako nové, zásobníky a munícia plné, batérie nabité a fľaše plné vody. Zásobník uvedený v obsahu zbrane je nabitý jednou guľou v komore.

Predvolená konfigurácia sa zapíše len vtedy, ak neexistuje žiadny `config.json`. Po aktualizácii modu odstráňte starý súbor (alebo pridajte nové polia doňho), aby ste získali nové predvolené hodnoty; server loguje varovanie, keď je súbor zo staršej verzie.

Herný čas a história spawnov na hráča sa ukladajú v `$profile:ChillSpawn/players/`.

## Pridávanie príkazov (rozšírenie)

Vytvorte podtriedu `CS_ChatCommand` a zaregistrujte ju z iného modu, alebo v tomto jednom:

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

## Známe obmedzenia / najprv otestujte

Základné funkcie fungujú na DayZ 1.30 experimental. Skontrolujte tieto body na testovacom serveri pri zmene konfigurácie alebo aktualizácii hry:

1. **Chat hook: funguje.** Mod číta chat cez `MissionServer.OnEvent` s `ChatMessageEventTypeID`, takže hráči nepotrebujú nič inštalovať. [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) uvádza `ChatMessageEventTypeID` ako udalosť na strane klienta, ale na serveri DayZ 1.30 experimental ju dostane aj server. Udalosť poskytuje iba meno odosielateľa, takže hráči sa spárujú podľa mena. Ak dvaja hráči zdieľajú rovnaké meno, príkaz je ignorovaný.
2. **Názvy tried príslušenstva a súprav.** Skontrolujte, či sa každá trieda v `Attachments` a každý predmet v súprave spawnuje (server log vytlačí `Could not attach …`, `Could not create …` alebo `Could not put …` pre tie, ktoré to neurobia) a či auto štartuje a jazdí.
3. **Ochrana auta.** `Invulnerable` zastavuje poškodenie auta a dielov, s ktorými bolo spawnované; diely pripojené neskôr nie sú chránené. `ProtectCrew` zastavuje poškodenie pri náraze pre ľudí vo vnútri. Ani jedno nechráni ľudí, ktoré auto narazí. Obe sú uložené v pamäti, takže po reštarte servera je auto, ktoré ešte nezmizlo, opäť bežným autom.
4. **Kontrola prekážok.** Používa `IsBoxColliding` s boxom veľkosti auta, zdvihnutým 0,3 m od zeme, plus kontrolu mora/rybníka. Skúša 10 smerov vo vzdialenostiach 6, 9 a 12 m. Skontrolujte, či správne nevyhodnotí kríky ako prekážky a či zachytí múry a ploty. Nesk kontroluje svah, takže auto môže stále dopadnúť na strmý svah.
5. **Časovače zmiznutia žijú v pamäti.** Ak sa server reštartuje skôr, než vyprší časovač auta, auto zostane vo svete ako bežné trvalé vozidlo. Ak je hráč v aute, keď časovač vyprší, pokus o zmiznutie sa opakuje každú minútu, kým nevyjde von.

## Buildovanie

### Linux

```bash
tools/build-linux.sh
```

Toto balí `dist/@ChillSpawn/addons/ChillSpawn.pbo` s PBO builderom z [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools je freeware pod vlastnou licenciou a nie je zahrnutý tu: skript ho klonuje do `~/.cache/rag-dayz-tools` pri prvom spustení, alebo použije cestu, ktorú predáte (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) alebo nastavíte v `RAG_TOOLS_DIR`. Vyžaduje `python3` 3.10+ a `git`.

Skript spúšťa preflight kontroly RaG a balí skripty tak, ako sú. Preskakuje binarizáciu, konverziu `config.cpp` a podpisovanie, čo vyžaduje Windows programy DayZ Tools. Serverový mod iba so skriptmi nepotrebuje žiadne z nich. Na Linuxe sa Enforce Script nekompiluje, takže chyby skriptov sa objavia len v `.rpt` logu servera pri štarte.

### Windows

Zabalte priečinok `ChillSpawn/` do `@ChillSpawn/addons/ChillSpawn.pbo` pomocou AddonBuilder z DayZ Tools alebo RaG PBO Buildera. Mod iba pre server nepotrebuje striktne podpisovanie, ale ak server kontroluje podpisy, podepište PBO a vložte `.bikey` do priečinka `keys/` servera.

## Inštalácia

1. Skopírujte priečinok `@ChillSpawn` do koreňového priečinka servera, vedľa `DayZServer_x64.exe`.
2. Načítajte ho ako serverový mod:
   - **Čistý server:** pridajte `-servermod=@ChillSpawn` do štartovacieho riadku.
   - **CFTools Architect:** použite **Add Local Mod**, vyberte `@ChillSpawn` a nastavte jeho typ na **Server**.
3. Spustite server. Zapíše `profiles/ChillSpawn/config.json` a loguje `[ChillSpawn] Loaded … spawn presets, … kits and … admins.`
4. Pridajte Steam64 ID vašich administrátorov do `Admins` v tomto súbore a reštartujte.

**Aktualizácia:** úplne zastavte server pred nahradením `ChillSpawn.pbo`. Bežiaci server uzamkne súbor a rýchly reštart nemusí nechať čas na jeho výmenu.

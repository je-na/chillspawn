# ChillSpawn

[English](README.md) · **Slovenčina** · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Strojový preklad. Pri rozdieloch platí [anglický originál](README.md).

Serverový DayZ mód. Hráči napíšu do chatu `!car` na vyvolanie pripraveného červeného Olgu 24 šesť metrov severne od nich: štyri kolesá, sviečka, chladič, nabitá batéria, svetlomety, dvere, kapota a kufor, s plnou nádržou paliva, chladiva, oleja a brzdovej kvapaliny. Ak je toto miesto blokované alebo vo vode, skúsi okolité miesta; ak nie je žiadne voľné, hráčovi sa oznámi, aby sa pohnul, a pokus sa nepočíta do jeho limitu. Získajú súkromnú správu, že vozidlo zanikne do 15 minút. Každý hráč si môže vyvolať 3 vozidlá za hodinu herného času (čas pripojenia, nie skutočný čas).

Klienti nepotrebujú mať nainštalovaný tento mód. Načítajte ho pomocou `-servermod=@ChillSpawn`. Napísané pre DayZ 1.30.

> **Stav: netestované.** Kód bol napísaný s Claude a ešte nebol skompilovaný ani spustený na serveri. Pred použitím na živom serveri si pozrite [Známe obmedzenia / najprv otestovať](#známe-obmedzenia--najprv-otestovať). Súbor [docs/PROMPTING.md](docs/PROMPTING.sk.md) ukazuje, ako bol tento mód vytvorený.

## Štruktúra súborov

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

## Konfigurácia

Pri prvom spustení mód zapíše `$profile:ChillSpawn/config.json` (do priečinka `-profiles` servera). Každá predvoľba v `Presets` sa stane samostatným príkazom do chatu, takže vlastníci serverov môžu pridať ďalšie vozidlá alebo predmety bez zásahu do kódu. Predvolená konfigurácia obsahuje tri:

| Príkaz | Vyvolá |
|---|---|
| `!car` | Červený Olga 24, pripravený na jazdu |
| `!ada` | Zelený Ada 4x4, pripravený na jazdu (`OffroadHatchback_Blue` / `_White` pre iné farby, s prislúchajúcimi triedami `_Blue` / `_White` dverí, kapoty a kufra) |
| `!hatchet` | Sekera na zemi 1 meter ďaleko |

Vždy sú dostupné dva vstavané príkazy:

| Príkaz | Robí |
|---|---|
| `!list` | Vypíše všetky predvoľby, jednu na riadok, spolu s počtom zostávajúcich vyvolaní pre hráča v tejto hodine herného času a, keď už žiadne nezostávajú, ako dlho čaká do ďalšej |
| `!help` | Vypíše všetky príkazy |

Predvoľba nemôže mať názov `list` alebo `help`; vstavané príkazy majú prednosť a serverový log vydá varovanie.

Každá predvoľba má vlastný limit (3 Olgu a 3 sekery za hodinu sú oddelené povolenia). Na pridanie predvoľby skopírujte blok v `Presets`, priraďte mu nový `Command` a reštartujte server. Polia:

| Pole | Význam |
|---|---|
| `Command` | Príkaz do chatu, bez prefixu (`car` → `!car`) |
| `DisplayName` | Názov zobrazený v správe |
| `ClassName` | Trieda vozidla alebo predmetu na vyvolanie |
| `Attachments` | Triedy pripojené k objektu. Opakujte triedu pre každú pozíciu, ktorú zaberá (štyri kolesá) |
| `DistanceMetres`, `BearingDegrees` | Kde sa vyvolá vzhľadom na hráča. 0° = sever, 90° = východ |
| `DespawnSeconds` | 0 = nikdy nezanikne |
| `MaxPerPlaytimeHour` | Limit vyvolaní za hodinu herného času, počítané podľa predvoľby |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minúty do zániku |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Len vozidlá: ako plná je každá kvapalina na začiatku, 0 až 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Priestor (v metroch) potrebný na mieste vyvolania. Predvolená hodnota sedí na Olgu; zvýšte ju pre nákladné vozidlá |

Predvolená konfigurácia sa zapíše len vtedy, ak neexistuje žiadny `config.json`. Po aktualizácii módu odstráňte starý súbor (alebo pridajte nové polia do existujúceho), aby sa načítali nové predvolené hodnoty.

Herný čas a história vyvolaní pre každého hráča sú uložené v `$profile:ChillSpawn/players/`.

## Pridávanie príkazov (rozšírenie)

Vytvorte podtriedu `CS_ChatCommand` a zaregistrujte ju z iného módu alebo v tomto:

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

## Známe obmedzenia / najprv otestovať

Ešte nebol skompilovaný ani testovaný. Skontrolujte tieto body najprv na testovacom serveri:

1. **Chat hook (najpravdepodobnejšie zlyhanie).** Mód číta chat cez `MissionServer.OnEvent` s `ChatMessageEventTypeID`, takže hráči nepotrebujú nič inštalovať. [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) uvádza `ChatMessageEventTypeID` ako udalosť na strane klienta a jeho tutoriál pre príkazy do chatu používa malý klientský mód namiesto toho. Otestujte to najprv: ak `!help` nedostane odpoveď, server neprijíma udalosti chatu a mód bude potrebovať klientsku časť. Udalosť poskytuje iba meno odosielateľa, takže hráči sú spájaní podľa mena. Ak dvaja hráči zdieľajú rovnaké meno, príkaz sa ignoruje.
2. **Názvy tried príslušenstva.** Skontrolujte, či každá trieda v `Attachments` pripája (serverový log vypíše `Could not attach …` pre akúkoľvek, ktorá to neurobí), a že auto nastartuje a ide jazdiť.
3. **Kontrola prekážok.** Používa `IsBoxColliding` s boxom veľkosti auta, zdvihnutým 0,3 m nad zemou, plus kontrolu mora/rybníka. Skúša 10 smerov na vzdialenostiach 6, 9 a 12 m. Skontrolujte, že nesprávne neberie kroviny ako prekážky a že zachytáva steny a ploty. Nekontroluje sklon, takže auto môže stále dopadnúť na strmom svahu.
4. **Časovače zániku sú v pamäti.** Ak sa server reštartuje skôr, než vyprší časovač auta, auto zostane vo svete ako bežné pretrvávajúce vozidlo. Ak je hráč v aute, keď časovač vyprší, pokus o zánik sa opakuje každú minútu, kým nevyjde von.

## Buildovanie

### Linux

```bash
tools/build-linux.sh
```

Toto zabalisí `dist/@ChillSpawn/addons/ChillSpawn.pbo` pomocou PBO buildera z [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools sú freeware pod vlastnou licenciou a nie sú tu zahrnuté: skript ich naklonuje do `~/.cache/rag-dayz-tools` pri prvom spustení, alebo použije cestu, ktorú zadáte (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) alebo nastavíte v premennej `RAG_TOOLS_DIR`. Potrebuje `python3` 3.10+ a `git`.

Skript spustí predbežné kontroly RaG a zabalisí skripty tak, ako sú. Preskočí binarizáciu, konverziu `config.cpp` a podpisovanie, čo vyžaduje Windows programy DayZ Tools. Serverový mód len so skriptmi ich nepotrebuje. Na Linuxe sa Enforce Script nekompiluje, takže chyby skriptov sa zobrazia až v serverovom logu `.rpt` pri štarte.

Skopírujte `dist/@ChillSpawn` do priečinka servera a pridajte `-servermod=@ChillSpawn` do spúšťacieho riadku.

### Windows

Zabalte priečinok `ChillSpawn/` do `@ChillSpawn/addons/ChillSpawn.pbo` pomocou AddonBuilderu DayZ Tools alebo RaG PBO Buildera. Serverové módy prísnym vzorom nepotrebujú podpisovanie, ale ak server kontroluje podpisy, podešlite PBO a vložte `.bikey` do priečinka `keys/` servera.

# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · **Polski** · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Tłumaczenie maszynowe. W razie rozbieżności obowiązuje [angielski oryginał](README.md).

Serwerowy mod do DayZ. Gracze wpisują `!car` na czacie, aby przywołać czerwony Olga 24 gotowy do jazdy, znajdujący się sześć metrów na północ od nich: z czterema kołami, świecą zapłonową, radiatorem, naładowaną baterią, reflektorami, drzwiami, maską i bagażnikiem, w pełni zatankowany płynami (paliwo, chłodniczy, olej, płyn hamulcowy). Jeśli to miejsce jest zablokowane lub znajduje się w wodzie, skrypt próbuje pobliskich lokalizacji; jeśli żadna nie jest wolna, gracz zostaje poproszony o zmianę pozycji, a ta próba nie jest liczona do limitu. Otrzymuje prywatną wiadomość informującą, że pojazd zniknie po 15 minutach. Każdy gracz może przywołać 3 pojazdy na godzinę rozgrywki (czas połączenia z serwerem, a nie czas rzeczywisty).

Klientom nie jest wymagany ten mod. Ładuj go za pomocą `-servermod=@ChillSpawn`. Napisany dla DayZ 1.30.

> **Status: nietestowany.** Kod został napisany przy użyciu Claude i nie został jeszcze skompilowany ani uruchomiony na serwerze. Zobacz [Znane ograniczenia / przetestuj najpierw](#znane-ograniczenia--przetestuj-najpierw) przed użyciem go na live'owym serwerze. [docs/PROMPTING.md](docs/PROMPTING.pl.md) pokazuje, jak powstał ten mod.

## Struktura plików

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

## Konfiguracja

Przy pierwszym uruchomieniu mod zapisuje `$profile:ChillSpawn/config.json` (w folderze `-profiles` serwera). Każdy preset w `Presets` staje się osobnym poleceniem czatu, dzięki czemu właściciele serwerów mogą dodawać więcej pojazdów lub przedmiotów bez dotykania kodu. Domyślna konfiguracja zawiera trzy presety:

| Polecenie | Przywołuje |
|---|---|
| `!car` | Czerwony Olga 24, gotowy do jazdy |
| `!ada` | Zielony Ada 4x4, gotowy do jazdy (`OffroadHatchback_Blue` / `_White` dla innych kolorów, z pasującymi klasami `_Blue` / `_White` drzwi, maski i bagażnika) |
| `!hatchet` | Toporek na ziemi w odległości 1 m |

Dwa wbudowane polecenia są zawsze dostępne:

| Polecenie | Działa |
|---|---|
| `!list` | Wyświetla każdy preset, po jednym na linię, wraz z liczbą pozostałych przywołań w tej godzinie rozgrywki i, gdy nie ma już limitu, ile czasu zostało do odświeżenia |
| `!help` | Wyświetla wszystkie polecenia |

Preset nie może mieć nazwy `list` lub `help`; wbudowane komendy mają priorytet, a w logu serwera pojawi się ostrzeżenie.

Każdy preset ma własny limit (3 Olgi i 3 toporki na godzinę to osobne przydziały). Aby dodać preset, skopiuj blok w `Presets`, nadaj mu nową `Command` i zrestartuj serwer. Pola:

| Pole | Znaczenie |
|---|---|
| `Command` | Polecenie czatu, bez prefiksu (`car` → `!car`) |
| `DisplayName` | Nazwa wyświetlana w wiadomości |
| `ClassName` | Klasa pojazdu lub przedmiotu do przywołania |
| `Attachments` | Klasy mocowane do obiektu. Powtórz klasę dla każdego slotu, który wypełnia (cztery koła) |
| `DistanceMetres`, `BearingDegrees` | Miejsce przywołania względem gracza. 0° = północ, 90° = wschód |
| `DespawnSeconds` | 0 = nigdy nie znikaj |
| `MaxPerPlaytimeHour` | Limit przywołań na godzinę rozgrywki, liczony osobno dla każdego presetu |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minuty do zniknięcia |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Tylko pojazdy: jak pełne są płyny na start, 0 do 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Przestrzeń (metry) wymagana w miejscu przywołania. Domyślnie pasuje do Olgi; podnieś dla ciężarówek |

Domyślna konfiguracja jest zapisywana tylko wtedy, gdy nie istnieje plik `config.json`. Po aktualizacji moda usuń stary plik (lub dodaj nowe pola do istniejącego), aby załadować nowe domyślne ustawienia.

Czas gry i historia przywołań dla każdego gracza są zapisywane w `$profile:ChillSpawn/players/`.

## Dodawanie poleceń (rozszerzenie)

Stwórz podklasę `CS_ChatCommand` i zarejestruj ją z innego moda lub w tym samym:

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

## Znane ograniczenia / przetestuj najpierw

Kod nie został jeszcze skompilowany ani przetestowany. Sprawdź te punkty na serwerze testowym:

1. **Hook czatu (najbardziej prawdopodobny do błędów).** Mod odczytuje czat przez `MissionServer.OnEvent` z `ChatMessageEventTypeID`, więc gracze nie muszą nic instalować. [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) wymienia `ChatMessageEventTypeID` jako zdarzenie po stronie klienta, a jego poradnik dotyczący poleceń czatu używa małego moda klienckiego zamiast tego. Przetestuj to najpierw: jeśli `!help` nie zwraca odpowiedzi, serwer nie odbiera zdarzeń czatu i mod będzie wymagał części klienckiej. Zdarzenie podaje tylko nazwę nadawcy, więc gracze są dopasowywani po nazwie. Jeśli dwóch graczy ma tę samą nazwę, polecenie zostanie zignorowane.
2. **Nazwy klas akcesoriów.** Sprawdź, czy każda klasa w `Attachments` się mocuje (serwer loguje `Could not attach …` dla tych, które nie), i czy pojazd startuje oraz jeździ.
3. **Sprawdzanie przeszkód.** Używa `IsBoxColliding` z pudełkiem wielkości pojazdu, uniesionym 0,3 m nad ziemią, plus sprawdzenie morza/stawu. Próbuje 10 kierunków w odległościach 6, 9 i 12 m. Sprawdź, czy nie traktuje krzaków błędnie jako przeszkód i czy łapię ściany oraz płoty. Nie sprawdza nachylenia terenu, więc pojazd może nadal wylądować na stromym zboczu.
4. **Timery znikania żyją w pamięci.** Jeśli serwer zostanie zrestartowany przed upływem czasu timera pojazdu, pojazd pozostaje w świecie jako normalny trwały pojazd. Jeśli gracz jest w pojeździe, gdy timer wygaśnie, próba usunięcia będzie powtarzana co minutę, aż gracz wysiądzie.

## Budowanie

### Linux

```bash
tools/build-linux.sh
```

Spakowuje to `dist/@ChillSpawn/addons/ChillSpawn.pbo` za pomocą kreatora PBO z [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools to freeware na własnej licencji i nie jest dołączony tutaj: skrypt klonuje go do `~/.cache/rag-dayz-tools` przy pierwszym uruchomieniu, lub używa ścieżki podanej przez Ciebie (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) lub ustawionej w zmiennej `RAG_TOOLS_DIR`. Wymaga `python3` 3.10+ i `git`.

Skrypt uruchamia kontrole preflight RaG i pakuje skrypty tak, jak są. Pomija binaryzację, konwersję `config.cpp` i podpisywanie, które wymagają programów DayZ Tools dla Windowsa. Serwerowy mod tylko ze skryptami nie wymaga żadnej z tych operacji. Nic na Linuxie nie kompiluje Enforce Script, więc błędy skryptów pojawiają się dopiero w logu `.rpt` serwera przy jego starcie.

Skopiuj `dist/@ChillSpawn` do folderu serwera i dodaj `-servermod=@ChillSpawn` do linii startowej.

### Windows

Spakuj folder `ChillSpawn/` do `@ChillSpawn/addons/ChillSpawn.pbo` używając AddonBuildera z DayZ Tools lub RaG PBO Buildera. Same moduły serwerowe nie wymagają ściśle podpisywania, ale jeśli serwer sprawdza sygnatury, podpisz PBO i umieść `.bikey` w folderze `keys/` serwera.

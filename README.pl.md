# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · **Polski** · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Tłumaczenie maszynowe. W razie rozbieżności obowiązuje [angielski oryginał](README.md).

Serwerowy mod do DayZ. Gracze wpisują `!car` na czacie, aby spawnować gotowy do jazdy czerwony Olga 24 sześć metrów przed nimi: cztery koła, świeca zapłonowa, radiator, naładowana bateria, reflektory, drzwi, maska i bagażnik, z pełnym zbiornikiem paliwa, chłodziwa, oleju i płynu hamulcowego. Jeśli to miejsce jest zablokowane lub znajduje się w wodzie, mod próbuje pobliskich lokacji; jeśli żadna nie jest wolna, gracz zostaje poproszony o zmianę pozycji, a próba nie liczy się do jego limitu. Otrzymuje prywatną wiadomość informującą, że pojazd zniknie po 15 minutach. Każdy gracz może spawnować jedną Olgę co 15 minut czasu gry (czas połączenia z serwerem, a nie czas rzeczywisty). Spawnowane pojazdy nie poniosą obrażeń, a nikomu w środku nic się nie stanie przy kolizji.

Gracze mogą również wpisywać `!survive` raz na 2 godziny rzeczywiste dla małego zestawu przetrwania. Administratorzy nie mają żadnych limitów i otrzymują `!adminkit`.

Klientom nie jest wymagany ten mod. Wczytuj go jako `-servermod=@ChillSpawn`. Stworzony dla DayZ 1.30.

> **Status: przetestowano na DayZ 1.30 experimental.** Każda komenda, administratorzy, zestawy i ochrona pojazdów działają na żywym serwerze bez modów po stronie klienta. Zobacz [Znane ograniczenia / testuj najpierw](#znane-ograniczenia--testuj-najpierw). [docs/PROMPTING.md](docs/PROMPTING.pl.md) pokazuje, jak ten mod został stworzony.

## Struktura

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

## Konfiguracja

Przy pierwszym uruchomieniu mod zapisuje `$profile:ChillSpawn/config.json` (w folderze `-profiles` serwera). Każda pozycja w `Presets` i `Kits` staje się osobną komendą czatu, więc właściciele serwerów mogą dodawać nowe bez dotykania kodu. Domyślne ustawienia:

| Komenda | Spawnuje | Limit |
|---|---|---|
| `!car` | Czerwona Olga 24, gotowa do jazdy, bez obrażeń, załoga chroniona | 1 co 15 min czasu gry |
| `!ada` | Zielony Ada 4x4, to samo (`OffroadHatchback_Blue` / `_White` dla innych kolorów, z pasującymi klasami drzwi `_Blue` / `_White`, maski i bagażnika) | 1 co 15 min czasu gry |
| `!hatchet` | Szczerbek na ziemi w odległości 1 m | 3 co 60 min czasu gry |
| `!survive` | Do ekwipunku gracza: puszka boczku, butelka wody (pełna), tabletki do oczyszczania wody, bandaż i tetracyklina | 1 co 2 godziny rzeczywiste |
| `!adminkit` | Tylko dla adminów. Na ziemi przed nimi: plecak Alice, M14 z 3 pełnymi magazynkami 20-nabojowymi i 3 pudełkami .308, M4A1 z 3 pełnymi magazynkami 60-nabojowymi i 2 tłumikami, kompletna kamizelka kuloodporna, hełm MICH z noktowizorem i bateriami, kurtka TTsKO w kamuflażu, spodnie i buty, rękawice i nóż myśliwski | brak |

Dwie wbudowane komendy są zawsze dostępne:

| Komenda | Działanie |
|---|---|
| `!list` | Wyświetla wszystko, co gracz może spawnować, po jednej linii na pozycję, z liczbą pozostałych sztuk i (gdy brak) czasem do następnej możliwości |
| `!help` | Wyświetla komendy dostępne dla gracza |

Preset lub zestaw nie mogą nazywać się `list` lub `help`; wbudowane komendy mają priorytet, a w logu serwera pojawia się ostrzeżenie.

### Administratorzy

Umieść ID Steam64 administratorów w sekcji `Admins`:

```json
"Admins": ["76561198000000000"],
```

Administratorzy nie mają limitów i mogą używać komend oznaczonych jako `AdminOnly`. Inni gracze nie widzą komend tylko dla adminów w `!help` ani w `!list`, a wpisanie takiej komendy przez zwykłego gracza nic nie robi.

### Presety

Każdy preset ma swój własny limit (jedna Olga i jedno Ada co 15 minut to osobne przydziały). Aby dodać preset, skopiuj blok w `Presets`, nadaj mu nową `Command` i zrestartuj serwer. Pola:

| Pole | Znaczenie |
|---|---|
| `Command` | Komenda czatu, bez prefiksu (`car` → `!car`) |
| `DisplayName` | Nazwa wyświetlana w wiadomości |
| `ClassName` | Klasa pojazdu lub przedmiotu do spawnu |
| `Attachments` | Klasy mocowane do pojazdu. Powtórz klasę dla każdego slotu, który wypełnia (cztery koła) |
| `DistanceMetres`, `BearingDegrees` | Miejsce spawnu względem gracza. 0° = północ, 90° = wschód |
| `DespawnSeconds` | 0 = nigdy nie zniknie |
| `MaxSpawns`, `LimitMinutes` | Każdy gracz może spawnować to `MaxSpawns` razy co `LimitMinutes` minut czasu gry |
| `AdminOnly` | Tylko administratorzy mogą tego używać |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minuty do zniknięcia |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Tylko pojazdy: jak pełne są płyny na starcie, od 0 do 1 |
| `Invulnerable` | Pojazd i jego elementy nie przyjmują żadnych obrażeń: kolizje, pociski, wybuchy |
| `ProtectCrew` | Tylko pojazdy: nikt w środku nie odnosi obrażeń ani nie ginie przy kolizji |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Wolna przestrzeń (metry) potrzebna w miejscu spawnu. Domyślne pasuje do Olgi; podnieś dla ciężarówek |

### Zestawy (Kits)

Zestaw to zbiór przedmiotów dla jednej komendy. Pola:

| Pole | Znaczenie |
|---|---|
| `Command`, `DisplayName` | Tak jak w presetach |
| `MaxSpawns`, `LimitMinutes` | Limit na gracza |
| `RealTimeLimit` | Licz `LimitMinutes` w czasie rzeczywistym zamiast czasu gry (przeżywa restarty) |
| `AdminOnly` | Tylko administratorzy mogą tego używać |
| `ToInventory` | Umieść przedmioty w ekwipunku gracza tam, gdzie jest miejsce; reszta ląduje na ziemi. W przeciwnym razie wszystko ląduje na ziemi przed graczem |
| `Items` | `ClassName`, `Count` i `Contents`. Zawartość trafia do przedmiotu: mocowana tam, gdzie pasuje (magazynki, tłumiki, baterie, kabury), w przeciwnym razie do jego ładowności, a same mogą mieć własną zawartość |

Przedmioty z zestawu spawnują się jako nowe, magazynki i amunicja pełne, baterie naładowane, butelki pełne wody. Magazynek wymieniony w zawartości broni jest załadowany nabojem w komorze zamka.

Domyślna konfiguracja jest zapisywana tylko wtedy, gdy nie istnieje plik `config.json`. Po aktualizacji moda usuń stary plik (lub dodaj nowe pola do istniejącego), aby załadować nowe domyślne wartości; serwer ostrzeże w logu, jeśli plik pochodzi ze starszej wersji.

Czas gry każdego gracza i historia spawnów są zapisywane w `$profile:ChillSpawn/players/`.

## Dodawanie komend (rozszerzenie)

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

## Znane ograniczenia / testuj najpierw

Podstawowe funkcje działają na DayZ 1.30 experimental. Sprawdź te punkty na serwerze testowym przy zmianie konfiguracji lub aktualizacji gry:

1. **Hook czatu: działa.** Mod odczytuje czat przez `MissionServer.OnEvent` z `ChatMessageEventTypeID`, więc gracze nie muszą nic instalować. [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) wymienia `ChatMessageEventTypeID` jako zdarzenie po stronie klienta, ale na serwerze DayZ 1.30 experimental serwer również je odbiera. Zdarzenie zwraca tylko nazwę nadawcy, więc gracze są dopasowywani po nazwie. Jeśli dwaj gracze mają tę samą nazwę, komenda zostanie zignorowana.
2. **Klasy mocowań i zestawów.** Sprawdź, czy każda klasa w `Attachments` i każdy przedmiot z zestawu spawnują się poprawnie (serwer loguje `Could not attach …`, `Could not create …` lub `Could not put …` dla tych, które nie działają) oraz że pojazd startuje i jedzie.
3. **Ochrona pojazdu.** `Invulnerable` zatrzymuje obrażenia dla samochodu i części, z którymi został spawnowany; części zamontowane później nie są chronione. `ProtectCrew` zatrzymuje obrażenia od kolizji dla osób w środku. Żadne z tych nie chroni ludzi uderzonych przez pojazd. Obie funkcje trzymane są w pamięci, więc po restarcie serwera pojazd, który jeszcze nie zniknął, staje się zwykłym pojazdem.
4. **Sprawdzanie przeszkód.** Używa `IsBoxColliding` z pudełkiem wielkości samochodu, uniesionym 0.3 m nad ziemią, plus sprawdzenie morza/stawu. Próbuje 10 kierunków w odległościach 6, 9 i 12 m. Sprawdź, czy nie traktuje błędnie krzaków jako przeszkód i czy łapie ściany oraz płoty. Nie sprawdza nachylenia terenu, więc pojazd nadal może wylądować na stromym zboczu.
5. **Timery znikania żyją w pamięci.** Jeśli serwer zrestartuje się przed upływem timera pojazdu, pojazd pozostaje w świecie jako zwykły trwały pojazd. Jeśli gracz jest w samochodzie, gdy timer wygaśnie, próba zniknięcia jest powtarzana co minutę, aż do momentu, gdy wysiądzie.

## Budowanie

### Linux

```bash
tools/build-linux.sh
```

To pakuje `dist/@ChillSpawn/addons/ChillSpawn.pbo` za pomocą PBO buildera z [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools to darmowe oprogramowanie na własnej licencji i nie jest ono dołączone tutaj: skrypt klonuje je do `~/.cache/rag-dayz-tools` przy pierwszym uruchomieniu, lub używa ścieżki, którą podasz (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) lub ustawisz w zmiennej `RAG_TOOLS_DIR`. Wymaga `python3` 3.10+ i `git`.

Skrypt uruchamia preflight checks RaG i pakuje skrypty tak, jak są. Pomija binaryzację, konwersję `config.cpp` i podpisywanie, co wymaga programów DayZ Tools na Windowsie. Serwerowy mod tylko ze skryptami nie potrzebuje żadnego z nich. Nic na Linuxie nie kompiluje Enforce Script, więc błędy skryptów pojawiają się tylko w logu `.rpt` serwera przy jego starcie.

### Windows

Pakuj folder `ChillSpawn/` do `@ChillSpawn/addons/ChillSpawn.pbo` używając AddonBuildera z DayZ Tools lub RaG PBO Buildera. Mod-only-serverowe nie wymagają ściśle podpisywania, ale jeśli serwer sprawdza sygnatury, podpisz PBO i umieść `.bikey` w folderze `keys/` serwera.

## Instalacja

1. Skopiuj folder `@ChillSpawn` do głównego folderu serwera, obok `DayZServer_x64.exe`.
2. Wczytaj go jako mod serwera:
   - **Zwykły serwer:** dodaj `-servermod=@ChillSpawn` do linii startowej.
   - **CFTools Architect:** użyj **Add Local Mod**, wybierz `@ChillSpawn` i ustaw jego typ na **Server**.
3. Uruchom serwer. Zapisuje on `profiles/ChillSpawn/config.json` i loguje `[ChillSpawn] Loaded … spawn presets, … kits and … admins.`
4. Dodaj ID Steam64 swoich adminów do `Admins` w tym pliku i zrestartuj serwer.

**Aktualizacja:** zatrzymaj serwer całkowicie przed zastąpieniem `ChillSpawn.pbo`. Działający serwer blokuje plik, a szybki restart może nie dać czasu na jego wymianę.

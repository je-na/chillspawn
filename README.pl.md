# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · **Polski** · [Français](README.fr.md) · [Español](README.es.md) · [Português](README.pt.md)

> Tłumaczenie maszynowe. W razie rozbieżności obowiązuje [angielski oryginał](README.md).

Mod do serwera DayZ. Gracze wpisują `!car` na czacie, aby przywołać czerwony Olęga 24 gotowy do jazdy, umieszczony sześć metrów na północ od nich: z czterema kołami, świecą, radiatorem, naładowaną baterią, reflektorami, drzwiami, maską i bagażnikiem, pełnym zbiornikiem paliwa, płynu chłodniczego, oleju i płynu hamulcowego. Jeśli to miejsce jest zablokowane lub znajduje się w wodzie, skrypt próbuje pobliskich lokalizacji; jeśli żadna nie jest wolna, gracz zostaje poproszony o zmianę miejsca, a próba nie jest doliczana do limitu. Otrzymują prywatną wiadomość informującą, że pojazd zniknie po 15 minutach. Każdy gracz może przywołać jednego Olęga co 15 minut czasu gry (czas połączenia, a nie czas rzeczywisty). Przywołane samochody nie przyjmują obrażeń, a nikomu w środku nie dzieje się krzywda podczas kolizji.

Gracze mogą również wpisywać `!survive` raz na każde 2 godziny czasu rzeczywistego, aby otrzymać mały zestaw przetrwania. Administratorzy nie mają żadnych limitów i otrzymują `!adminkit`.

Klientom nie jest potrzebny ten mod. Ładuj go za pomocą `-servermod=@ChillSpawn`. Stworzony dla DayZ 1.30.

> **Status: przetestowano na DayZ 1.30 experimental.** `!help`, `!list`, `!car`, `!ada` i `!hatchet` działają na żywym serwerze bez klienta z modyfikacją. Administratorzy, zestawy (`kits`), `!survive` oraz ochrona pojazdów to nowe funkcje, które nie zostały jeszcze przetestowane. Zobacz [Znane ograniczenia / testuj najpierw](#znane-ograniczenia--testuj-najpierw). [docs/PROMPTING.md](docs/PROMPTING.pl.md) pokazuje, jak powstał ten mod.

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

Przy pierwszym uruchomieniu mod zapisuje `$profile:ChillSpawn/config.json` (w folderze `-profiles` serwera). Każda pozycja w `Presets` i `Kits` staje się osobnym poleceniem czatu, więc właściciele serwerów mogą dodawać więcej bez dotykania kodu. Domyślne wartości:

| Polecenie | Przywołuje | Limit |
|---|---|---|
| `!car` | Czerwony Olęg 24, gotowy do jazdy, bez obrażeń, załoga chroniona | 1 co 15 min czasu gry |
| `!ada` | Zielona Ada 4x4, to samo (`OffroadHatchback_Blue` / `_White` dla innych kolorów, z pasującymi klasami `_Blue` / `_White` drzwi, maski i bagażnika) | 1 co 15 min czasu gry |
| `!hatchet` | Toporek na ziemi 1 m dalej | 3 co 60 min czasu gry |
| `!survive` | Do ekwipunku gracza: puszka boczniu, pełna butelka wody, tabletki do uzdatniania, opatrunek i tetracyklina | 1 co 2 godziny rzeczywiste |
| `!adminkit` | Tylko dla administratorów. Na ziemi przed nimi: plecak Alice, M14 z 3 pełnymi magazynkami 20-nabojowymi i 3 pudełkami .308, M4A1 z 3 pełnymi magazynkami 60-nabojowymi i 2 tłumikami, kompletna kamizelka kuloodporna, hełm MICH z noktowizją i bateriami, kurtka TTsKO w kamuflażu, spodnie i buty, rękawice i nóż myśliwski | brak |

Dwa wbudowane polecenia są zawsze dostępne:

| Polecenie | Działa |
|---|---|
| `!list` | Wyświetla wszystko, co gracz może przywołać, po jednym na linię, z informacją o liczbie pozostałych i, gdy brak jest zasobów, ile czasu zostało do następnego |
| `!help` | Wyświetla polecenia dostępne dla gracza |

Preset lub kit nie mogą nazywać się `list` ani `help`; wbudowane wygrywają, a w logu serwera pojawia się ostrzeżenie.

### Administratorzy

Umieść ID Steam64 administratorów w `Admins`:

```json
"Admins": ["76561198000000000"],
```

Administratorzy nie mają limitów i mogą używać poleceń oznaczonych jako `AdminOnly`. Inni gracze nie widzą poleceń tylko dla administratorów w `!help` ani `!list`, a jeśli wpiszą jedno, nic się nie stanie.

### Presety

Każdy preset ma własny limit (jeden Olęg i jedna Ada co 15 minut to osobne przydziały). Aby dodać preset, skopiuj blok w `Presets`, nadaj mu nową wartość `Command` i zrestartuj serwer. Pola:

| Pole | Znaczenie |
|---|---|
| `Command` | Polecenie czatu, bez prefiksu (`car` → `!car`) |
| `DisplayName` | Nazwa wyświetlana w wiadomości |
| `ClassName` | Klasa pojazdu lub przedmiotu do przywołania |
| `Attachments` | Klasy dołączane do niego. Powtórz klasę dla każdego slotu, który zajmuje (cztery koła) |
| `DistanceMetres`, `BearingDegrees` | Miejsce przywołania względem gracza. 0° = północ, 90° = wschód |
| `DespawnSeconds` | 0 = nigdy nie znikaj |
| `MaxSpawns`, `LimitMinutes` | Każdy gracz może przywołać to `MaxSpawns` razy co `LimitMinutes` minut czasu gry |
| `AdminOnly` | Tylko administratorzy mogą tego używać |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minuty do zniknięcia |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Tylko pojazdy: jak pełne są płyny na start, od 0 do 1 |
| `Invulnerable` | Pojazd i jego części nie przyjmują żadnych obrażeń: kolizje, pociski, eksplozje |
| `ProtectCrew` | Tylko pojazdy: nikomu w środku nie dzieje się krzywda ani śmierć podczas kolizji |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Wymagana wolna przestrzeń (metry) w miejscu przywołania. Domyślne pasuje do Olęga; zwiększ dla ciężarówek |

### Zestawy (Kits)

Zestaw to zbiór przedmiotów dla jednego polecenia. Pola:

| Pole | Znaczenie |
|---|---|
| `Command`, `DisplayName` | Tak samo jak w presetach |
| `MaxSpawns`, `LimitMinutes` | Limit na gracza |
| `RealTimeLimit` | Licz `LimitMinutes` w czasie rzeczywistym zamiast czasu gry (przetrwaje restarty) |
| `AdminOnly` | Tylko administratorzy mogą tego używać |
| `ToInventory` | Umieść przedmioty w ekwipunku gracza tam, gdzie jest miejsce; reszta ląduje na ziemi. W przeciwnym razie wszystko ląduje na ziemi przed nimi |
| `Items` | `ClassName`, `Count` i `Contents`. Związki trafiają do przedmiotu: dołączone tam, gdzie pasują (magazynki, tłumiki, baterie, kabury), w przeciwnym razie do jego ładunku, a mogą mieć własne zawartości |

Przedmioty z zestawów pojawiają się nowe, magazynki i amunicja pełne, baterie naładowane, a butelki wypełnione wodą. Magazynek wymieniony w zawartości broni jest załadowany nabojem w komorze.

Domyślna konfiguracja jest zapisywana tylko wtedy, gdy nie istnieje plik `config.json`. Po aktualizacji moda usuń stary plik (lub dodaj nowe pola do niego), aby wczytać nowe domyślne wartości; log serwera ostrzega, gdy plik pochodzi ze starszej wersji.

Czas gry i historia przywołań każdego gracza są zapisywane w `$profile:ChillSpawn/players/`.

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

## Znane ograniczenia / testuj najpierw

Podstawowe funkcje działają na DayZ 1.30 experimental. Sprawdź te punkty na serwerze testowym przy zmianie konfiguracji lub aktualizacji gry:

1. **Hook czatu: działa.** Mod odczytuje czat przez `MissionServer.OnEvent` z `ChatMessageEventTypeID`, więc gracze nie muszą nic instalować. [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) wymienia `ChatMessageEventTypeID` jako zdarzenie po stronie klienta, ale na serwerze DayZ 1.30 experimental serwer również je otrzymuje. Zdarzenie podaje tylko imię nadawcy, więc gracze są dopasowywani po nazwie. Jeśli dwaj gracze mają tę samą nazwę, polecenie jest ignorowane.
2. **Nazwy klas przyłączeń i zestawów.** Sprawdź każdą klasę w `Attachments` i każdy przedmiot z zestawu (log serwera drukuje `Could not attach …`, `Could not create …` lub `Could not put …` dla tych, które nie zadziałają), oraz że samochód się uruchamia i jeździ.
3. **Ochrona pojazdu (nowe).** `Invulnerable` blokuje obrażenia dla samochodu i części, z którymi został przywołany; części dołączone później nie są chronione. `ProtectCrew` blokuje obrażenia od kolizji dla osób w środku. Żadna z nich nie chroni ludzi uderzonych przez samochód. Obie funkcje są przechowywane w pamięci, więc po restarcie serwera samochód, który jeszcze nie zniknął, staje się zwykłym pojazdem.
4. **Sprawdzanie przeszkód.** Używa `IsBoxColliding` z polem rozmiaru samochodu, podniesionym 0,3 m nad ziemię, plus sprawdzanie morza/stawu. Próbuje 10 kierunków w odległościach 6, 9 i 12 m. Sprawdź, czy nie traktuje błędnie krzaków jako przeszkód i czy wychwytuje ściany i płoty. Nie sprawdza nachylenia terenu, więc samochód może nadal wylądować na stromym zboczu.
5. **Timery znikania żyją w pamięci.** Jeśli serwer zostanie zrestartowany przed upływem czasu znikania samochodu, pojazd pozostaje w świecie jako zwykły trwały pojazd. Jeśli gracz jest w samochodzie, gdy czas znikania minie, próba zniknięcia jest ponawiana co minutę, aż wysiądzie.

## Budowanie

### Linux

```bash
tools/build-linux.sh
```

Spakowuje to `dist/@ChillSpawn/addons/ChillSpawn.pbo` przy użyciu kreatora PBO z [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools jest darmowym oprogramowaniem na własnej licencji i nie jest dołączone tutaj: skrypt klonuje go do `~/.cache/rag-dayz-tools` przy pierwszym uruchomieniu, lub używa ścieżki, którą podasz (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) lub ustawisz w `RAG_TOOLS_DIR`. Wymaga `python3` 3.10+ i `git`.

Skrypt uruchamia kontrole wstępne RaG i pakuje skrypty tak, jak są. Pomija binaryzację, konwersję `config.cpp` i podpisywanie, co wymaga programów DayZ Tools na Windowsie. Mod do serwera tylko ze skryptami nie potrzebuje żadnego z nich. Nic na Linuxie nie kompiluje Enforce Script, więc błędy w skryptach pojawiają się dopiero w logu `.rpt` serwera przy jego starcie.

Skopiuj `dist/@ChillSpawn` do folderu serwera i dodaj `-servermod=@ChillSpawn` do linii startowej.

### Windows

Spakuj folder `ChillSpawn/` do `@ChillSpawn/addons/ChillSpawn.pbo` przy użyciu AddonBuildera z DayZ Tools lub kreatora PBO RaG. Mody tylko dla serwera nie wymagają ściśle podpisywania, ale jeśli serwer sprawdza podpisy, podpisz PBO i umieść `.bikey` w folderze `keys/` serwera.

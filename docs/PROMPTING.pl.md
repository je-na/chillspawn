# Jak powstał ChillSpawn z użyciem Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · **Polski** · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Tłumaczenie maszynowe. W razie rozbieżności obowiązuje [angielski oryginał](PROMPTING.md).

ChillSpawn został napisany w jednej sesji Claude Code (Claude Opus 5.5) na maszynie Linux bez zainstalowanej gry DayZ. Claude napisał każdy plik. Osoba nadzorująca proces formułowała wymagania, prosiła o zmiany i porównywała wynik z dwoma lokalnymi modelami. Żaden kod nie był kompilowany ani uruchamiany w grze podczas sesji, więc należy traktować go jako solidny pierwszy szkic, który nadal wymaga testów na serwerze.

Ta strona przedstawia żądania w kolejności (przefrazowane i uporządkowane, a nie cytowane słowo w słowo), to, co każda z nich zmieniła oraz wnioski, które wyciągnięto. Zakończona jest promptami, które można wykorzystać do stworzenia podobnego moda.

## Prompty, w kolejności

### 1. Pierwsze żądanie

> Napisz serwerowy mod do DayZ, który pozwala graczowi przywołać samochód za pomocą komendy czatu:
>
> - Samochodem jest czerwona Olga z czterema kołami, świecą zapłonową i radiatorem.
> - Pojawia się 6 metrów na północ od gracza.
> - Gracz otrzymuje prywatną wiadomość informującą, że samochód zniknie w ciągu 15 minut.
> - Każdy gracz może przywołać 3 samochody na godzinę czasu rozgrywki.
> - Uwzględnij framework, który pozwoli na późniejsze rozszerzanie moda.
> - Konfiguracja JSON pozwala właścicielowi serwera wybrać, co ma się pojawić: samochód wraz z jego elementami dodatkowymi czy przedmiot.

Claude wygenerował cały mod: `config.cpp`, podział skryptów na 3_Game/4_World/5_Mission, konfigurację JSON zapisywaną do `$profile:` przy pierwszym uruchomieniu, rejestr komend (tzw. "framework"), śledzenie czasu gry zapisywane dla każdego gracza oraz plik README. Zaznaczył również elementy, co do których miał wątpliwości, zamiast ich ukrywać:

- że serwer odbiera czat poprzez `MissionServer.OnEvent` z `ChatMessageEventTypeID`, otrzymując jedynie imię nadawcy;
- że Olga wyposażona tylko w koła, świecę zapłonową i radiator **nie będzie jechać** (brak akumulatora i płynów);
- że timery odliczające czas znikania są tracone przy restarcie;
- że nie sprawdzano przeszkód.

**Wniosek:** numerowana lista konkretnych wymagań (dokładny pojazd, dystans, kierunek, limity) daje kompletną odpowiedź za jednym razem. Poproś Claude'a o wypisanie jego założeń oraz rzeczy, których nie mógł zweryfikować.

### 2. Sprawienie, że pojazd jest jezdny i sprawdzenie przeszkód

> Dodaj wszystko, czego potrzebuje samochód, aby po pojawieniu się był gotowy do jazdy. Sprawdź również przeszkody przed przywołaniem pojazdu.

Dodano akumulator (naładowany), reflektory, drzwi, maskę i bagażnik oraz napełniono paliwo, płyn chłodzący, olej i płyn hamulcowy (wszystko konfigurowalne). Sprawdzenie przeszkód używa `IsBoxColliding` z pudełkiem wielkości samochodu oraz sprawdzeniem morza/stawu. Próbuje 10 kierunków w trzech dystansach. Jeśli nie ma wolnego miejsca, gracz jest o tym informowany, a próba nie jest liczona do limitu.

**Wniosek:** uzupełnienia reagujące na własną listę "znanych ograniczeń" Claude'a są tanie i celne.

### 3. Więcej niż jeden przywoływany obiekt

> Pokaż w przykładzie konfiguracji kilka różnych obiektów do przywołania, np. siekierkę i Adę 4x4.

Konfiguracja już to obsługiwała (każdy preset to osobna komenda czatu). Claude dodał presety `!ada` i `!hatchet` zarówno do domyślnych wartości, jak i do `config.example.json`, oraz udokumentował sposób dodawania kolejnych.

### 4. Porównanie z lokalnymi modelami

> Niech mój lokalny model Qwen napisze ten sam mod, a następnie porównaj jego wersję z moją: jak bardzo się różnią i czy któryś zawiera błędy?

Claude wysłał te same wymagania do lokalnego modelu Qwen, a później przeanalizował drugą próbę Qwen dostarczoną przez użytkownika. Żaden z nich nie zadziałałby poprawnie:

| | Lokalny Qwen #1 | Lokalny Qwen #2 |
|---|---|---|
| Rzeczywiste API DayZ | Większość wymyślona (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Większość prawdziwa |
| Załadowałby się | Nie: brak modułów skryptów, nie podłącza `MissionServer` | Nie: `config.cpp` nie ma `CfgMods` |
| Skompilowałby się | Nie | Nie: operator trójargumentowy, iteratory w stylu C++, `ToLower()` użyte jak zwracające string |
| "Północ" | 6 m w górę (w powietrzu) | 6 m na południe |
| Limit częstotliwości | W pamięci, stałe bloki | Czas działania serwera, tracony przy restarcie |

**Wniosek:** dla Enforce Script, ogólnego przeznaczenia modele pewnie piszą konstrukcje w stylu C++/Java, których DayZ nie obsługuje. Uzyskaj drugą opinię, prosząc jeden model o recenzję kodu innego modelu względem checklisty (załadowanie, kompilacja, zachowanie), a nie polegając na którymkolwiek z nich.

### 5. Pożyczenie dobrego pomysłu

> Wprowadź do ChillSpawn pomysł z listy pojazdów/przedmiotów z wersji drugiego Qwen.

Qwen #2 miał podkomendę `/vspawn list`. Claude dodał `!list` (każdy preset z liczbą pozostałych przywołań gracza w tej godzinie) oraz `!help` jako wbudowane komendy w frameworku ChillSpawn.

### 6. Publikacja

> Zcommituj to do mojego publicznego repozytorium GitHub (licencja MIT; zastąp README jeśli chcesz). Upewnij się, że żadne dane uwierzytelniające nie trafią do środka. Dodaj dokumentację pokazującą, jak prowadzono promptowanie i jakie prompty ktoś inny mógłby wykorzystać do stworzenia podobnego moda.

To zaowocowało tym repozytorium i tą stroną.

### 7. Sprawdzenie względem aktualizacji 1.30

> Sprawdź notatki do patcha 1.30 oraz tracker feedbacku Bohemii pod kątem wszystkiego, co może zepsuć naszą logikę.

Nic w notatkach nie dotyczyło API używanego przez moda. Wersja 1.30 wprowadziła bardziej restrykcyjny kompilator skryptów, więc zmieniono dwie konstrukcje, które mogłyby zostać odrzucone (metoda zadeklarowana bez ciała oraz konwersja float na int).

### 8. Budowanie na Linuxie

> Sprawdź, czy RaG DayZ Tools mogą to zbudować na Linuxie.

RaG DayZ Tools są nastawione na Windows, ale ich budowniczy PBO to czysty Python i działa on na Linuxie. `tools/build-linux.sh` używa go do pakowania moda. Binarizacja, konwersja konfiguracji i podpisywanie nadal wymagają programów z systemu Windows, ale mod serwerowy zawierający tylko skrypty ich nie potrzebuje.

## Prompty, które możesz ponownie wykorzystać

### Rozpoczęcie nowego modu serwerowego

Skopiuj to, wypełnij nawiasy i wklej do Claude'a (Claude Code działa najlepiej, ponieważ sam pisze pliki):

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

Linia "Enforce Script has no…" pochodzi bezpośrednio z błędów popełnionych przez lokalne modele. Jej pozostawienie nie kosztuje nic, a zapobiega najczęstszym błędom.

### Uzupełnienia, które działały dobrze

- "Make the vehicle spawn fully drivable: every part it needs, charged battery and all fluids full."
- "Add an obstacle check before spawning. If the spot is blocked or in water, try nearby spots; if none is clear, tell the player and don't count it against their limit."
- "Add a `!list` command showing every spawnable preset and how many the player has left."
- "Review this code [paste or attach] for things that won't compile or load in DayZ, and for logic errors against these requirements: [list]."
- "Write a test plan I can follow on a local server to check each requirement."

### Wskazówki

- **Bądź precyzyjny co do obiektów w grze.** "Czerwona Olga" stała się `CivilianSedan_Wine`. Jeśli znasz nazwy klas, podaj je; jeśli nie, poproś Claude'a o wypisanie tych, których użył, abyś mógł je sprawdzić w `types.xml` serwera lub za pomocą narzędzia admina.
- **Określ, co oznacza "na godzinę".** "Na godzinę czasu rozgrywki" i "na godzinę czasu serwera" to różne funkcje, a tylko jedna z nich przetrwa ponowne połączenie.
- **Poproś o sekcję "znanych ograniczeń"** i traktuj ją jako swoją listę testową.
- **Testuj na lokalnym serwerze przed udostępnieniem.** Obserwuj log `.rpt` serwera pod kątem błędów skryptów oraz linii typu `[ChillSpawn] Could not attach …`.
- **Na Windows z zainstalowanymi DayZ Tools**, wtyczka [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) dla Claude'a Code dodaje agentów i umiejętności specyficznych dla DayZ, które budują PBO i uruchamiają lokalny serwer testowy, co zamyka lukę "nieprzetestowanego" kodu.

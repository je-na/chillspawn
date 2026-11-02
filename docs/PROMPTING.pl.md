# Jak powstał ChillSpawn z użyciem Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · **Polski** · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Tłumaczenie maszynowe. W razie rozbieżności obowiązuje [angielski oryginał](PROMPTING.md).

ChillSpawn został napisany w jednej sesji Claude Code (Claude Opus 5.5) na maszynie z Linuksem, bez zainstalowanego DayZ. Claude napisał każdy plik. Osoba kierująca pracą przygotowała wymagania, prosiła o zmiany i porównywała wynik z dwoma lokalnymi modelami. Pierwsza wersja została napisana bez kompilowania ani uruchamiania czegokolwiek. Później w tej samej sesji ustawiono serdedykowany serwer Windows z DayZ 1.30 experimental, Claude połączył się przez SSH i wdrożył modyfikację, przetestowano ją w grze i rozszerzono.

Ta strona przedstawia żądania w kolejności (przefrazowane i uporządkowane, nie cytowane słowo w słowo), co każda z nich zmieniła oraz czego się nauczyliśmy. Kończą ją prompty, które możesz wykorzystać do stworzenia podobnego moda.

## Prompty, w kolejności

### 1. Pierwsze żądanie

> Napisz serwerowy mod do DayZ, który pozwala graczowi spawnować samochód za pomocą komendy czatu:
>
> - Samochodem ma być czerwona Olga, z czterema kołami, świecą zapłonową i chłodnicą zamontowanymi.
> - Spawnuje się 6 metrów na północ od gracza.
> - Gracz otrzymuje prywatną wiadomość informującą, że samochód zostanie usunięty w ciągu 15 minut.
> - Każdy gracz może spawnować 3 auta na godzinę spędzoną w grze.
> - Uwzględnij szkielet (framework), dzięki któremu mod będzie można łatwo rozszerzać.
> - Konfiguracja JSON pozwala właścicielowi serwera wybrać, co ma się spawnować: samochód wraz z jego akcesoriami lub przedmiot.

Claude wygenerował cały mod: `config.cpp`, podział skryptów na 3_Game/4_World/5_Mission, konfigurację JSON zapisywaną do `$profile:` przy pierwszym uruchomieniu, rejestr komend (tzw. "framework"), śledzenie czasu gry zapisywane dla każdego gracza oraz plik README. Oznaczył też rzeczy, których był niepewny, zamiast je ukrywać:

- że serwer odbiera czat przez `MissionServer.OnEvent` z `ChatMessageEventTypeID`, i otrzymuje tylko imię nadawcy;
- że Olga posiadająca tylko koła, świecę zapłonową i chłodnicę **nie będzie jeździć** (brak akumulatora i płynów);
- że timery despawnu są tracone przy restarcie;
- że nie było sprawdzania przeszkód.

**Lekcja:** liczona lista konkretnych wymagań (dokładny pojazd, odległość, kierunek, limity) daje kompletny wynik za jednym razem. Poproś Claude'a o wypisanie jego założeń i wszystkiego, czego nie mógł zweryfikować.

### 2. Sprawienie, by samochód dało się prowadzić i sprawdzenie przeszkód

> Dodaj wszystko, czego potrzebuje samochód, aby spawnował się gotowy do jazdy. Sprawdź też przeszkody przed spawnem.

Dodano akumulator (naładowany), reflektory, drzwi, maskę i bagażnik oraz uzupełniono paliwo, chłodnicę, olej i płyn hamulcowy (wszystko konfigurowalne). Sprawdzenie przeszkód używa `IsBoxColliding` z pudełkiem wielkości samochodu plus sprawdzenie morza/stawu. Próbuje 10 kierunków w trzech odległościach. Jeśli nic nie jest wolne, gracz zostaje poinformowany, a próba nie jest liczona do limitu.

**Lekcja:** doprecyzowania reagujące na własną listę "znanych ograniczeń" Claude'a są tanie i ukierunkowane.

### 3. Więcej niż jedna rzecz do spawnu

> Pokaż w przykładowej konfiguracji kilka różnych rzeczy do spawnu, np. siekierę i Adę 4x4.

Konfiguracja już to obsługiwała (każdy preset to osobna komenda czatu). Claude dodał presety `!ada` i `!hatchet` zarówno do domyślnych, jak i `config.example.json`, oraz udokumentował sposób dodawania kolejnych.

### 4. Porównanie z lokalnymi modelami

> Niech mój lokalny model Qwen napisze ten sam mod, a następnie porównaj jego wersję ze swoją: jak bardzo się różnią i czy którykolwiek zawiera błędy?

Claude wysłał te same połączone wymagania do lokalnego modelu Qwen, a później przejrzał drugą próbę Qwen dostarczoną przez użytkownika. Żadna z nich nie zadziałałaby:

| | Lokalny Qwen #1 | Lokalny Qwen #2 |
|---|---|---|
| Rzeczywiste API DayZ | Większość wymyślona (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Większość prawdziwa |
| Załadowałby się | Nie: brak modułów skryptów, nie podłącza `MissionServer` | Nie: `config.cpp` nie ma `CfgMods` |
| Skompilowałby się | Nie | Nie: operator trójargumentowy, iteratory w stylu C++, `ToLower()` użyte tak, jakby zwracało stringa |
| "Północ" | 6 m w powietrzu | 6 m na południe |
| Limit szybkości | W pamięci, stałe bloki | Czas działania serwera, tracone przy restarcie |

**Lekcja:** dla Enforce Script ogólnego przeznaczenia modele pewnie piszą C++/Java-izmy, których DayZ nie obsługuje. Zdobądź drugą opinię, prosząc jeden model o recenzję kodu innego modelu względem listy kontrolnej (załadowanie, kompilacja, zachowanie), a nie polegając na którymkolwiek z nich.

### 5. Pożyczenie dobrego pomysłu

> Wprowadź pomysł z drugiej wersji Qwen dotyczący listy pojazdów/przedmiotów do ChillSpawn.

Qwen #2 miało podkomendę `/vspawn list`. Claude dodał `!list` (każdy preset z liczbą spawnów, które gracz ma jeszcze do dyspozycji w tej godzinie) i `!help` jako wbudowane komendy w frameworku ChillSpawn.

### 6. Publikacja

> Zcommituj to do mojego publicznego repozytorium GitHub (licencja MIT; zamień README, jeśli chcesz). Upewnij się, że żadne dane uwierzytelniające nie trafią do środka. Dodaj dokumentację pokazującą, jak prowadzono promptowanie i jakie prompty ktoś inny mógłby użyć do stworzenia podobnego moda.

To doprowadziło do powstania tego repozytorium i tej strony.

### 7. Sprawdzenie pod kątem aktualizacji 1.30

> Sprawdź notatki do patcha 1.30 i tracker feedbacku Bohemii pod kątem wszystkiego, co mogłoby zepsuć naszą logikę.

Nic w notatkach nie dotykało API używanego przez moda. 1.30 przyniosło bardziej rygorystyczny kompilator skryptów, więc dwie konstrukcje, które mógł odrzucić, zostały zmienione (metoda zadeklarowana bez ciała oraz konwersja float na int).

### 8. Budowanie na Linuxie

> Sprawdź, czy RaG DayZ Tools może to zbudować na Linuxie.

RaG DayZ Tools jest nastawiony na Windowsa, ale jego budownik PBO to zwykły Python i działa na Linuxie. `tools/build-linux.sh` używa go do spakowania moda. Binaryzacja, konwersja konfiguracji i podpisywanie wciąż wymagają programów z Windowsa, a mod serwerowy tylko ze skryptami ich nie potrzebuje.

### 9. Testowanie na prawdziwym serwerze

> Ustawiłem dedykowany serwer Windows z CFTools Architect i otworzyłem SSH dla Ciebie. Wdroż tam moda i sprawdź, czy działa.

Serwer uruchamiał DayZ 1.30 experimental, zarządzany przez CFTools Architect. Przez SSH Claude:

- skopiował własny `scripts.pbo` gry z serwera, rozpakował go i sprawdził każdą funkcję wywoływaną przez moda względem prawdziwych skryptów 1.30 experimental. Wszystkie istniały z pasującymi parametrami;
- zbudował PBO na Linuxie, skopiował `@ChillSpawn` do folderu głównego serwera i poprosił właściciela o dodanie go w Architect jako **Add Local Mod**, typ **Server**;
- odczytał logi `.rpt` i skryptów po każdym restarcie, aby potwierdzić, że skrypty się skompilowały i mod się załadował.

`!help` i `!car` zadziałały za pierwszym razem. Hook czatu, część najbardziej podatna na błędy, działa na serwerze bez moda klienckiego.

**Lekcje:**

- **Daj Claude'owi prawdziwe skrypty gry.** Rozpakowanie `scripts.pbo` z zainstalowanej gry zamienia "to API pewnie istnieje" w szybkie sprawdzenie i rozstrzyga pytania, które wiki potrafią źle opisywać.
- **Wymień PBO, gdy serwer jest całkowicie zatrzymany.** Działający serwer blokuje plik moda, a restart w Architect jest zbyt szybki, by to wychwycić.

### 10. Administratorzy, limity, chronione auta i zestawy

> Pozwól administratorom spawnować ile chcą. Ogranicz graczy do jednego z każdego auta co 15 minut. Czy spawnowane auta można zrobić niezniszczalne, lub sprawić, by wypadki nie raniły osób w środku, bez moda klienckiego? Dodaj admin-only `!adminkit` z [listą broni, pancerza i ubrań] oraz zestaw `!survive`, którego gracze mogą użyć raz na 2 godziny rzeczywiste z [jedzeniem, wodą, przedmiotami medycznymi].

Claude najpierw przeczytał kod aut w 1.30. Uszkodzenia załogi przy wypadku są stosowane przez skrypt serwerowy (`CarScript.DamageCrew`), więc mod serwerowy może to wyłączyć. Uszkodzenia samego auta wyłącza się przez `SetAllowDamage(false)`. Następnie dodał:

- listę `Admins` z identyfikatorami Steam64, bez limitów i dostępem do komend tylko dla adminów, niewidoczną dla innych graczy;
- limity jako `MaxSpawns` na `LimitMinutes`, liczone w czasie gry, lub w czasie rzeczywistym dla zestawów, które o to proszą;
- opcje `Invulnerable` i `ProtectCrew` dla presetów;
- zestawy: zagnieżdżone przedmioty (broń z magazynkiem i tłumikiem, plecak z jego zawartością), pełne magazynki i woda, dostarczane do ekwipunku lub na ziemię.

Wszystko zadziałało w grze przy pierwszym wdrożeniu.

**Lekcja:** pytanie "czy to da się zrobić po stronie serwera?" przed prośbą o funkcję daje odpowiedź opartą na rzeczywistym kodzie gry, wraz z jej ograniczeniami: tutaj jest to fakt, że osoby uderzane przez auto nadal są ranné, a ochrona znika przy restarcie.

## Prompty do ponownego użycia

### Rozpoczęcie nowego serwerowego moda

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

### Doprecyzowania, które zadziałały dobrze

- "Spraw, by pojazd spawnował się w pełni jezdny: każda część, której potrzebuje, naładowany akumulator i pełne płyny."
- "Dodaj sprawdzenie przeszkód przed spawnem. Jeśli miejsce jest zablokowane lub w wodzie, spróbuj pobliskich miejsc; jeśli żadne nie jest wolne, poinformuj gracza i nie licz tego do limitu."
- "Dodaj komendę `!list` pokazującą każdy preset do spawnu i ile zostało graczowi."
- "Przejrzyj ten kod [wklej lub załącz] pod kątem rzeczy, które się nie skompilują ani nie załadują w DayZ, oraz błędów logicznych względem tych wymagań: [lista]."
- "Napisz plan testowy, którego mogę przestrzegać na lokalnym serwerze, aby sprawdzić każde wymaganie."
- "Oto dostęp SSH do mojego serwera. Sprawdź każde API używane przez moda względem własnych skryptów gry przed wdrożeniem."
- "Czy [funkcja] da się zrobić tylko po stronie serwera? Sprawdź skrypty gry i powiedz mi, czego nie obejmuje."

### Wskazówki

- **Bądź dokładny co do obiektów w grze.** "Czerwona Olga" stała się `CivilianSedan_Wine`. Jeśli znasz nazwy klas, podaj je; jeśli nie, poproś Claude'a o wypisanie tych, których użył, żebyś mógł je sprawdzić w `types.xml` serwera lub za pomocą narzędzia admina.
- **Określ, co oznacza "na godzinę".** "Na godzinę gry" i "na godzinę czasu serwera" to różne funkcje, a tylko jedna z nich przetrwa ponowne połączenie.
- **Poproś o sekcję "znane ograniczenia"** i traktuj ją jako swoją listę testową.
- **Testuj na lokalnym serwerze przed przejściem na produkcję.** Obserwuj log serwera `.rpt` pod kątem błędów skryptów i linii typu `[ChillSpawn] Could not attach …`.
- **Daj Claude'owi dostęp SSH do testowego serwera**, jeśli możesz. Wtedy może wdrożyć, odczytać logi i sprawdzić własne skrypty gry, co zamienia zgadywanie w sprawdzanie.
- **Na Windowsie z zainstalowanym DayZ Tools** plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) dla Claude'a Code dodaje agentów i umiejętności specyficznych dla DayZ, które budują PBO i uruchamiają lokalny serwer testowy, co zamyka lukę "nieprzetestowane".

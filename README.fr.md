# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · **Français** · [Español](README.es.md) · [Português](README.pt.md)

> Traduction automatique. En cas de différence, l'[original anglais](README.md) fait foi.

Un mod côté serveur pour DayZ. Les joueurs tapent `!car` dans le chat pour faire apparaître une Olga 24 rouge prête à être conduite, située six mètres au nord d'eux : quatre roues, bougie d'allumage, radiateur, batterie chargée, phares, portes, capot et coffre, avec du carburant, du liquide de refroidissement, de l'huile et du liquide de frein à niveau plein. Si cet emplacement est bloqué ou dans l'eau, le mod essaie des emplacements voisins ; si aucun n'est libre, le joueur reçoit l'instruction de bouger et la tentative ne compte pas contre sa limite. Il reçoit un message privé indiquant que le véhicule disparaîtra au bout de 15 minutes. Chaque joueur peut faire apparaître 3 véhicules par heure de temps de jeu (temps passé connecté, et non pas en temps réel).

Les clients n'ont pas besoin du mod. Chargez-le avec `-servermod=@ChillSpawn`. Écrit pour DayZ 1.30.

> **Statut : non testé.** Le code a été rédigé avec Claude et n'a encore ni été compilé ni exécuté sur un serveur. Consultez [Limites connues / tester d'abord](#limites-connues--tester-dabord) avant de l'utiliser sur un serveur en ligne. [docs/PROMPTING.md](docs/PROMPTING.fr.md) explique comment il a été créé.

## Structure

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

## Configuration

Au premier démarrage, le mod écrit `$profile:ChillSpawn/config.json` (dans le dossier `-profiles` du serveur). Chaque préconfiguration dans `Presets` devient sa propre commande de chat, permettant aux propriétaires de serveur d'ajouter plus de véhicules ou d'objets sans toucher au code. La configuration par défaut en contient trois :

| Commande | Fait apparaître |
|---|---|
| `!car` | Olga 24 rouge, prête à conduire |
| `!ada` | Ada 4x4 verte, prête à conduire (`OffroadHatchback_Blue` / `_White` pour d'autres couleurs, avec les classes de porte, capot et coffre correspondantes `_Blue` / `_White`) |
| `!hatchet` | Une hache posée au sol à 1 mètre |

Deux commandes intégrées sont toujours disponibles :

| Commande | Fait |
|---|---|
| `!list` | Liste chaque préconfiguration, une par ligne, avec le nombre d'apparitions restantes pour le joueur cette heure de jeu et, lorsqu'il n'en reste plus, combien de temps avant la prochaine |
| `!help` | Liste toutes les commandes |

Une préconfiguration ne peut pas s'appeler `list` ou `help` ; les intégrées l'emportent et le journal du serveur affiche un avertissement.

Chaque préconfiguration a sa propre limite (3 Olgas et 3 haches par heure sont des allocations séparées). Pour ajouter une préconfiguration, copiez un bloc dans `Presets`, attribuez-lui une nouvelle `Command` et redémarrez le serveur. Champs :

| Champ | Signification |
|---|---|
| `Command` | Commande de chat, sans le préfixe (`car` → `!car`) |
| `DisplayName` | Nom affiché dans le message |
| `ClassName` | Classe du véhicule ou de l'objet à faire apparaître |
| `Attachments` | Classes attachées à celui-ci. Répétez une classe pour chaque emplacement qu'elle occupe (quatre roues) |
| `DistanceMetres`, `BearingDegrees` | Endroit où il apparaît par rapport au joueur. 0° = nord, 90° = est |
| `DespawnSeconds` | 0 = ne jamais disparaître |
| `MaxPerPlaytimeHour` | Limite d'apparition par heure de jeu, comptée par préconfiguration |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutes avant disparition |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Véhicules uniquement : niveau initial de chaque fluide, de 0 à 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espace dégagé (en mètres) nécessaire au point d'apparition. La valeur par défaut convient pour une Olga ; augmentez-la pour les camions |

La configuration par défaut n'est écrite que s'il n'existe pas encore de `config.json`. Après avoir mis à jour le mod, supprimez l'ancien fichier (ou ajoutez les nouveaux champs) pour prendre en compte les nouvelles valeurs par défaut.

Le temps de jeu et l'historique d'apparition par joueur sont enregistrés dans `$profile:ChillSpawn/players/`.

## Ajout de commandes (extension)

Sous-classez `CS_ChatCommand` et enregistrez-le depuis un autre mod, ou dans celui-ci :

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

## Limites connues / tester d'abord

Pas encore compilé ou testé. Vérifiez ces points sur un serveur de test en premier :

1. **Hook de chat (le plus susceptible d'échouer).** Le mod lit le chat via `MissionServer.OnEvent` avec `ChatMessageEventTypeID`, donc les joueurs n'ont rien à installer. La [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) liste `ChatMessageEventTypeID` comme un événement côté client, et son tutoriel sur les commandes de chat utilise plutôt un petit mod client. Testez ceci en premier : si `!help` ne reçoit pas de réponse, le serveur ne reçoit pas les événements de chat, et le mod aura besoin d'une partie côté client. L'événement donne uniquement le nom de l'expéditeur, donc les joueurs sont appariés par leur nom. Si deux joueurs partagent un nom, la commande est ignorée.
2. **Noms des classes d'accessoires.** Vérifiez que chaque classe dans `Attachments` s'attache (le journal du serveur affiche `Could not attach …` pour celles qui ne le font pas), et que la voiture démarre et roule.
3. **Vérification des obstacles.** Utilise `IsBoxColliding` avec une boîte de la taille de la voiture, surélevée de 0,3 m du sol, plus une vérification mer/étang. Il essaie 10 directions à 6, 9 et 12 m. Vérifiez qu'il ne traite pas faussement les buissons comme des obstacles, et qu'il détecte bien les murs et les clôtures. Il ne vérifie pas la pente, donc une voiture peut toujours atterrir sur une colline raide.
4. **Les minuteurs de disparition vivent en mémoire.** Si le serveur redémarre avant que le minuteur d'une voiture ne se termine, la voiture reste dans le monde en tant que véhicule persistant normal. Si un joueur est dans la voiture lorsque le minuteur expire, la disparition est réessayée toutes les minutes jusqu'à ce qu'il descende.

## Construction

### Linux

```bash
tools/build-linux.sh
```

Cela packe `dist/@ChillSpawn/addons/ChillSpawn.pbo` avec le constructeur PBO de [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools est un logiciel gratuit sous sa propre licence et n'est pas inclus ici : le script le clone dans `~/.cache/rag-dayz-tools` lors de la première exécution, ou utilise le chemin que vous passez (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) ou définissez dans `RAG_TOOLS_DIR`. Nécessite `python3` 3.10+ et `git`.

Le script exécute les vérifications préliminaires de RaG et packe les scripts tels quels. Il saute la binarisation, la conversion `config.cpp` et la signature, qui nécessitent les programmes DayZ Tools sous Windows. Un mod serveur uniquement en scripts n'en a besoin d'aucun. Rien sur Linux ne compile Enforce Script, donc les erreurs de script n'apparaissent que dans le journal `.rpt` du serveur au démarrage.

Copiez `dist/@ChillSpawn` dans le dossier du serveur et ajoutez `-servermod=@ChillSpawn` à la ligne de lancement.

### Windows

Packez le dossier `ChillSpawn/` dans `@ChillSpawn/addons/ChillSpawn.pbo` avec l'AddonBuilder de DayZ Tools ou le RaG PBO Builder. Les mods uniquement serveur n'ont pas strictement besoin d'être signés, mais si le serveur vérifie les signatures, signe le PBO et place le `.bikey` dans le dossier `keys/` du serveur.

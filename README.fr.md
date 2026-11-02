# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · **Français** · [Español](README.es.md) · [Português](README.pt.md)

> Traduction automatique. En cas de différence, l'[original anglais](README.md) fait foi.

Un mod DayZ côté serveur. Les joueurs tapent `!car` dans le chat pour faire apparaître une Olga 24 rouge prête à être conduite, six mètres au nord d'eux : quatre roues, bougie d'allumage, radiateur, batterie chargée, phares, portes, capot et coffre, avec un plein de carburant, liquide de refroidissement, huile et liquide de frein. Si cet emplacement est bloqué ou dans l'eau, le mod essaie des emplacements proches ; si aucun n'est libre, le joueur est invité à bouger et la tentative ne compte pas contre sa limite. Il reçoit un message privé indiquant que le véhicule disparaîtra au bout de 15 minutes. Chaque joueur peut faire apparaître une Olga toutes les 15 minutes de temps de jeu (temps passé connecté, pas le temps réel). Les voitures spawnées ne prennent aucun dommage, et personne à l'intérieur n'est blessée lors d'un crash.

Les joueurs peuvent aussi taper `!survive` une fois toutes les 2 heures réelles pour obtenir un petit kit de survie. Les administrateurs n'ont aucune limite et obtiennent un `!adminkit`.

Les clients n'ont pas besoin du mod. Chargez-le avec `-servermod=@ChillSpawn`. Écrit pour DayZ 1.30.

> **Statut : testé sur DayZ 1.30 experimental.** `!help`, `!list`, `!car`, `!ada` et `!hatchet` fonctionnent sur un serveur live sans mod client. Les admins, les kits, `!survive` et la protection des voitures sont nouveaux et n'ont pas encore été testés. Voir [Limites connues / tester d'abord](#limites-connues--tester-dabord). [docs/PROMPTING.md](docs/PROMPTING.fr.md) explique comment ce mod a été créé.

## Structure

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

## Configuration

Au premier démarrage, le mod écrit `$profile:ChillSpawn/config.json` (dans le dossier `-profiles` du serveur). Chaque entrée dans `Presets` et `Kits` devient sa propre commande de chat, afin que les propriétaires de serveur puissent en ajouter sans toucher au code. Les valeurs par défaut :

| Commande | Fait apparaître | Limite |
|---|---|---|
| `!car` | Olga 24 rouge, prête à conduire, sans dommage, équipage protégé | 1 toutes les 15 min de jeu |
| `!ada` | Ada 4x4 verte, identique (`OffroadHatchback_Blue` / `_White` pour d'autres couleurs, avec les classes de porte, capot et coffre correspondantes `_Blue` / `_White`) | 1 toutes les 15 min de jeu |
| `!hatchet` | Une hache au sol à 1 mètre | 3 toutes les 60 min de jeu |
| `!survive` | Dans l'inventaire du joueur : une boîte de bacon, une bouteille d'eau pleine, comprimés de purification, un bandage et de la tétracycline | 1 toutes les 2 heures réelles |
| `!adminkit` | Réservé aux admins. Au sol devant eux : sac Alice, M14 avec 3 chargeurs pleins de 20 coups et 3 boîtes de .308, M4A1 avec 3 chargeurs pleins de 60 coups et 2 silencieux, plastron complet, casque MICH avec vision nocturne et piles, veste TTsKO camouflage, pantalon et bottes, gants et couteau de chasse | aucune |

Deux commandes intégrées sont toujours disponibles :

| Commande | Fait |
|---|---|
| `!list` | Liste tout ce que le joueur peut faire apparaître, une par ligne, avec combien il lui en reste et, lorsqu'il n'en reste plus, combien de temps avant la prochaine |
| `!help` | Liste les commandes que le joueur peut utiliser |

Un preset ou un kit ne peut pas s'appeler `list` ou `help` ; les intégrées l'emportent et le journal du serveur émet un avertissement.

### Administrateurs

Placez les ID Steam64 des administrateurs dans `Admins` :

```json
"Admins": ["76561198000000000"],
```

Les administrateurs n'ont aucune limite et peuvent utiliser les commandes marquées `AdminOnly`. Les autres joueurs ne voient pas les commandes réservées aux admins dans `!help` ou `!list`, et rien ne se passe s'ils en tapent une.

### Presets

Chaque preset a sa propre limite (une Olga et une Ada toutes les 15 minutes sont des allocations séparées). Pour ajouter un preset, copiez un bloc dans `Presets`, donnez-lui un nouveau `Command`, et redémarrez le serveur. Champs :

| Champ | Signification |
|---|---|
| `Command` | Commande de chat, sans le préfixe (`car` → `!car`) |
| `DisplayName` | Nom affiché dans le message |
| `ClassName` | Classe du véhicule ou de l'objet à faire apparaître |
| `Attachments` | Classes attachées. Répétez une classe pour chaque emplacement qu'il occupe (quatre roues) |
| `DistanceMetres`, `BearingDegrees` | Où il apparaît par rapport au joueur. 0° = nord, 90° = est |
| `DespawnSeconds` | 0 = ne jamais disparaître |
| `MaxSpawns`, `LimitMinutes` | Chaque joueur peut le faire apparaître `MaxSpawns` fois toutes les `LimitMinutes` minutes de jeu |
| `AdminOnly` | Réservé aux administrateurs uniquement |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutes avant disparition |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Véhicules uniquement : remplissage initial de chaque fluide, de 0 à 1 |
| `Invulnerable` | Le véhicule et ses pièces attachées ne prennent aucun dommage : crashes, balles, explosions |
| `ProtectCrew` | Véhicules uniquement : personne à l'intérieur n'est blessée ou tuée lors d'un crash |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espace libre (mètres) nécessaire au point d'apparition. La valeur par défaut convient pour une Olga ; augmentez-la pour les camions |

### Kits

Un kit est un ensemble d'objets pour une seule commande. Champs :

| Champ | Signification |
|---|---|
| `Command`, `DisplayName` | Comme pour les presets |
| `MaxSpawns`, `LimitMinutes` | Limite par joueur |
| `RealTimeLimit` | Comptez `LimitMinutes` en temps réel au lieu du temps de jeu (survive aux redémarrages) |
| `AdminOnly` | Réservé aux administrateurs uniquement |
| `ToInventory` | Place les objets dans l'inventaire du joueur là où il y a de la place ; le reste va au sol. Sinon, tout va au sol devant eux |
| `Items` | `ClassName`, `Count` et `Contents`. Les contenus vont dans l'objet : attachés là où ils rentrent (chargeurs, silencieux, piles, étuis), sinon dans son fret, et peuvent avoir leurs propres contenus |

Les objets des kits apparaissent neufs, les chargeurs et munitions pleins, les batteries chargées et les bouteilles remplies d'eau. Un chargeur listé dans le contenu d'une arme est chargé avec un projectile en chambre.

La configuration par défaut n'est écrite que s'il n'existe pas encore de `config.json`. Après avoir mis à jour le mod, supprimez l'ancien fichier (ou ajoutez les nouveaux champs) pour appliquer les nouvelles valeurs par défaut ; le journal du serveur émet un avertissement si le fichier provient d'une ancienne version.

Le temps de jeu et l'historique des apparitions par joueur sont sauvegardés dans `$profile:ChillSpawn/players/`.

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

Les bases fonctionnent sur DayZ 1.30 experimental. Vérifiez ces points sur un serveur de test lors du changement de configuration ou de la mise à jour du jeu :

1. **Hook de chat : fonctionne.** Le mod lit le chat via `MissionServer.OnEvent` avec `ChatMessageEventTypeID`, donc les joueurs n'ont rien à installer. Le [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) liste `ChatMessageEventTypeID` comme un événement côté client, mais sur un serveur DayZ 1.30 experimental, le serveur le reçoit aussi. L'événement ne donne que le nom de l'expéditeur, donc les joueurs sont appariés par leur nom. Si deux joueurs partagent le même nom, la commande est ignorée.
2. **Noms de classes d'attachments et de kits.** Vérifiez que chaque classe dans `Attachments` et chaque objet de kit apparaît (le journal du serveur affiche `Could not attach …`, `Could not create …` ou `Could not put …` pour ceux qui ne fonctionnent pas), et que la voiture démarre et roule.
3. **Protection des voitures (nouveau).** `Invulnerable` arrête les dommages au véhicule et aux pièces avec lesquelles il a été spawné ; les pièces installées plus tard ne sont pas protégées. `ProtectCrew` arrête les dommages de crash pour les personnes à l'intérieur. Aucun ne protège les personnes heurtées par la voiture. Les deux sont conservés en mémoire, donc après un redémarrage du serveur, une voiture qui n'a pas encore disparu redevient un véhicule ordinaire.
4. **Vérification des obstacles.** Utilise `IsBoxColliding` avec une boîte de la taille de la voiture, surélevée de 0,3 m au-dessus du sol, plus une vérification mer/étang. Il essaie 10 directions à 6, 9 et 12 m. Vérifiez qu'il ne traite pas faussement les buissons comme des obstacles, et qu'il détecte bien les murs et les clôtures. Il ne vérifie pas la pente, donc une voiture peut toujours atterrir sur une pente raide.
5. **Les compteurs de disparition vivent en mémoire.** Si le serveur redémarre avant que le timer d'une voiture ne s'écoule, la voiture reste dans le monde comme un véhicule persistant normal. Si un joueur est dans la voiture lorsque le timer expire, la disparition est réessayée chaque minute jusqu'à ce qu'il sorte.

## Construction

### Linux

```bash
tools/build-linux.sh
```

Cela packe `dist/@ChillSpawn/addons/ChillSpawn.pbo` avec le PBO builder de [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools est un logiciel gratuit sous sa propre licence et n'est pas inclus ici : le script le clone dans `~/.cache/rag-dayz-tools` au premier lancement, ou utilise le chemin que vous passez (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) ou définissez dans `RAG_TOOLS_DIR`. Nécessite `python3` 3.10+ et `git`.

Le script exécute les vérifications préliminaires de RaG et packe les scripts tels quels. Il saute la binarisation, la conversion `config.cpp` et la signature, qui nécessitent les programmes Windows DayZ Tools. Un mod serveur uniquement de scripts n'en a besoin d'aucun. Rien sur Linux ne compile Enforce Script, donc les erreurs de script n'apparaissent que dans le journal `.rpt` du serveur au démarrage.

Copiez `dist/@ChillSpawn` dans le dossier du serveur et ajoutez `-servermod=@ChillSpawn` à la ligne de démarrage.

### Windows

Packez le dossier `ChillSpawn/` dans `@ChillSpawn/addons/ChillSpawn.pbo` avec l'AddonBuilder de DayZ Tools ou le RaG PBO Builder. Les mods serveur uniquement n'ont pas strictement besoin d'être signés, mais si le serveur vérifie les signatures, signe le PBO et place le `.bikey` dans le dossier `keys/` du serveur.

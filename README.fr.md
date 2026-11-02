# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · **Français** · [Español](README.es.md) · [Português](README.pt.md)

> Traduction automatique. En cas de différence, l'[original anglais](README.md) fait foi.

Un mod pour serveur DayZ. Les joueurs tapent `!car` dans le chat pour faire apparaître une Olga 24 rouge prête à conduire, située à six mètres au nord d'eux : quatre roues, bougie d'allumage, radiateur, batterie chargée, phares, portes, capot et coffre, avec un plein de carburant, liquide de refroidissement, huile et liquide de frein. Si cet emplacement est bloqué ou dans l'eau, le mod essaie des emplacements voisins ; si aucun n'est libre, le joueur reçoit l'instruction de bouger et la tentative ne compte pas contre sa limite. Il reçoit un message privé indiquant que le véhicule disparaît au bout de 15 minutes. Chaque joueur peut faire apparaître une Olga toutes les 15 minutes de temps de jeu (temps passé connecté, pas le temps réel). Les voitures spawnées ne prennent aucun dégât, et personne à l'intérieur n'est blessée lors d'un crash.

Les joueurs peuvent aussi taper `!survive` une fois toutes les 2 heures réelles pour obtenir un petit kit de survie. Les admins n'ont aucune limite et disposent de `!adminkit`.

Les clients n'ont pas besoin du mod. Chargez-le avec `-servermod=@ChillSpawn`. Écrit pour DayZ 1.30.

> **Statut : testé sur DayZ 1.30 experimental.** Chaque commande, les admins, les kits et la protection des véhicules fonctionnent sur un serveur en direct sans aucun mod client. Voir [Limites connues / Tester d'abord](#limites-connues--tester-dabord). [docs/PROMPTING.md](docs/PROMPTING.fr.md) explique comment ce mod a été créé.

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

Au premier démarrage, le mod écrit `$profile:ChillSpawn/config.json` (dans le dossier `-profiles` du serveur). Chaque entrée dans `Presets` et `Kits` devient sa propre commande de chat, permettant aux propriétaires de serveur d'en ajouter sans toucher au code. Les valeurs par défaut :

| Commande | Génère | Limite |
|---|---|---|
| `!car` | Olga 24 rouge, prête à conduire, sans dégâts, équipage protégé | 1 toutes les 15 min de jeu |
| `!ada` | Ada 4x4 verte, identique (`OffroadHatchback_Blue` / `_White` pour d'autres couleurs, avec classes de portes, capot et coffre correspondantes `_Blue` / `_White`) | 1 toutes les 15 min de jeu |
| `!hatchet` | Une hache à 1 mètre du joueur au sol | 3 toutes les 60 min de jeu |
| `!survive` | Dans l'inventaire du joueur : une conserve de bacon, une bouteille d'eau pleine, comprimés de purification, un bandage et de la tétracycline | 1 toutes les 2 heures réelles |
| `!adminkit` | Réservé aux admins. Au sol devant eux : sac à dos Alice, M14 avec 3 chargeurs complets de 20 coups et 3 boîtes de .308, M4A1 avec 3 chargeurs complets de 60 coups et 2 silencieux, gilet pare-balles complet, casque MICH avec vision nocturne et piles, veste camouflage TTsKO, pantalon et bottes, gants et couteau de chasse | aucune |

Deux commandes intégrées sont toujours disponibles :

| Commande | Fait |
|---|---|
| `!list` | Liste tout ce que le joueur peut faire apparaître, une par ligne, avec le nombre restant et, lorsqu'il n'en reste plus, le temps avant la prochaine disponibilité |
| `!help` | Liste les commandes utilisables par le joueur |

Un préréglage ou un kit ne peut pas s'appeler `list` ou `help` ; les intégrées l'emportent et le journal du serveur émet un avertissement.

### Admins

Placez les ID Steam64 des admins dans `Admins` :

```json
"Admins": ["76561198000000000"],
```

Les admins n'ont aucune limite et peuvent utiliser les commandes marquées `AdminOnly`. Les autres joueurs ne voient pas les commandes réservées aux admins dans `!help` ou `!list`, et rien ne se passe s'ils en tapent une.

### Presets (Préréglages)

Chaque préréglage a sa propre limite (une Olga et une Ada toutes les 15 minutes sont des allowances séparées). Pour ajouter un préréglage, copiez un bloc dans `Presets`, donnez-lui un nouveau `Command` et redémarrez le serveur. Champs :

| Champ | Signification |
|---|---|
| `Command` | Commande de chat, sans le préfixe (`car` → `!car`) |
| `DisplayName` | Nom affiché dans le message |
| `ClassName` | Classe du véhicule ou de l'objet à générer |
| `Attachments` | Classes attachées au véhicule. Répétez une classe pour chaque emplacement qu'il occupe (quatre roues) |
| `DistanceMetres`, `BearingDegrees` | Position par rapport au joueur lors de la génération. 0° = nord, 90° = est |
| `DespawnSeconds` | 0 = ne jamais disparaître |
| `MaxSpawns`, `LimitMinutes` | Chaque joueur peut le générer `MaxSpawns` fois toutes les `LimitMinutes` minutes de temps de jeu |
| `AdminOnly` | Réservé aux seuls admins |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutes avant disparition |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Véhicules uniquement : niveau de remplissage initial de chaque fluide, de 0 à 1 |
| `Invulnerable` | Le véhicule et ses pièces attachées ne prennent aucun dégât : chocs, balles, explosions |
| `ProtectCrew` | Véhicules uniquement : personne à l'intérieur n'est blessée ou tuée lors d'un crash |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espace libre (en mètres) nécessaire au point de spawn. La valeur par défaut convient pour une Olga ; augmentez-la pour les camions |

### Kits

Un kit est un ensemble d'objets pour une seule commande. Champs :

| Champ | Signification |
|---|---|
| `Command`, `DisplayName` | Comme pour les préréglages |
| `MaxSpawns`, `LimitMinutes` | Limite par joueur |
| `RealTimeLimit` | Comptez `LimitMinutes` en temps réel au lieu du temps de jeu (persiste après redémarrage) |
| `AdminOnly` | Réservé aux seuls admins |
| `ToInventory` | Place les objets dans l'inventaire du joueur là où il y a de la place ; le reste va au sol. Sinon, tout va au sol devant eux |
| `Items` | `ClassName`, `Count` et `Contents`. Le contenu va dans l'objet : attaché là où ça rentre (chargeurs, silencieux, piles, étuis), sinon dans son coffre, et peut avoir son propre contenu |

Les objets des kits apparaissent neufs, chargeurs et munitions pleins, batteries chargées et bouteilles remplies d'eau. Une chargeur listé dans le contenu d'une arme est chargé avec un projectile en chambre.

La configuration par défaut n'est écrite que s'il n'existe pas encore de `config.json`. Après avoir mis à jour le mod, supprimez l'ancien fichier (ou ajoutez les nouveaux champs) pour activer les nouvelles valeurs par défaut ; le journal du serveur avertit lorsque le fichier provient d'une version plus ancienne.

Le temps de jeu par joueur et l'historique des spawns sont sauvegardés dans `$profile:ChillSpawn/players/`.

## Ajouter des commandes (extension)

Héritez de `CS_ChatCommand` et enregistrez-la depuis un autre mod, ou dans celui-ci :

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

## Limites connues / Tester d'abord

Les bases fonctionnent sur DayZ 1.30 experimental. Vérifiez ces points sur un serveur de test lors de modifications de la configuration ou de mises à jour du jeu :

1. **Hook de chat : fonctionne.** Le mod lit le chat via `MissionServer.OnEvent` avec `ChatMessageEventTypeID`, donc les joueurs n'ont rien à installer. Le [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) liste `ChatMessageEventTypeID` comme un événement côté client, mais sur un serveur DayZ 1.30 experimental, le serveur le reçoit aussi. L'événement ne donne que le nom de l'expéditeur, donc les joueurs sont appariés par nom. Si deux joueurs partagent le même nom, la commande est ignorée.
2. **Noms de classes d'accessoires et de kits.** Vérifiez que chaque classe dans `Attachments` et chaque objet de kit apparaît (le journal du serveur affiche `Could not attach …`, `Could not create …` ou `Could not put …` pour ceux qui échouent), et que la voiture démarre et roule.
3. **Protection des véhicules.** `Invulnerable` arrête les dégâts sur la voiture et les pièces avec lesquelles elle a été spawnée ; les pièces installées plus tard ne sont pas protégées. `ProtectCrew` arrête les dégâts de crash pour les personnes à l'intérieur. Aucun des deux ne protège les personnes heurtées par la voiture. Les deux sont conservés en mémoire, donc après un redémarrage du serveur, une voiture qui n'a pas encore disparu redevient une voiture ordinaire.
4. **Vérification des obstacles.** Utilise `IsBoxColliding` avec une boîte de la taille de la voiture, soulevée de 0,3 m du sol, plus une vérification mer/étang. Il essaie 10 directions à 6, 9 et 12 mètres. Vérifiez qu'il ne traite pas faussement les buissons comme des obstacles, et qu'il détecte bien murs et clôtures. Il ne vérifie pas la pente, donc une voiture peut encore atterrir sur une colline abrupte.
5. **Les minuteurs de disparition vivent en mémoire.** Si le serveur redémarre avant la fin du timer d'une voiture, celle-ci reste dans le monde en tant que véhicule persistant normal. Si un joueur est dans la voiture lorsque le timer expire, la disparition est réessayée chaque minute jusqu'à ce qu'il sorte.

## Construction

### Linux

```bash
tools/build-linux.sh
```

Cela packe `dist/@ChillSpawn/addons/ChillSpawn.pbo` avec le constructeur de PBO de [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools est un logiciel gratuit sous sa propre licence et n'est pas inclus ici : le script le clone dans `~/.cache/rag-dayz-tools` au premier lancement, ou utilise le chemin que vous passez (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) ou définissez dans `RAG_TOOLS_DIR`. Nécessite `python3` 3.10+ et `git`.

Le script exécute les vérifications préalables de RaG et packe les scripts tels quels. Il saute la binarisation, la conversion `config.cpp` et la signature, qui nécessitent les programmes DayZ Tools sous Windows. Un mod serveur uniquement en scripts n'a besoin d'aucun d'eux. Rien sous Linux ne compile Enforce Script, donc les erreurs de script n'apparaissent que dans le journal `.rpt` du serveur au démarrage.

### Windows

Packez le dossier `ChillSpawn/` dans `@ChillSpawn/addons/ChillSpawn.pbo` avec l'AddonBuilder de DayZ Tools ou le RaG PBO Builder. Les mods serveur uniquement n'ont pas strictement besoin d'être signés, mais si le serveur vérifie les signatures, signe le PBO et place le `.bikey` dans le dossier `keys/` du serveur.

## Installation

1. Copiez le dossier `@ChillSpawn` dans le dossier racine du serveur, à côté de `DayZServer_x64.exe`.
2. Chargez-le en tant que mod serveur :
   - **Serveur standard :** ajoutez `-servermod=@ChillSpawn` à la ligne de démarrage.
   - **CFTools Architect :** utilisez **Add Local Mod**, choisissez `@ChillSpawn` et définissez son type sur **Server**.
3. Démarrez le serveur. Il écrit `profiles/ChillSpawn/config.json` et journalise `[ChillSpawn] Loaded … spawn presets, … kits and … admins.`
4. Ajoutez les ID Steam64 de vos admins dans `Admins` de ce fichier et redémarrez.

**Mise à jour :** arrêtez complètement le serveur avant de remplacer `ChillSpawn.pbo`. Un serveur en cours d'exécution verrouille le fichier, et un redémarrage rapide peut ne pas laisser le temps de le remplacer.

# Comment ChillSpawn a été créé avec Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · **Français** · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Traduction automatique. En cas de différence, l'[original anglais](PROMPTING.md) fait foi.

ChillSpawn a été écrit lors d'une seule session Claude Code (Claude Opus 5.5) sur une machine Linux sans installation de DayZ. Claude a rédigé chaque fichier. La personne qui le dirigeait a rédigé les exigences, demandé des modifications et comparé le résultat avec deux modèles locaux. La première version a été écrite sans compiler ni exécuter quoi que ce soit. Plus tard dans la même session, un serveur dédié Windows exécutant DayZ 1.30 experimental a été configuré, Claude y a accédé via SSH, et le mod a été déployé, testé en jeu et étendu à partir de là.

Cette page parcourt les demandes dans l'ordre (reformulées et nettoyées, pas citées mot pour mot), ce que chacune a changé et ce qui a été appris. Elle se termine par des invites que vous pouvez réutiliser pour créer un mod similaire.

## Les invites, dans l'ordre

### 1. La première demande

> Écrire un mod DayZ côté serveur qui permet à un joueur de faire apparaître une voiture avec une commande de chat :
>
> - La voiture est l'Olga rouge, avec quatre roues, une bougie d'allumage et un radiateur fixés.
> - Elle apparaît à 6 mètres au nord du joueur.
> - Le joueur reçoit un message privé indiquant que la voiture disparaîtra dans les 15 minutes.
> - Chaque joueur peut faire apparaître 3 voitures par heure de temps passé en jeu.
> - Inclure un cadre pour que le mod puisse être étendu plus tard.
> - Une configuration JSON permet au propriétaire du serveur de choisir ce qui doit apparaître : une voiture avec ses accessoires, ou un objet.

Claude a produit l'intégralité du mod : `config.cpp`, la répartition 3_Game/4_World/5_Mission, une configuration JSON écrite dans `$profile:` au premier démarrage, un registre de commandes (le « cadre »), le suivi du temps de jeu enregistré par joueur, et un README. Il a également signalé ce dont il n'était pas sûr plutôt que de le cacher :

- que le serveur reçoit le chat via `MissionServer.OnEvent` avec `ChatMessageEventTypeID`, et ne reçoit que le nom de l'expéditeur ;
- qu'une Olga avec seulement des roues, une bougie d'allumage et un radiateur **ne roule pas** (pas de batterie ni de fluides) ;
- que les minuteurs de disparition sont perdus lors du redémarrage ;
- qu'il n'y avait aucun contrôle d'obstacle.

**Leçon :** une liste numérotée d'exigences concrètes (véhicule exact, distance, direction, limites) obtient une réponse complète en une seule fois. Demandez à Claude de lister ses hypothèses et tout ce qu'il n'a pas pu vérifier.

### 2. Rendre la voiture conduisable et vérifier les obstacles

> Ajoutez tout le reste dont la voiture a besoin pour apparaître prête à conduire. Vérifiez également les obstacles avant de la faire apparaître.

Ajout de la batterie (chargée), des phares, des portes, du capot et du coffre, et remplissage de l'essence, du liquide de refroidissement, de l'huile et du liquide de frein (tous configurables). Le contrôle d'obstacle utilise `IsBoxColliding` avec une boîte de taille voiture plus une vérification mer/étang. Il essaie 10 directions à trois distances. Si rien n'est clair, le joueur est informé et la tentative ne compte pas contre sa limite.

**Leçon :** les suites qui répondent à la propre liste « limites connues » de Claude sont peu coûteuses et ciblées.

### 3. Plus d'une chose spawnable

> Faites en sorte que la configuration d'exemple montre plusieurs choses spawnables, par exemple une hachette et l'Ada 4x4.

La configuration le supportait déjà (chaque préréglage est sa propre commande de chat). Claude a ajouté les préréglages `!ada` et `!hatchet` aux valeurs par défaut et à `config.example.json`, et a documenté comment en ajouter d'autres.

### 4. Comparaison avec des modèles locaux

> Faites écrire le même mod par mon modèle Qwen local, puis comparez sa version avec la vôtre : en quoi sont-elles différentes, et l'une ou l'autre contient-elle des erreurs ?

Claude a envoyé les mêmes exigences combinées à un modèle Qwen local, puis a revu une seconde tentative de Qwen fournie par l'utilisateur. Aucune n'aurait fonctionné :

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| API DayZ réelle | Majoritairement inventée (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Majoritairement réelle |
| Se chargerait | Non : pas de modules de script, ne hook jamais `MissionServer` | Non : `config.cpp` n'a pas de `CfgMods` |
| Compilerait | Non | Non : opérateur ternaire, itérateurs style C++, `ToLower()` utilisé comme s'il retournait une chaîne |
| « Nord » | 6 m dans les airs | 6 m au sud |
| Limite de débit | En mémoire, blocs fixes | Uptime du serveur, perdu au redémarrage |

**Leçon :** pour Enforce Script, les modèles à usage général écrivent confiamment des idiomes C++/Java que DayZ ne supporte pas. Obtenez un deuxième avis en demandant à un modèle de réviser le code d'un autre par rapport à une liste de contrôle (chargement, compilation, comportement), et non en faisant confiance à l'un ou l'autre.

### 5. Emprunter une bonne idée

> Apportez l'idée de la seconde version de Qwen concernant une liste de véhicules/objets dans ChillSpawn.

Qwen #2 avait une sous-commande `/vspawn list`. Claude a ajouté `!list` (chaque préréglage avec combien d'apparitions il reste au joueur cette heure) et `!help` en tant que commandes intégrées dans le cadre de ChillSpawn.

### 6. Publication

> Commitez ceci dans mon repo GitHub public (licence MIT ; remplacez le README si vous voulez). Assurez-vous qu'aucun identifiant ne soit inclus. Ajoutez une doc montrant comment l'incitation a été faite et quelles invites quelqu'un d'autre pourrait utiliser pour créer un mod similaire.

Cela a produit ce dépôt et cette page.

### 7. Vérification par rapport à la mise à jour 1.30

> Vérifiez les notes de patch 1.30 et le tracker de feedback de Bohemia pour tout ce qui pourrait casser notre logique.

Rien dans les notes ne touchait aux API utilisées par le mod. La 1.30 a apporté un compilateur de script plus strict, donc deux constructions qu'il aurait pu rejeter ont été modifiées (une méthode déclarée sans corps, et une conversion float-to-int).

### 8. Construction sous Linux

> Voir si RaG DayZ Tools peut construire ceci sur Linux.

RaG DayZ Tools est Windows-first, mais son constructeur PBO est du Python pur et s'exécute sur Linux. `tools/build-linux.sh` l'utilise pour packager le mod. La binarisation, la conversion de configuration et la signature nécessitent toujours des programmes Windows, et un mod serveur uniquement scripts n'en a pas besoin.

### 9. Test sur un vrai serveur

> J'ai configuré un serveur dédié Windows avec CFTools Architect et ouvert SSH pour vous. Déployez le mod là-bas et vérifiez qu'il fonctionne.

Le serveur exécutait DayZ 1.30 experimental, géré par CFTools Architect. Via SSH, Claude :

- a copié les `scripts.pbo` du jeu lui-même depuis le serveur, les a dépaquetés et a vérifié chaque fonction appelée par le mod contre les scripts réels de la 1.30 experimental. Tous existaient avec des paramètres correspondants ;
- a construit le PBO sous Linux, copié `@ChillSpawn` dans le dossier racine du serveur, et a demandé au propriétaire de l'ajouter dans Architect avec **Add Local Mod**, type **Server** ;
- a lu les logs `.rpt` et de script du serveur après chaque redémarrage pour confirmer que les scripts compilaient et que le mod chargeait.

`!help` et `!car` ont fonctionné dès la première fois. Le hook de chat, la partie la plus susceptible d'échouer, fonctionne sur un serveur sans mod client.

**Leçons :**

- **Donnez à Claude les vrais scripts du jeu.** Le dépaquetage des `scripts.pbo` du jeu installé transforme « cette API existe probablement » en une vérification rapide, et cela règle les questions que les wikis traitent mal.
- **Remplacez le PBO avec le serveur entièrement arrêté.** Un serveur en cours d'exécution verrouille le fichier du mod, et le redémarrage d'Architect est trop rapide pour le capturer.

### 10. Admins, limites, voitures protégées et kits

> Permettez aux admins de faire apparaître autant qu'ils veulent. Limitez les joueurs à un de chaque voiture toutes les 15 minutes. Les voitures spawnables peuvent-elles être rendues invulnérables, ou les accidents cesser de blesser les personnes à l'intérieur, sans un mod client ? Ajoutez un `!adminkit` réservé aux admins avec [une liste d'armes, d'armure et de vêtements], et un kit `!survive` que les joueurs peuvent utiliser une fois toutes les 2 heures réelles avec [nourriture, eau, objets médicaux].

Claude a d'abord lu le code des voitures 1.30. Les dégâts aux occupants lors d'un crash sont appliqués par un script côté serveur (`CarScript.DamageCrew`), donc un mod serveur peut le désactiver. Les dégâts à la voiture elle-même sont désactivés avec `SetAllowDamage(false)`. Ensuite, il a ajouté :

- une liste `Admins` d'IDs Steam64, sans limites et avec accès aux commandes réservées aux admins, que les autres joueurs ne peuvent pas voir ;
- des limites sous forme de `MaxSpawns` par `LimitMinutes`, comptées en temps de jeu, ou en temps réel pour les kits qui le demandent ;
- des options `Invulnerable` et `ProtectCrew` pour les préréglages ;
- des kits : objets imbriqués (un fusil avec son chargeur et son silencieux, un sac à dos avec son contenu), chargeurs pleins et eau, livrés dans l'inventaire ou sur le sol.

Tout a fonctionné en jeu lors du premier déploiement.

**Leçon :** demander « cela peut-il être fait côté serveur ? » avant de demander la fonctionnalité obtient une réponse ancrée dans le code réel du jeu, ainsi que ses limites : ici, que les personnes heurtées par la voiture sont toujours blessées, et que la protection est perdue au redémarrage.

## Invites réutilisables

### Démarrer un nouveau mod côté serveur

Copiez ceci, remplissez les crochets et collez-le dans Claude (Claude Code fonctionne mieux, car il écrit les fichiers pour vous) :

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

La ligne « Enforce Script has no… » provient directement des erreurs commises par les modèles locaux. La laisser coûter rien et empêche les erreurs les plus courantes.

### Suites qui ont bien fonctionné

- "Rendez la voiture spawnable entièrement conduisable : chaque pièce dont elle a besoin, batterie chargée et tous les fluides pleins."
- "Ajoutez un contrôle d'obstacle avant l'apparition. Si l'emplacement est bloqué ou dans l'eau, essayez des emplacements proches ; si aucun n'est clair, informez le joueur et ne le comptez pas contre sa limite."
- "Ajoutez une commande `!list` montrant chaque préréglage spawnable et combien il en reste au joueur."
- "Révisez ce code [collez ou attachez] pour les éléments qui ne compileront pas ou ne chargeront pas dans DayZ, et pour les erreurs de logique par rapport à ces exigences : [liste]."
- "Écrivez un plan de test que je peux suivre sur un serveur local pour vérifier chaque exigence."
- "Voici l'accès SSH à mon serveur. Vérifiez chaque API utilisée par le mod contre les scripts du jeu avant de déployer."
- "[Fonctionnalité] peut-elle être faite uniquement côté serveur ? Vérifiez les scripts du jeu et dites-moi ce qu'elle ne peut pas couvrir."

### Conseils

- **Soyez précis sur les objets du jeu.** « Red Olga » est devenu `CivilianSedan_Wine`. Si vous connaissez les noms de classe, donnez-les ; sinon, demandez à Claude de lister ceux qu'il a utilisés pour que vous puissiez les vérifier dans le `types.xml` du serveur ou avec un outil admin.
- **Précisez ce que signifie « par heure ».** « Par heure de temps de jeu » et « par heure de temps serveur » sont des fonctionnalités différentes, et seule l'une d'elles survit à une déconnexion/reconnexion.
- **Demandez une section « limites connues »** et traitez-la comme votre liste de test.
- **Testez sur un serveur local avant la mise en ligne.** Surveillez le log `.rpt` du serveur pour les erreurs de script et les lignes comme `[ChillSpawn] Could not attach …`.
- **Donnez à Claude l'accès SSH à un serveur de test** si vous le pouvez. Il peut alors déployer, lire les logs et vérifier les scripts du jeu lui-même, ce qui transforme les suppositions en vérifications.
- **Sur Windows avec DayZ Tools installé**, le plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) pour Claude Code ajoute des agents et compétences spécifiques à DayZ qui construisent des PBO et lancent un serveur de test local, comblant ainsi l'écart « non testé ».

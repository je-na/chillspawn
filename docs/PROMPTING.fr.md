# Comment ChillSpawn a été créé avec Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · **Français** · [Español](PROMPTING.es.md) · [Português](PROMPTING.pt.md)

> Traduction automatique. En cas de différence, l'[original anglais](PROMPTING.md) fait foi.

ChillSpawn a été écrit lors d'une seule session de Claude Code (Claude Opus 5.5) sur une machine Linux sans installation de DayZ. Claude a rédigé chaque fichier. La personne qui le guidait a rédigé les exigences, demandé des modifications et comparé le résultat avec deux modèles locaux. Aucun code n'a été compilé ou exécuté dans le jeu pendant la session, considérez donc le code comme une première version solide qui nécessite encore un serveur de test.

Cette page présente les demandes dans l'ordre (reformulées et épurées, pas citées mot pour mot), ce que chacune a changé et ce qui a été appris. Elle se termine par des invites de commande (prompts) que vous pouvez réutiliser pour créer un mod similaire.

## Les invites de commande, dans l'ordre

### 1. La première demande

> Écrivez un mod côté serveur DayZ qui permet à un joueur de faire apparaître une voiture avec une commande de chat :
>
> - La voiture est l'Olga rouge, équipée de quatre roues, d'une bougie d'allumage et d'un radiateur.
> - Elle apparaît à 6 mètres au nord du joueur.
> - Le joueur reçoit un message privé indiquant que la voiture disparaîtra dans les 15 minutes.
> - Chaque joueur peut faire apparaître 3 voitures par heure de temps passé en jeu.
> - Incluez une structure de base afin que le mod puisse être étendu plus tard.
> - Une configuration JSON permet au propriétaire du serveur de choisir ce qui doit apparaître : une voiture avec ses accessoires, ou un objet.

Claude a produit le mod entier : `config.cpp`, la répartition des scripts en 3_Game/4_World/5_Mission, une configuration JSON écrite dans `$profile:` au premier démarrage, un registre de commandes (la « structure de base »), le suivi du temps de jeu sauvegardé par joueur, et un README. Il a également signalé ce dont il n'était pas sûr plutôt que de le cacher :

- que le serveur reçoit les messages de chat via `MissionServer.OnEvent` avec `ChatMessageEventTypeID`, et ne reçoit que le nom de l'expéditeur ;
- qu'une Olga équipée uniquement de roues, d'une bougie d'allumage et d'un radiateur **ne roulera pas** (pas de batterie ni de fluides) ;
- que les minuteurs de disparition sont perdus lors d'un redémarrage ;
- qu'il n'y avait aucun contrôle des obstacles.

**Leçon :** une liste numérotée d'exigences concrètes (véhicule exact, distance, direction, limites) obtient une réponse complète en une seule fois. Demandez à Claude de lister ses hypothèses et tout ce qu'il n'a pas pu vérifier.

### 2. Rendre la voiture conduisible et vérifier les obstacles

> Ajoutez tout le reste dont la voiture a besoin pour apparaître prête à conduire. Vérifiez également les obstacles avant de faire apparaître la voiture.

Ajout de la batterie (chargée), des phares, des portes, du capot et du coffre, et remplissage de l'essence, du liquide de refroidissement, de l'huile et du liquide de frein (tous configurables). Le contrôle des obstacles utilise `IsBoxColliding` avec une boîte aux dimensions d'une voiture plus une vérification mer/étang. Il essaie 10 directions à trois distances. Si rien n'est clair, le joueur en est informé et la tentative ne compte pas contre sa limite.

**Leçon :** les demandes suivantes qui répondent à la propre liste des « limites connues » de Claude sont peu coûteuses et ciblées.

### 3. Plus d'un élément pouvant apparaître

> Faites en sorte que la configuration d'exemple montre plusieurs éléments pouvant apparaître, par exemple une hachette et l'Ada 4x4.

La configuration le supportait déjà (chaque préréglage est sa propre commande de chat). Claude a ajouté les préréglages `!ada` et `!hatchet` aux valeurs par défaut et à `config.example.json`, et a documenté comment en ajouter d'autres.

### 4. Comparaison avec des modèles locaux

> Faites écrire le même mod par mon modèle local Qwen, puis comparez sa version avec la vôtre : en quoi diffèrent-elles, et l'une ou l'autre contient-elle des erreurs ?

Claude a envoyé les mêmes exigences combinées à un modèle Qwen local, puis a examiné une seconde tentative de Qwen fournie par l'utilisateur. Aucune n'aurait fonctionné :

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| API DayZ réelle | Majoritairement inventée (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Majoritairement réelle |
| Se chargerait-elle ? | Non : pas de modules de script, ne s'accroche jamais à `MissionServer` | Non : `config.cpp` n'a pas de `CfgMods` |
| Compilerait-il ? | Non | Non : opérateur ternaire, itérateurs de style C++, `ToLower()` utilisé comme s'il retournait une chaîne |
| « Nord » | 6 m dans les airs | 6 m au sud |
| Limite de débit | En mémoire, blocs fixes | Uptime du serveur, perdu au redémarrage |

**Leçon :** pour Enforce Script, les modèles à usage général écrivent avec confiance des idiomes C++/Java que DayZ ne prend pas en charge. Obtenez un deuxième avis en demandant à un modèle de réviser le code d'un autre par rapport à une liste de contrôle (chargement, compilation, comportement), plutôt qu'en faisant confiance à l'un ou l'autre.

### 5. Emprunter une bonne idée

> Intégrez l'idée de la liste de véhicules/objets de la deuxième version de Qwen dans ChillSpawn.

Qwen #2 avait une sous-commande `/vspawn list`. Claude a ajouté `!list` (chaque préréglage avec le nombre d'apparitions restantes du joueur cette heure) et `!help` en tant que commandes intégrées à la structure de base de ChillSpawn.

### 6. Publication

> Committez ceci dans mon dépôt GitHub public (licence MIT ; remplacez le README si vous le souhaitez). Assurez-vous qu'aucun identifiant ne soit inclus. Ajoutez une documentation montrant comment les invites ont été formulées et quelles invites quelqu'un d'autre pourrait utiliser pour créer un mod similaire.

Cela a produit ce dépôt et cette page.

### 7. Vérification par rapport à la mise à jour 1.30

> Vérifiez les notes de patch 1.30 et le tracker de feedback de Bohemia pour tout ce qui pourrait casser notre logique.

Rien dans les notes ne touchait aux APIs utilisées par le mod. La version 1.30 a apporté un compilateur de scripts plus strict, donc deux constructions qu'il aurait pu rejeter ont été modifiées (une méthode déclarée sans corps, et une conversion float-to-int).

### 8. Construction sous Linux

> Vérifiez si RaG DayZ Tools peut construire ceci sur Linux.

RaG DayZ Tools est principalement orienté Windows, mais son constructeur de PBO est du Python pur et s'exécute sous Linux. `tools/build-linux.sh` l'utilise pour packager le mod. La binarisation, la conversion des configurations et la signature nécessitent toujours des programmes Windows, et un mod serveur uniquement script n'en a pas besoin.

## Invites de commande réutilisables

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

La ligne « Enforce Script has no… » provient directement des erreurs commises par les modèles locaux. La laisser ici ne coûte rien et empêche les erreurs les plus courantes.

### Suivi qui ont bien fonctionné

- "Rendez le véhicule entièrement conduisible à l'apparition : chaque pièce dont il a besoin, batterie chargée et tous les fluides pleins."
- "Ajoutez un contrôle des obstacles avant l'apparition. Si l'endroit est bloqué ou dans l'eau, essayez des endroits proches ; si aucun n'est clair, dites-le au joueur et ne le comptez pas contre sa limite."
- "Ajoutez une commande `!list` affichant chaque préréglage pouvant apparaître et combien il en reste au joueur."
- "Examinez ce code [collez ou joignez] pour les éléments qui ne compileront ou ne se chargeront pas dans DayZ, et pour les erreurs de logique par rapport à ces exigences : [liste]."
- "Écrivez un plan de test que je peux suivre sur un serveur local pour vérifier chaque exigence."

### Conseils

- **Soyez précis sur les objets du jeu.** « Olga rouge » est devenu `CivilianSedan_Wine`. Si vous connaissez les noms de classe, donnez-les ; sinon, demandez à Claude de lister ceux qu'il a utilisés afin que vous puissiez les vérifier dans le `types.xml` du serveur ou avec un outil d'administration.
- **Précisez ce que signifie « par heure ».** « Par heure de temps de jeu » et « par heure de temps serveur » sont des fonctionnalités différentes, et une seule survit à une déconnexion/reconnexion.
- **Demandez une section « limites connues »** et traitez-la comme votre liste de test.
- **Testez sur un serveur local avant la mise en production.** Surveillez le journal `.rpt` du serveur pour les erreurs de script et les lignes telles que `[ChillSpawn] Could not attach …`.
- **Sur Windows avec DayZ Tools installé**, le plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) pour Claude Code ajoute des agents et des compétences spécifiques à DayZ qui construisent des PBO et lancent un serveur de test local, ce qui comble l'écart « non testé ».

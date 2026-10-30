# Cómo se creó ChillSpawn con Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · **Español** · [Português](PROMPTING.pt.md)

> Traducción automática. En caso de diferencias, prevalece el [original en inglés](PROMPTING.md).

ChillSpawn fue escrito en una única sesión de Claude Code (Claude Opus 5.5) en una máquina Linux sin ninguna instalación de DayZ. Claude escribió cada archivo. La persona que lo dirigía redactó los requisitos, solicitó cambios y comparó el resultado con dos modelos locales. No se compiló ni ejecutó nada en el juego durante la sesión, por lo que debe tratarse al código como un primer borrador sólido que aún necesita pruebas en un servidor de prueba.

Esta página recorre las solicitudes en orden (reformuladas y pulidas, no citadas palabra por palabra), qué cambió cada una y qué se aprendió. Termina con los *prompts* que puedes reutilizar para crear un mod similar.

## Los prompts, en orden

### 1. La primera solicitud

> Escribe un mod de servidor para DayZ que permita a un jugador generar un coche con un comando de chat:
>
> - El coche es el Olga rojo, con cuatro ruedas, una bujía y un radiador instalados.
> - Aparece 6 metros al norte del jugador.
> - El jugador recibe un mensaje privado indicando que el coche desaparecerá en 15 minutos.
> - Cada jugador puede generar 3 coches por cada hora de tiempo pasado en el juego.
> - Incluye un marco de trabajo para que el mod pueda ampliarse más adelante.
> - Una configuración JSON permite al administrador del servidor elegir qué generar: un coche con sus accesorios o un objeto.

Claude produjo el mod completo: `config.cpp`, la división de scripts en 3_Game/4_World/5_Mission, una configuración JSON escrita en `$profile:` al primer inicio, un registro de comandos (el "marco"), el seguimiento del tiempo de juego guardado por jugador y un README. También señaló lo que no estaba seguro en lugar de ocultarlo:

- que el servidor recibe el chat a través de `MissionServer.OnEvent` con `ChatMessageEventTypeID`, y solo obtiene el nombre del remitente;
- que un Olga con solo ruedas, bujía y radiador **no conducirá** (sin batería ni fluidos);
- que los temporizadores de desaparición se pierden al reiniciar;
- que no había una comprobación de obstáculos.

**Lección:** una lista numerada de requisitos concretos (vehículo exacto, distancia, dirección, límites) obtiene una respuesta completa de golpe. Pide a Claude que liste sus suposiciones y cualquier cosa que no pudo verificar.

### 2. Hacerlo conducible y comprobar obstáculos

> Añade todo lo demás que necesite el coche para que aparezca listo para conducir. También comprueba los obstáculos antes de generarlo.

Añadió batería (cargada), luces delanteras, puertas, capó y maletero, y llenó combustible, refrigerante, aceite y líquido de frenos (todos configurables). La comprobación de obstáculos utiliza `IsBoxColliding` con una caja del tamaño de un coche más una comprobación de mar/estanque. Intenta 10 direcciones a tres distancias diferentes. Si no hay nada despejado, se informa al jugador y el intento no cuenta contra su límite.

**Lección:** las solicitudes posteriores que responden a la propia lista de "límites conocidos" de Claude son baratas y dirigidas.

### 3. Más de una cosa generable

> Haz que el ejemplo de configuración muestre varias cosas generables, por ejemplo, un hacha y el Ada 4x4.

La configuración ya lo soportaba (cada preset es su propio comando de chat). Claude añadió los presets `!ada` y `!hatchet` tanto a los valores predeterminados como a `config.example.json`, y documentó cómo añadir más.

### 4. Comparación con modelos locales

> Haz que mi modelo local Qwen escriba el mismo mod, luego compara su versión con la tuya: ¿cuánto difieren y tiene alguno errores?

Claude envió los mismos requisitos combinados a un modelo Qwen local, y más tarde revisó un segundo intento de Qwen proporcionado por el usuario. Ninguno habría funcionado:

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| API real de DayZ | Mayoritariamente inventada (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Mayoritariamente real |
| Cargaría | No: no hay módulos de script, nunca engancha `MissionServer` | No: `config.cpp` no tiene `CfgMods` |
| Compilaría | No | No: operador ternario, iteradores estilo C++, `ToLower()` usado como si devolviera una cadena |
| "Norte" | 6 m en el aire | 6 m al sur |
| Límite de frecuencia | En memoria, bloques fijos | Uptime del servidor, se pierde al reiniciar |

**Lección:** para Enforce Script, los modelos de propósito general escriben confidentemente modismos de C++/Java que DayZ no soporta. Obtén una segunda opinión haciendo que un modelo revise el código de otro contra una lista de comprobación (carga, compilación, comportamiento), en lugar de confiar en cualquiera de ellos.

### 5. Tomar prestada una buena idea

> Trae la idea del segundo Qwen sobre una lista de vehículos/objetos a ChillSpawn.

Qwen #2 tenía un subcomando `/vspawn list`. Claude añadió `!list` (cada preset con cuántas generaciones le quedan al jugador esta hora) y `!help` como comandos integrados en el marco de trabajo de ChillSpawn.

### 6. Publicación

> Confirma esto en mi repositorio público de GitHub (licencia MIT; reemplaza el README si quieres). Asegúrate de que no haya credenciales. Añade un documento que muestre cómo se hizo el prompting y qué prompts podría usar alguien más para crear un mod similar.

Eso produjo este repositorio y esta página.

### 7. Comprobación contra la actualización 1.30

> Comprueba las notas del parche 1.30 y el rastreador de comentarios de Bohemia por cualquier cosa que pueda romper nuestra lógica.

Nada en las notas tocaba las APIs que usa el mod. La versión 1.30 trajo un compilador de scripts más estricto, por lo que se cambiaron dos construcciones que podría rechazar (un método declarado sin cuerpo y una conversión de float a int).

### 8. Construcción en Linux

> Veamos si RaG DayZ Tools puede construir esto en Linux.

RaG DayZ Tools es principalmente para Windows, pero su constructor de PBO es Python puro y se ejecuta en Linux. `tools/build-linux.sh` lo utiliza para empaquetar el mod. La binarización, la conversión de configuración y la firma aún necesitan programas de Windows, y un mod de servidor solo de scripts no los necesita.

## Prompts que puedes reutilizar

### Iniciar un nuevo mod de servidor

Copia esto, rellena los corchetes y pégalo en Claude (Claude Code funciona mejor, ya que escribe los archivos por ti):

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

La línea "Enforce Script no tiene…" proviene directamente de los errores que cometieron los modelos locales. Dejarla ahí no cuesta nada y evita los errores más comunes.

### Seguimientos que funcionaron bien

- "Haz que el vehículo aparezca completamente conducible: cada parte que necesita, batería cargada y todos los fluidos llenos."
- "Añade una comprobación de obstáculos antes de generarlo. Si el lugar está bloqueado o en agua, intenta lugares cercanos; si ninguno está despejado, avisa al jugador y no lo cuentes contra su límite."
- "Añade un comando `!list` que muestre cada preset generable y cuántos le quedan al jugador."
- "Revisa este código [pega o adjunta] por cosas que no compilarán o cargarán en DayZ, y por errores de lógica contra estos requisitos: [lista]."
- "Escribe un plan de pruebas que pueda seguir en un servidor local para comprobar cada requisito."

### Consejos

- **Sé exacto con los objetos del juego.** "Olga rojo" se convirtió en `CivilianSedan_Wine`. Si conoces los nombres de las clases, dáselos; si no, pide a Claude que liste las que usó para que puedas comprobarlas en el `types.xml` del servidor o con una herramienta de administración.
- **Di qué significa "por hora".** "Por hora de tiempo de juego" y "por hora de tiempo de servidor" son características diferentes, y solo una de ellas sobrevive a una reconexión.
- **Pide una sección de "límites conocidos"** y trátala como tu lista de pruebas.
- **Prueba en un servidor local antes de ir en vivo.** Observa el registro `.rpt` del servidor buscando errores de script y líneas como `[ChillSpawn] Could not attach …`.
- **En Windows con DayZ Tools instalado**, el plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) para Claude Code añade agentes y habilidades específicas de DayZ que construyen PBOs e inician un servidor de prueba local, lo que cierra la brecha de "no probado".

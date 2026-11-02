# Cómo se creó ChillSpawn con Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · **Español** · [Português](PROMPTING.pt.md)

> Traducción automática. En caso de diferencias, prevalece el [original en inglés](PROMPTING.md).

ChillSpawn fue escrito en una sola sesión de Claude Code (Claude Opus 5.5) en una máquina Linux sin ninguna instalación de DayZ. Claude escribió cada archivo. La persona que lo dirigía redactó los requisitos, solicitó cambios y comparó el resultado con dos modelos locales. La primera versión se escribió sin compilar ni ejecutar nada. Más tarde, en la misma sesión, se configuró un servidor dedicado de Windows ejecutando DayZ 1.30 experimental, Claude lo alcanzó a través de SSH y el mod fue desplegado, probado en el juego y ampliado allí.

Esta página recorre las solicitudes por orden (reformuladas y pulidas, no citadas palabra por palabra), qué cambió cada una y qué se aprendió. Termina con prompts que puedes reutilizar para crear un mod similar.

## Los prompts, en orden

### 1. La primera solicitud

> Escribe un mod de servidor para DayZ que permita a un jugador generar un coche con un comando de chat:
>
> - El coche es el Olga rojo, con cuatro ruedas, una bujía y un radiador adjuntos.
> - Aparece 6 metros al norte del jugador.
> - El jugador recibe un mensaje privado indicando que el coche desaparecerá en 15 minutos.
> - Cada jugador puede generar 3 coches por cada hora de tiempo jugado.
> - Incluye un marco (framework) para que el mod pueda extenderse más adelante.
> - Una configuración JSON permite al dueño del servidor elegir qué generar: un coche con sus accesorios o un objeto.

Claude produjo el mod completo: `config.cpp`, la división de scripts en 3_Game/4_World/5_Mission, una configuración JSON escrita en `$profile:` al primer inicio, un registro de comandos (el "framework"), seguimiento del tiempo jugado guardado por jugador y un README. También señaló lo que no estaba seguro en lugar de ocultarlo:

- que el servidor recibe el chat a través de `MissionServer.OnEvent` con `ChatMessageEventTypeID`, y solo obtiene el nombre del remitente;
- que un Olga con solo ruedas, bujía y radiador **no conducirá** (sin batería ni fluidos);
- que los temporizadores de desaparición se pierden al reiniciar;
- que no había comprobación de obstáculos.

**Lección:** una lista numerada de requisitos concretos (vehículo exacto, distancia, dirección, límites) obtiene una respuesta completa de inmediato. Pide a Claude que liste sus suposiciones y cualquier cosa que no pudo verificar.

### 2. Hacerlo conducible y comprobar obstáculos

> Añade todo lo demás que necesite el coche para que aparezca listo para conducir. También comprueba los obstáculos antes de generarlo.

Se añadió batería (cargada), faros, puertas, capó y maletero, y se llenaron combustible, refrigerante, aceite y fluido de frenos (todo configurable). La comprobación de obstáculos utiliza `IsBoxColliding` con una caja del tamaño de un coche más una comprobación de mar/estanque. Prueba 10 direcciones a tres distancias. Si nada está libre, se informa al jugador y el intento no cuenta contra su límite.

**Lección:** los seguimientos que responden a la propia lista de "límites conocidos" de Claude son baratos y dirigidos.

### 3. Más de una cosa generable

> Haz que el ejemplo de configuración muestre varias cosas generables, por ejemplo, un hacha y el Ada 4x4.

La configuración ya lo soportaba (cada preset es su propio comando de chat). Claude añadió presets `!ada` y `!hatchet` tanto a los valores predeterminados como a `config.example.json`, y documentó cómo añadir más.

### 4. Comparando con modelos locales

> Pide a mi modelo local Qwen que escriba el mismo mod, luego compara su versión con la tuya: ¿cuánto difieren y tiene alguno errores?

Claude envió los mismos requisitos combinados a un modelo Qwen local, y más tarde revisó un segundo intento de Qwen proporcionado por el usuario. Ninguno habría funcionado:

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| API real de DayZ | Inventada en su mayoría (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Mayormente real |
| Se cargaría | No: no hay módulos de script, nunca se engancha a `MissionServer` | No: `config.cpp` no tiene `CfgMods` |
| Se compilaría | No | No: operador ternario, iteradores estilo C++, `ToLower()` usado como si devolviera una cadena |
| "Norte" | 6 m en el aire | 6 m al sur |
| Límite de tasa | En memoria, bloques fijos | Tiempo de actividad del servidor, se pierde al reiniciar |

**Lección:** para Enforce Script, los modelos de propósito general escriben confidentemente modismos de C++/Java que DayZ no soporta. Obtén una segunda opinión haciendo que un modelo revise el código de otro contra una lista de verificación (carga, compilación, comportamiento), en lugar de confiar en cualquiera de ellos.

### 5. Tomar prestada una buena idea

> Trae la idea de la segunda versión de Qwen sobre una lista de vehículos/objetos a ChillSpawn.

Qwen #2 tenía un subcomando `/vspawn list`. Claude añadió `!list` (cada preset con cuántas generaciones le quedan al jugador esta hora) y `!help` como comandos integrados en el framework de ChillSpawn.

### 6. Publicación

> Confirma esto en mi repositorio público de GitHub (licencia MIT; reemplaza el README si quieres). Asegúrate de que no haya credenciales incluidas. Añade un documento mostrando cómo se hizo el prompting y qué prompts podría usar otra persona para crear un mod similar.

Eso produjo este repositorio y esta página.

### 7. Comprobando contra la actualización 1.30

> Revisa las notas de la parche 1.30 y el rastreador de comentarios de Bohemia por cualquier cosa que pueda romper nuestra lógica.

Nada en las notas tocaba las APIs que usa el mod. La 1.30 trajo un compilador de scripts más estricto, así que se cambiaron dos constructos que podría rechazar (un método declarado sin cuerpo y una conversión de float a int).

### 8. Construyendo en Linux

> Ve si RaG DayZ Tools puede construir esto en Linux.

RaG DayZ Tools es principalmente para Windows, pero su constructor de PBO es Python puro y se ejecuta en Linux. `tools/build-linux.sh` lo usa para empaquetar el mod. La binarización, la conversión de configuración y la firma aún necesitan programas de Windows, y un mod de servidor solo de scripts no los necesita.

### 9. Probando en un servidor real

> He configurado un servidor dedicado de Windows con CFTools Architect y he abierto SSH para ti. Despliega el mod allí y comprueba que funciona.

El servidor ejecutaba DayZ 1.30 experimental, gestionado por CFTools Architect. A través de SSH, Claude:

- copió los `scripts.pbo` propios del juego desde el servidor, los descomprimió y comprobó cada función que llama el mod contra los scripts reales de la 1.30 experimental. Todos existían con parámetros coincidentes;
- construyó el PBO en Linux, copió `@ChillSpawn` a la carpeta raíz del servidor y pidió al dueño añadirlo en Architect con **Add Local Mod**, tipo **Server**;
- leyó los logs `.rpt` y de scripts del servidor después de cada reinicio para confirmar que los scripts se compilaban y el mod cargaba.

`!help` y `!car` funcionaron a la primera. El gancho de chat, la parte más propensa a fallar, funciona en un servidor sin mod de cliente.

**Lecciones:**

- **Dale a Claude los scripts reales del juego.** Descomprimir `scripts.pbo` desde el juego instalado convierte "esta API probablemente existe" en una comprobación rápida, y resuelve preguntas que las wikias ponen mal.
- **Cambia el PBO con el servidor totalmente detenido.** Un servidor en ejecución bloquea el archivo del mod, y el reinicio de Architect es demasiado rápido para capturarlo.

### 10. Administradores, límites, coches protegidos y kits

> Deja que los admins generen tanto como quieran. Limita a los jugadores a uno de cada coche cada 15 minutos. ¿Pueden los coches generados hacerse invulnerables al daño, o los choques dejar de herir a las personas dentro, sin un mod de cliente? Añade un `!adminkit` solo para admins con [una lista de armas, armadura y ropa], y un kit `!survive` que los jugadores puedan usar una vez cada 2 horas reales con [comida, agua, artículos médicos].

Claude leyó primero el código de coches de la 1.30. El daño a la tripulación en choques se aplica por script del lado del servidor (`CarScript.DamageCrew`), así que un mod de servidor puede apagarlo. El daño al propio coche se desactiva con `SetAllowDamage(false)`. Luego añadió:

- una lista `Admins` de IDs Steam64, sin límites y acceso a comandos solo para admins, que otros jugadores no pueden ver;
- límites como `MaxSpawns` por `LimitMinutes`, contados en tiempo jugado, o en tiempo real para kits que lo soliciten;
- opciones `Invulnerable` y `ProtectCrew` para presets;
- kits: objetos anidados (un arma con su cargador y silenciador, una mochila con su contenido), cargadores completos y agua, entregados al inventario o al suelo.

Todo funcionó en el juego en el primer despliegue.

**Lección:** preguntar "¿puede esto hacerse solo del lado del servidor?" antes de pedir la función obtiene una respuesta basada en el código real del juego, más sus límites: aquí, que las personas que golpea el coche siguen heridas, y la protección se pierde al reiniciar.

## Prompts que puedes reutilizar

### Iniciar un nuevo mod del lado del servidor

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

La línea "Enforce Script no tiene…" proviene directamente de los errores que cometieron los modelos locales. Dejarla ahí no cuesta nada y previene los errores más comunes.

### Seguimientos que funcionaron bien

- "Haz que el coche aparezca completamente conducible: cada parte que necesita, batería cargada y todos los fluidos llenos."
- "Añade una comprobación de obstáculos antes de generar. Si el lugar está bloqueado o en agua, prueba lugares cercanos; si ninguno está libre, informa al jugador y no lo cuentes contra su límite."
- "Añade un comando `!list` que muestre cada preset generable y cuántos le quedan al jugador."
- "Revisa este código [pega o adjunta] por cosas que no compilarán o cargarán en DayZ, y por errores lógicos contra estos requisitos: [lista]."
- "Escribe un plan de pruebas que pueda seguir en un servidor local para comprobar cada requisito."
- "Aquí tienes acceso SSH a mi servidor. Comprueba cada API que usa el mod contra los scripts propios del juego antes de desplegar."
- "¿Se puede hacer [característica] solo del lado del servidor? Revisa los scripts del juego y dime qué no puede cubrir."

### Consejos

- **Sé exacto con los objetos del juego.** "Olga rojo" se convirtió en `CivilianSedan_Wine`. Si conoces los nombres de clase, dáselos; si no, pide a Claude que liste los que usó para que puedas comprobarlos en el `types.xml` del servidor o con una herramienta de admin.
- **Di qué significa "por hora".** "Por hora de tiempo jugado" y "por hora de tiempo del servidor" son características diferentes, y solo una de ellas sobrevive a una reconexión.
- **Pide una sección de "límites conocidos"** y trátala como tu lista de pruebas.
- **Prueba en un servidor local antes de ir en vivo.** Observa el log `.rpt` del servidor buscando errores de script y líneas como `[ChillSpawn] Could not attach …`.
- **Dale a Claude acceso SSH a un servidor de prueba** si puedes. Así puede desplegar, leer los logs y comprobar los scripts propios del juego, lo que convierte conjeturas en comprobaciones.
- **En Windows con DayZ Tools instalado**, el plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) para Claude Code añade agentes y habilidades específicas de DayZ que construyen PBOs e inician un servidor de prueba local, lo que cierra la brecha de "no probado".

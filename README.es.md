# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · **Español** · [Português](README.pt.md)

> Traducción automática. En caso de diferencias, prevalece el [original en inglés](README.md).

Un mod de DayZ del lado del servidor. Los jugadores escriben `!car` en el chat para generar un Olga 24 rojo listo para conducir, situado seis metros al norte de ellos: con cuatro ruedas, bujía, radiador, batería cargada, faros, puertas, capó y maletero, con tanque de combustible, refrigerante, aceite y líquido de frenos llenos. Si ese punto está bloqueado o bajo el agua, intenta puntos cercanos; si ninguno está libre, se le indica al jugador que se mueva y el intento no cuenta contra su límite. Reciben un mensaje privado indicando que desaparecerá (despawn) en 15 minutos. Cada jugador puede generar un Olga cada 15 minutos de tiempo de juego (tiempo conectado, no tiempo real). Los coches generados no reciben daño, y nadie dentro sufre heridas al chocar.

Los jugadores también pueden escribir `!survive` una vez cada 2 horas reales para obtener un pequeño kit de supervivencia. Los administradores no tienen límites y obtienen un `!adminkit`.

Los clientes no necesitan el mod. Cárgalo con `-servermod=@ChillSpawn`. Escrito para DayZ 1.30.

> **Estado: probado en DayZ 1.30 experimental.** `!help`, `!list`, `!car`, `!ada` y `!hatchet` funcionan en un servidor en vivo sin mod de cliente. Los administradores, los kits, `!survive` y la protección de coches son nuevos y aún no se han probado. Ver [Límites conocidos / probar primero](#límites-conocidos--probar-primero). [docs/PROMPTING.md](docs/PROMPTING.es.md) muestra cómo fue creado.

## Estructura

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

## Configuración

Al primer inicio, el mod escribe `$profile:ChillSpawn/config.json` (en la carpeta `-profiles` del servidor). Cada entrada en `Presets` y `Kits` se convierte en su propio comando de chat, por lo que los propietarios del servidor pueden añadir más sin tocar el código. Los valores por defecto:

| Comando | Genera | Límite |
|---|---|---|
| `!car` | Olga 24 roja, lista para conducir, sin daño, tripulación protegida | 1 cada 15 min de juego |
| `!ada` | Ada 4x4 verde, lo mismo (`OffroadHatchback_Blue` / `_White` para otros colores, con clases de puerta, capó y maletero coincidentes `_Blue` / `_White`) | 1 cada 15 min de juego |
| `!hatchet` | Un hacha en el suelo a 1 m de distancia | 3 cada 60 min de juego |
| `!survive` | En el inventario del jugador: una lata de bacon, una botella llena de agua, tabletas purificadoras, una venda y tetraciclina | 1 cada 2 horas reales |
| `!adminkit` | Solo administradores. En el suelo frente a ellos: mochila Alice, M14 con 3 cargadores completos de 20 rondas y 3 cajas de .308, M4A1 con 3 cargadores completos de 60 rondas y 2 silenciadores, chaleco antibalas completo, casco MICH con visión nocturna y baterías, chaqueta camuflada TTsKO, pantalones y botas, guantes y un cuchillo de caza | ninguno |

Dos comandos integrados están siempre disponibles:

| Comando | Hace |
|---|---|
| `!list` | Lista todo lo que el jugador puede generar, uno por línea, con cuántos le quedan y, cuando no le quedan, cuánto falta para el siguiente |
| `!help` | Lista los comandos que el jugador puede usar |

Un preset o kit no puede llamarse `list` o `help`; los integrados ganan y el registro del servidor advierte.

### Administradores

Pon los IDs Steam64 de los administradores en `Admins`:

```json
"Admins": ["76561198000000000"],
```

Los administradores no tienen límites y pueden usar comandos marcados como `AdminOnly`. Otros jugadores no ven los comandos solo para admins en `!help` o `!list`, y no pasa nada si escriben uno.

### Presets

Cada preset tiene su propio límite (un Olga y un Ada cada 15 minutos son permisos separados). Para añadir un preset, copia un bloque en `Presets`, dale un nuevo `Command` y reinicia el servidor. Campos:

| Campo | Significado |
|---|---|
| `Command` | Comando de chat, sin el prefijo (`car` → `!car`) |
| `DisplayName` | Nombre mostrado en el mensaje |
| `ClassName` | Clase de vehículo o item a generar |
| `Attachments` | Clases adjuntadas a él. Repite una clase para cada ranura que ocupa (cuatro ruedas) |
| `DistanceMetres`, `BearingDegrees` | Dónde se genera relativo al jugador. 0° = norte, 90° = este |
| `DespawnSeconds` | 0 = nunca desaparecer |
| `MaxSpawns`, `LimitMinutes` | Cada jugador puede generarlo `MaxSpawns` veces cada `LimitMinutes` minutos de tiempo de juego |
| `AdminOnly` | Solo los administradores pueden usarlo |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutos hasta la desaparición |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Solo vehículos: qué tan lleno empieza cada fluido, de 0 a 1 |
| `Invulnerable` | Él y sus adjuntos no reciben ningún daño: choques, balas, explosiones |
| `ProtectCrew` | Solo vehículos: nadie dentro sufre heridas o muerte al chocar |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espacio libre (metros) necesario en el punto de generación. El valor por defecto cabe un Olga; áumentalo para camiones |

### Kits

Un kit es un conjunto de items para un comando. Campos:

| Campo | Significado |
|---|---|
| `Command`, `DisplayName` | Como en presets |
| `MaxSpawns`, `LimitMinutes` | Límite por jugador |
| `RealTimeLimit` | Cuenta `LimitMinutes` en tiempo real en lugar de tiempo de juego (sobrevive a reinicios) |
| `AdminOnly` | Solo los administradores pueden usarlo |
| `ToInventory` | Pon items en el inventario del jugador donde haya espacio; el resto va al suelo. De lo contrario, todo va al suelo frente a ellos |
| `Items` | `ClassName`, `Count` y `Contents`. Los contenidos van dentro del item: adjuntados donde quepan (cargadores, silenciadores, baterías, fundas), de lo contrario en su carga, y pueden tener sus propios contenidos |

Los items de los kits se generan impecables, cargadores y munición llenos, baterías cargadas y botellas llenas de agua. Un cargador listado en los contenidos de un arma está cargado con un cartucho en la recámara.

La configuración por defecto solo se escribe cuando no existe `config.json`. Después de actualizar el mod, elimina el archivo antiguo (o añade los nuevos campos) para obtener los nuevos valores por defecto; el registro del servidor advierte cuando el archivo es de una versión anterior.

El tiempo de juego por jugador y el historial de generación se guardan en `$profile:ChillSpawn/players/`.

## Añadir comandos (expansión)

Subclase `CS_ChatCommand` y regístralo desde otro mod, o en este mismo:

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

## Límites conocidos / probar primero

Lo básico funciona en DayZ 1.30 experimental. Comprueba esto en un servidor de prueba al cambiar la configuración o actualizar el juego:

1. **Gancho de chat: funciona.** El mod lee el chat a través de `MissionServer.OnEvent` con `ChatMessageEventTypeID`, por lo que los jugadores no necesitan instalar nada. La [Wiki de Modding StarDZ DayZ](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) lista `ChatMessageEventTypeID` como un evento del lado del cliente, pero en un servidor DayZ 1.30 experimental el servidor también lo recibe. El evento solo da el nombre del remitente, por lo que los jugadores se emparejan por nombre. Si dos jugadores comparten nombre, el comando se ignora.
2. **Nombres de clases de adjuntos y kits.** Comprueba que cada clase en `Attachments` y cada item de kit se genera (el registro del servidor imprime `Could not attach …`, `Could not create …` o `Could not put …` para cualquiera que no lo haga), y que el coche arranca y conduce.
3. **Protección de coches (nuevo).** `Invulnerable` detiene el daño al coche y a las partes con las que se generó; las partes ajustadas después no están protegidas. `ProtectCrew` detiene el daño por choque a las personas dentro. Ninguno protege a las personas que golpea el coche. Ambos se mantienen en memoria, por lo que tras un reinicio del servidor, un coche que aún no ha desaparecido vuelve a ser un coche normal.
4. **Comprobación de obstáculos.** Usa `IsBoxColliding` con una caja del tamaño del coche, elevada 0.3 m del suelo, más una comprobación de mar/estanque. Intenta 10 direcciones a 6, 9 y 12 m. Comprueba que no trata erróneamente los arbustos como obstáculos, y que detecta paredes y vallas. No comprueba la pendiente, por lo que un coche aún puede aterrizar en una ladera empinada.
5. **Los temporizadores de desaparición viven en memoria.** Si el servidor se reinicia antes de que expire el temporizador de un coche, el coche permanece en el mundo como un vehículo persistente normal. Si un jugador está dentro del coche cuando expira el temporizador, la desaparición se reintenta cada minuto hasta que salga.

## Construcción

### Linux

```bash
tools/build-linux.sh
```

Esto empaqueta `dist/@ChillSpawn/addons/ChillSpawn.pbo` con el constructor de PBO de [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools es freeware bajo su propia licencia y no está incluido aquí: el script lo clona en `~/.cache/rag-dayz-tools` en el primer inicio, o usa la ruta que pases (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) o configures en `RAG_TOOLS_DIR`. Necesita `python3` 3.10+ y `git`.

El script ejecuta las comprobaciones previas de RaG y empaqueta los scripts tal cual. Omite la binarización, la conversión de `config.cpp` y la firma, que requieren programas de DayZ Tools para Windows. Un mod de servidor solo de scripts no necesita ninguno de ellos. Nada en Linux compila Enforce Script, por lo que los errores de script solo aparecen en el registro `.rpt` del servidor al iniciar.

Copia `dist/@ChillSpawn` en la carpeta del servidor y añade `-servermod=@ChillSpawn` a la línea de inicio.

### Windows

Empaqueta la carpeta `ChillSpawn/` en `@ChillSpawn/addons/ChillSpawn.pbo` con el AddonBuilder de DayZ Tools o el RaG PBO Builder. Los mods solo para servidor no estrictamente necesitan firma, pero si el servidor verifica firmas, firma el PBO y pon el `.bikey` en la carpeta `keys/` del servidor.

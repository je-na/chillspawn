# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · **Español** · [Português](README.pt.md)

> Traducción automática. En caso de diferencias, prevalece el [original en inglés](README.md).

Un mod de DayZ del lado del servidor. Los jugadores escriben `!car` en el chat para generar un Olga 24 rojo listo para conducir a seis metros al norte de ellos: cuatro ruedas, bujía, radiador, batería cargada, faros, puertas, capó y maletero, con tanque lleno de combustible, refrigerante, aceite y fluido de frenos. Si ese lugar está bloqueado o en agua, intenta lugares cercanos; si ninguno está libre, se le indica al jugador que se mueva y el intento no cuenta contra su límite. Reciben un mensaje privado indicando que desaparecerá (despawn) dentro de 15 minutos. Cada jugador puede generar 3 vehículos por cada hora de tiempo de juego (tiempo conectado, no tiempo de reloj).

Los clientes no necesitan tener el mod. Cargarlo con `-servermod=@ChillSpawn`. Escrito para DayZ 1.30.

> **Estado: sin probar.** El código fue escrito con Claude y aún no se ha compilado ni ejecutado en un servidor. Consulta [Límites conocidos / prueba primero](#límites-conocidos--prueba-primero) antes de usarlo en un servidor en vivo. [docs/PROMPTING.md](docs/PROMPTING.es.md) muestra cómo se creó.

## Estructura

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

## Configuración

Al iniciar por primera vez, el mod escribe `$profile:ChillSpawn/config.json` (la carpeta `-profiles` del servidor). Cada preajuste en `Presets` se convierte en su propio comando de chat, por lo que los propietarios del servidor pueden agregar más vehículos o objetos sin tocar el código. El predeterminado tiene tres:

| Comando | Genera |
|---|---|
| `!car` | Olga 24 rojo, listo para conducir |
| `!ada` | Ada 4x4 verde, listo para conducir (`OffroadHatchback_Blue` / `_White` para otros colores, con clases de puerta, capó y maletero coincidentes `_Blue` / `_White`) |
| `!hatchet` | Un hacha en el suelo a 1 m de distancia |

Dos comandos integrados están siempre disponibles:

| Comando | Hace |
|---|---|
| `!list` | Lista cada preajuste, uno por línea, con cuántos generados le quedan al jugador esta hora de juego y, cuando no quedan más, cuánto falta para el siguiente |
| `!help` | Lista todos los comandos |

Un preajuste no puede llamarse `list` o `help`; los integrados tienen prioridad y el registro del servidor muestra una advertencia.

Cada preajuste tiene su propio límite (3 Olgas y 3 hachas por hora son permitencias separadas). Para agregar un preajuste, copia un bloque en `Presets`, dale un nuevo `Command` y reinicia el servidor. Campos:

| Campo | Significado |
|---|---|
| `Command` | Comando de chat, sin el prefijo (`car` → `!car`) |
| `DisplayName` | Nombre mostrado en el mensaje |
| `ClassName` | Clase de vehículo o objeto a generar |
| `Attachments` | Clases adjuntas a él. Repite una clase para cada ranura que ocupa (cuatro ruedas) |
| `DistanceMetres`, `BearingDegrees` | Dónde se genera relativo al jugador. 0° = norte, 90° = este |
| `DespawnSeconds` | 0 = nunca desaparecerá |
| `MaxPerPlaytimeHour` | Límite de generación por hora de tiempo de juego, contado por preajuste |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutos hasta la desaparición |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Solo vehículos: qué tan lleno está cada fluido al inicio, de 0 a 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espacio libre (metros) necesario en el punto de generación. El predeterminado cabe un Olga; súbelo para camiones |

La configuración predeterminada solo se escribe cuando no existe ningún `config.json`. Después de actualizar el mod, elimina el archivo antiguo (o agrega los nuevos campos a él) para adoptar los valores predeterminados nuevos.

El tiempo de juego por jugador y el historial de generación se guardan en `$profile:ChillSpawn/players/`.

## Agregar comandos (expansión)

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

## Límites conocidos / prueba primero

Aún no compilado ni probado. Verifica esto en un servidor de prueba primero:

1. **Gancho de chat (el más probable a fallar).** El mod lee el chat a través de `MissionServer.OnEvent` con `ChatMessageEventTypeID`, por lo que los jugadores no necesitan instalar nada. La [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) enumera `ChatMessageEventTypeID` como un evento del lado del cliente, y su tutorial de comandos de chat usa un pequeño mod del cliente en su lugar. Prueba esto primero: si `!help` no recibe respuesta, el servidor no está recibiendo eventos de chat y el mod necesitará una parte del lado del cliente. El evento solo da el nombre del remitente, por lo que los jugadores se emparejan por nombre. Si dos jugadores comparten un nombre, el comando se ignora.
2. **Nombres de clases de accesorios.** Verifica que cada clase en `Attachments` se adjunte correctamente (el registro del servidor imprime `Could not attach …` para cualquiera que no lo logre), y que el coche arranque y conduzca.
3. **Verificación de obstáculos.** Usa `IsBoxColliding` con una caja del tamaño del coche, levantada 0.3 m del suelo, más una verificación de mar/estanque. Intenta 10 direcciones a 6, 9 y 12 m. Verifica que no trate erróneamente los arbustos como obstáculos, y que detecte muros y cercas. No verifica la pendiente, por lo que un coche aún puede aterrizar en una ladera empinada.
4. **Los temporizadores de desaparición viven en memoria.** Si el servidor se reinicia antes de que expire el tiempo de un coche, el coche permanece en el mundo como un vehículo persistente normal. Si un jugador está dentro del coche cuando expira el temporizador, el intento de desaparición se reintentará cada minuto hasta que salga.

## Construcción

### Linux

```bash
tools/build-linux.sh
```

Esto empaqueta `dist/@ChillSpawn/addons/ChillSpawn.pbo` con el constructor de PBO de [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). RaG DayZ Tools es un software gratuito bajo su propia licencia y no está incluido aquí: el script lo clona en `~/.cache/rag-dayz-tools` en la primera ejecución, o usa la ruta que pases (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) o establezcas en `RAG_TOOLS_DIR`. Necesita `python3` 3.10+ y `git`.

El script ejecuta las verificaciones previas de RaG y empaqueta los scripts tal como están. Omite la binarización, la conversión de `config.cpp` y la firma, que requieren programas de DayZ Tools para Windows. Un mod del lado del servidor solo con scripts no necesita ninguno de ellos. Nada en Linux compila Enforce Script, por lo que los errores de script solo aparecen en el registro `.rpt` del servidor cuando este inicia.

Copia `dist/@ChillSpawn` en la carpeta del servidor y añade `-servermod=@ChillSpawn` a la línea de inicio.

### Windows

Empaqueta la carpeta `ChillSpawn/` en `@ChillSpawn/addons/ChillSpawn.pbo` con el AddonBuilder de DayZ Tools o el RaG PBO Builder. Los mods solo para servidor no estrictamente necesitan firma, pero si el servidor verifica firmas, firma el PBO y coloca el `.bikey` en la carpeta `keys/` del servidor.

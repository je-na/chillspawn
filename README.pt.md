# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · **Português**

> Tradução automática. Em caso de diferenças, vale o [original em inglês](README.md).

Um mod de DayZ do lado do servidor. Os jogadores digitam `!car` no chat para spawnar um Olga 24 vermelho pronto para dirigir a seis metros ao norte deles: quatro rodas, vela de ignição, radiador, bateria carregada, faróis, portas, capô e porta-malas, com tanque cheio de combustível, fluido de arrefecimento, óleo e fluido de freio. Se esse local estiver bloqueado ou submerso, o mod tenta locais próximos; se nenhum estiver livre, o jogador é instruído a se mover e a tentativa não conta contra seu limite. Eles recebem uma mensagem privada informando que o veículo será removido em 15 minutos. Cada jogador pode spawnar 3 veículos por hora de tempo de jogo (tempo conectado, não tempo real).

Os clientes não precisam do mod. Carregue-o com `-servermod=@ChillSpawn`. Escrito para DayZ 1.30.

> **Status: não testado.** O código foi escrito com Claude e ainda não foi compilado ou executado em um servidor. Veja [Limitações conhecidas / teste primeiro](#limitações-conhecidas--teste-primeiro) antes de usá-lo em um servidor ativo. [docs/PROMPTING.md](docs/PROMPTING.pt.md) mostra como ele foi criado.

## Estrutura

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

## Configuração

Na primeira inicialização, o mod escreve `$profile:ChillSpawn/config.json` (na pasta `-profiles` do servidor). Cada predefinição em `Presets` se torna seu próprio comando de chat, para que os proprietários do servidor possam adicionar mais veículos ou itens sem tocar no código. O padrão tem três:

| Comando | Spawna |
|---|---|
| `!car` | Olga 24 vermelha, pronta para dirigir |
| `!ada` | Ada 4x4 verde, pronta para dirigir (`OffroadHatchback_Blue` / `_White` para outras cores, com classes correspondentes de porta, capô e porta-malas `_Blue` / `_White`) |
| `!hatchet` | Um machado no chão a 1 m de distância |

Dois comandos embutidos estão sempre disponíveis:

| Comando | Faz |
|---|---|
| `!list` | Lista cada predefinição, uma por linha, com quantos spawns o jogador ainda tem nesta hora de jogo e, quando não restarem mais, quanto tempo falta para o próximo |
| `!help` | Lista todos os comandos |

Uma predefinição não pode ser chamada `list` ou `help`; os embutidos têm prioridade e o log do servidor emite um aviso.

Cada predefinição tem seu próprio limite (3 Olgas e 3 machados por hora são cotas separadas). Para adicionar uma predefinição, copie um bloco em `Presets`, dê a ele um novo `Command` e reinicie o servidor. Campos:

| Campo | Significado |
|---|---|
| `Command` | Comando de chat, sem o prefixo (`car` → `!car`) |
| `DisplayName` | Nome mostrado na mensagem |
| `ClassName` | Classe do veículo ou item para spawnar |
| `Attachments` | Classes anexadas a ele. Repita uma classe para cada slot que ela preenche (quatro rodas) |
| `DistanceMetres`, `BearingDegrees` | Onde é spawnado em relação ao jogador. 0° = norte, 90° = leste |
| `DespawnSeconds` | 0 = nunca despawnar |
| `MaxPerPlaytimeHour` | Limite de spawns por hora de tempo de jogo, contado por predefinição |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutos até o despawn |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Apenas veículos: quão cheio cada fluido começa, de 0 a 1 |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espaço livre (metros) necessário no ponto de spawn. O padrão cabe uma Olga; aumente para caminhões |

A configuração padrão é escrita apenas quando não existe um `config.json`. Após atualizar o mod, exclua o arquivo antigo (ou adicione os novos campos a ele) para aplicar as novas configurações padrão.

O tempo de jogo por jogador e o histórico de spawns são salvos em `$profile:ChillSpawn/players/`.

## Adicionando comandos (expansão)

Subclassifique `CS_ChatCommand` e registre-o de outro mod, ou neste mesmo:

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

## Limitações conhecidas / teste primeiro

Ainda não compilado ou testado. Verifique estes pontos em um servidor de teste primeiro:

1. **Hook de chat (mais provável de falhar).** O mod lê o chat através do `MissionServer.OnEvent` com `ChatMessageEventTypeID`, então os jogadores não precisam instalar nada. A [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) lista `ChatMessageEventTypeID` como um evento do lado do cliente, e seu tutorial de comando de chat usa um pequeno mod cliente em vez disso. Teste isso primeiro: se `!help` não receber resposta, o servidor não está recebendo eventos de chat, e o mod precisará de uma parte do lado do cliente. O evento fornece apenas o nome do remetente, então os jogadores são correspondidos por nome. Se dois jogadores compartilharem um nome, o comando será ignorado.
2. **Nomes das classes de acessórios.** Verifique se cada classe em `Attachments` é anexada corretamente (o log do servidor imprime `Could not attach …` para qualquer uma que não seja), e que o carro liga e dirige.
3. **Verificação de obstáculos.** Usa `IsBoxColliding` com uma caixa do tamanho do carro, elevada 0,3 m do chão, mais uma verificação de mar/pond. Tenta 10 direções a 6, 9 e 12 m. Verifique se ele não trata arbustos erroneamente como obstáculos, e se captura paredes e cercas. Não verifica inclinação, então um carro ainda pode aterrissar em uma encosta íngreme.
4. **Os timers de despawn vivem na memória.** Se o servidor reiniciar antes do timer de um carro acabar, o carro permanece no mundo como um veículo persistente normal. Se um jogador estiver no carro quando o timer acabar, o despawn será tentado a cada minuto até que eles saiam.

## Compilando

### Linux

```bash
tools/build-linux.sh
```

Isso empacota `dist/@ChillSpawn/addons/ChillSpawn.pbo` com o PBO builder do [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). O RaG DayZ Tools é freeware sob sua própria licença e não está incluído aqui: o script o clona para `~/.cache/rag-dayz-tools` na primeira execução, ou usa o caminho que você passar (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) ou definir em `RAG_TOOLS_DIR`. Necessita de `python3` 3.10+ e `git`.

O script executa as verificações preliminares do RaG e empacota os scripts conforme estão. Ele pula a binarização, a conversão de `config.cpp` e a assinatura, que precisam dos programas DayZ Tools do Windows. Um mod de servidor apenas com scripts não precisa de nenhum deles. Nada no Linux compila o Enforce Script, então erros de script só aparecem no log `.rpt` do servidor quando ele inicia.

Copie `dist/@ChillSpawn` para a pasta do servidor e adicione `-servermod=@ChillSpawn` à linha de inicialização.

### Windows

Empacote a pasta `ChillSpawn/` em `@ChillSpawn/addons/ChillSpawn.pbo` usando o AddonBuilder do DayZ Tools ou o RaG PBO Builder. Mods apenas para servidor não precisam estritamente de assinatura, mas se o servidor verificar assinaturas, assine o PBO e coloque o `.bikey` na pasta `keys/` do servidor.

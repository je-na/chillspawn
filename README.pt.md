# ChillSpawn

[English](README.md) · [Slovenčina](README.sk.md) · [Deutsch](README.de.md) · [Українська](README.uk.md) · [Polski](README.pl.md) · [Français](README.fr.md) · [Español](README.es.md) · **Português**

> Tradução automática. Em caso de diferenças, vale o [original em inglês](README.md).

Um mod de DayZ rodado no servidor. Os jogadores digitam `!car` no chat para gerar um Olga 24 vermelho pronto para dirigir, seis metros ao norte deles: quatro rodas, vela de ignição, radiador, bateria carregada, faróis, portas, capô e porta-malas, com tanque de combustível, líquido de arrefecimento, óleo e fluido de freio cheios. Se esse local estiver bloqueado ou na água, o mod tenta locais próximos; se nenhum estiver livre, o jogador é instruído a se mover e a tentativa não conta contra seu limite. Eles recebem uma mensagem privada informando que o veículo desaparece em 15 minutos. Cada jogador pode gerar um Olga a cada 15 minutos de tempo de jogo (tempo conectado, não tempo real). Os carros gerados não sofrem danos, e ninguém dentro deles é ferido ao sofrer acidente.

Os jogadores também podem digitar `!survive` uma vez a cada 2 horas reais para receber um pequeno kit de sobrevivência. Administradores não têm limites e recebem um `!adminkit`.

Os clientes não precisam do mod. Carregue-o com `-servermod=@ChillSpawn`. Desenvolvido para DayZ 1.30.

> **Status: testado no DayZ 1.30 experimental.** Todos os comandos, administradores, kits e proteção de veículos funcionam em um servidor ao vivo sem mod de cliente. Veja [Limitações conhecidas / teste primeiro](#limitações-conhecidas--teste-primeiro). [docs/PROMPTING.md](docs/PROMPTING.pt.md) mostra como o mod foi feito.

## Estrutura

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

## Configuração

Na primeira inicialização, o mod escreve `$profile:ChillSpawn/config.json` (pasta `-profiles` do servidor). Cada entrada em `Presets` e `Kits` se torna seu próprio comando de chat, então os proprietários de servidor podem adicionar mais sem tocar no código. Os padrões:

| Comando | Gera | Limite |
|---|---|---|
| `!car` | Olga 24 vermelha, pronta para dirigir, sem danos, tripulação protegida | 1 a cada 15 min de jogo |
| `!ada` | Ada 4x4 verde, o mesmo (`OffroadHatchback_Blue` / `_White` para outras cores, com classes de porta, capô e porta-malas correspondentes `_Blue` / `_White`) | 1 a cada 15 min de jogo |
| `!hatchet` | Um machado no chão a 1 m de distância | 3 a cada 60 min de jogo |
| `!survive` | No inventário do jogador: uma lata de bacon, uma garrafa cheia de água, comprimidos de purificação, um curativo e tetraciclina | 1 a cada 2 horas reais |
| `!adminkit` | Apenas para administradores. No chão à frente deles: mochila Alice, M14 com 3 carregadores cheios de 20 tiros e 3 caixas de .308, M4A1 com 3 carregadores cheios de 60 tiros e 2 silenciadores, colete completo, capacete MICH com visão noturna e baterias, jaqueta camuflada TTsKO, calças e botas, luvas e uma faca de caça | nenhum |

Dois comandos internos estão sempre disponíveis:

| Comando | Faz |
|---|---|
| `!list` | Lista tudo o que o jogador pode gerar, um por linha, com quantos ele tem restantes e, quando não restarem, quanto tempo falta para o próximo |
| `!help` | Lista os comandos que o jogador pode usar |

Um preset ou kit não pode ser chamado `list` ou `help`; os internos têm precedência e o log do servidor emite um aviso.

### Administradores

Coloque os IDs Steam64 dos administradores em `Admins`:

```json
"Admins": ["76561198000000000"],
```

Os administradores não têm limites e podem usar comandos marcados como `AdminOnly`. Outros jogadores não veem comandos exclusivos de administradores no `!help` ou `!list`, e nada acontece se eles digitarem um.

### Presets

Cada preset tem seu próprio limite (um Olga e um Ada a cada 15 minutos são cotas separadas). Para adicionar um preset, copie um bloco em `Presets`, dê a ele um novo `Command` e reinicie o servidor. Campos:

| Campo | Significado |
|---|---|
| `Command` | Comando de chat, sem o prefixo (`car` → `!car`) |
| `DisplayName` | Nome mostrado na mensagem |
| `ClassName` | Classe do veículo ou item para gerar |
| `Attachments` | Classes anexadas a ele. Repita uma classe para cada slot que ela ocupa (quatro rodas) |
| `DistanceMetres`, `BearingDegrees` | Onde ele surge em relação ao jogador. 0° = norte, 90° = leste |
| `DespawnSeconds` | 0 = nunca desaparece |
| `MaxSpawns`, `LimitMinutes` | Cada jogador pode gerá-lo `MaxSpawns` vezes a cada `LimitMinutes` minutos de tempo de jogo |
| `AdminOnly` | Apenas administradores podem usá-lo |
| `SpawnMessage` | `%1` = DisplayName, `%2` = minutos até desaparecer |
| `FuelFraction`, `CoolantFraction`, `OilFraction`, `BrakeFluidFraction` | Apenas veículos: quão cheio começa cada fluido, de 0 a 1 |
| `Invulnerable` | O veículo e seus anexos não sofrem nenhum dano: colisões, balas, explosões |
| `ProtectCrew` | Apenas veículos: ninguém dentro é ferido ou morto ao sofrer acidente |
| `ClearanceWidth`, `ClearanceLength`, `ClearanceHeight` | Espaço livre (metros) necessário no ponto de geração. O padrão cabe um Olga; aumente para caminhões |

### Kits

Um kit é um conjunto de itens para um único comando. Campos:

| Campo | Significado |
|---|---|
| `Command`, `DisplayName` | Igual aos presets |
| `MaxSpawns`, `LimitMinutes` | Limite por jogador |
| `RealTimeLimit` | Conta `LimitMinutes` em tempo real em vez de tempo de jogo (sobrevive a reinicializações) |
| `AdminOnly` | Apenas administradores podem usá-lo |
| `ToInventory` | Coloca os itens no inventário do jogador onde houver espaço; o resto vai para o chão. Caso contrário, tudo vai para o chão à frente deles |
| `Items` | `ClassName`, `Count` e `Contents`. Os conteúdos vão para dentro do item: anexados onde cabem (carregadores, silenciadores, baterias, coldres), caso contrário no seu cargueiro, e podem ter seus próprios conteúdos |

Os itens do kit são gerados novos, carregadores e munição cheios, baterias carregadas e garrafas cheias de água. Um carregador listado nos conteúdos de uma arma é carregado com um tiro na câmara.

A configuração padrão é escrita apenas quando não existe nenhum `config.json`. Após atualizar o mod, exclua o arquivo antigo (ou adicione os novos campos a ele) para pegar as novas configurações padrão; o log do servidor emite um aviso quando o arquivo é de uma versão mais antiga.

O tempo de jogo por jogador e o histórico de geração são salvos em `$profile:ChillSpawn/players/`.

## Adicionando comandos (expansão)

Subclassifique `CS_ChatCommand` e registre-o a partir de outro mod, ou neste:

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

O básico funciona no DayZ 1.30 experimental. Verifique estes pontos em um servidor de teste ao alterar a configuração ou atualizar o jogo:

1. **Gancho de chat: funciona.** O mod lê o chat através de `MissionServer.OnEvent` com `ChatMessageEventTypeID`, então os jogadores não precisam instalar nada. A [StarDZ DayZ Modding Wiki](https://github.com/StarDZ-Team/DayZ-Modding-Wiki) lista `ChatMessageEventTypeID` como um evento do lado do cliente, mas em um servidor DayZ 1.30 experimental o servidor também o recebe. O evento fornece apenas o nome do remetente, então os jogadores são correspondidos pelo nome. Se dois jogadores compartilharem um nome, o comando será ignorado.
2. **Nomes de classes de anexos e kits.** Verifique se cada classe em `Attachments` e cada item do kit é gerado (o log do servidor imprime `Could not attach …`, `Could not create …` ou `Could not put …` para qualquer um que não funcione), e que o carro liga e dirige.
3. **Proteção de veículos.** `Invulnerable` impede danos ao carro e às peças com que ele foi gerado; as partes ajustadas posteriormente não são protegidas. `ProtectCrew` impede danos por colisão às pessoas dentro dele. Nenhum protege as pessoas atingidas pelo carro. Ambos são mantidos na memória, então após uma reinicialização do servidor, um carro que ainda não desapareceu torna-se um veículo persistente comum novamente.
4. **Verificação de obstáculos.** Usa `IsBoxColliding` com uma caixa do tamanho do carro, elevada 0,3 m do chão, além de uma verificação de mar/piscina. Tenta 10 direções a 6, 9 e 12 m. Verifique se não trata arbustos erroneamente como obstáculos, e se detecta paredes e cercas. Não verifica inclinação, então um carro ainda pode pousar em uma encosta íngreme.
5. **Os temporizadores de desaparecimento vivem na memória.** Se o servidor reiniciar antes do tempo do carro acabar, o carro permanece no mundo como um veículo persistente normal. Se um jogador estiver no carro quando o tempo acabar, o desaparecimento será tentado a cada minuto até que ele saia.

## Construindo

### Linux

```bash
tools/build-linux.sh
```

Isso empacota `dist/@ChillSpawn/addons/ChillSpawn.pbo` com o construtor de PBO do [RaG DayZ Tools](https://github.com/Tyson89/RaG-DayZ-Tools). O RaG DayZ Tools é freeware sob sua própria licença e não está incluído aqui: o script o clona para `~/.cache/rag-dayz-tools` na primeira execução, ou usa o caminho que você passa (`tools/build-linux.sh /path/to/RaG-DayZ-Tools`) ou define em `RAG_TOOLS_DIR`. Precisa de `python3` 3.10+ e `git`.

O script executa as verificações preliminares do RaG e empacota os scripts conforme estão. Ele ignora a binarização, a conversão do `config.cpp` e a assinatura, que precisam dos programas DayZ Tools para Windows. Um mod de servidor apenas com scripts não precisa de nenhum deles. Nada no Linux compila o Enforce Script, então erros de script só aparecem no log `.rpt` do servidor quando ele inicia.

### Windows

Empacote a pasta `ChillSpawn/` em `@ChillSpawn/addons/ChillSpawn.pbo` com o AddonBuilder ou o RaG PBO Builder dos DayZ Tools. Mods apenas para servidor não precisam estritamente de assinatura, mas se o servidor verificar assinaturas, assine o PBO e coloque o `.bikey` na pasta `keys/` do servidor.

## Instalando

1. Copie a pasta `@ChillSpawn` para a pasta raiz do servidor, ao lado de `DayZServer_x64.exe`.
2. Carregue-o como um mod de servidor:
   - **Servidor puro:** adicione `-servermod=@ChillSpawn` à linha de inicialização.
   - **CFTools Architect:** use **Add Local Mod**, escolha `@ChillSpawn` e defina seu tipo como **Server**.
3. Inicie o servidor. Ele escreve `profiles/ChillSpawn/config.json` e registra `[ChillSpawn] Loaded … spawn presets, … kits and … admins.`
4. Adicione os IDs Steam64 dos seus administradores em `Admins` nesse arquivo e reinicie.

**Atualizando:** pare completamente o servidor antes de substituir o `ChillSpawn.pbo`. Um servidor em execução bloqueia o arquivo, e uma reinicialização rápida pode não deixar tempo para trocá-lo.

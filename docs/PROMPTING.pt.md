# Como o ChillSpawn foi criado com o Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · **Português**

> Tradução automática. Em caso de diferenças, vale o [original em inglês](PROMPTING.md).

O ChillSpawn foi escrito em uma única sessão do Claude Code (Claude Opus 5.5) em uma máquina Linux sem instalação de DayZ. O Claude escreveu todos os arquivos. A pessoa que orientava o processo escreveu os requisitos, pediu alterações e comparou o resultado com dois modelos locais. Nada foi compilado ou executado no jogo durante a sessão, portanto, trate o código como um forte rascunho inicial que ainda precisa de um servidor de teste.

Esta página percorre as solicitações em ordem (reformuladas e organizadas, não citadas palavra por palavra), o que cada uma mudou e o que foi aprendido. Termina com prompts que você pode reutilizar para criar um mod similar.

## Os prompts, em ordem

### 1. O primeiro pedido

> Escreva um mod de servidor do DayZ que permita a um jogador gerar um carro com um comando de chat:
>
> - O carro é o Olga vermelho, com quatro rodas, uma vela de ignição e um radiador acoplados.
> - Ele aparece 6 metros ao norte do jogador.
> - O jogador recebe uma mensagem privada informando que o carro será desativado dentro de 15 minutos.
> - Cada jogador pode gerar 3 carros por hora de tempo gasto no jogo.
> - Inclua uma estrutura para que o mod possa ser estendido posteriormente.
> - Uma configuração JSON permite que o dono do servidor escolha o que gerar: um carro mais seus acessórios, ou um item.

O Claude produziu o mod inteiro: `config.cpp`, a divisão de scripts 3_Game/4_World/5_Mission, uma configuração JSON escrita em `$profile:` na primeira inicialização, um registro de comandos (a "estrutura"), rastreamento de tempo de jogo salvo por jogador e um README. Ele também sinalizou o que não tinha certeza em vez de esconder:

- que o servidor recebe o chat através de `MissionServer.OnEvent` com `ChatMessageEventTypeID`, e recebe apenas o nome do remetente;
- que um Olga com apenas rodas, vela de ignição e radiador **não dirigirá** (sem bateria ou fluidos);
- que os temporizadores de desativação são perdidos na reinicialização;
- que não havia verificação de obstáculos.

**Lição:** uma lista numerada de requisitos concretos (veículo exato, distância, direção, limites) gera uma resposta completa de uma vez. Peça ao Claude para listar suas suposições e qualquer coisa que ele não pudesse verificar.

### 2. Torná-lo dirigível e verificar obstáculos

> Adicione tudo o mais que o carro precisa para nascer pronto para dirigir. Também verifique obstáculos antes de gerar o veículo.

Adicionou bateria (carregada), faróis, portas, capô e porta-malas, e preencheu combustível, fluido de arrefecimento, óleo e fluido de freio (todos configuráveis). A verificação de obstáculos usa `IsBoxColliding` com uma caixa do tamanho do carro mais uma verificação de mar/pond. Tenta 10 direções em três distâncias. Se nada estiver livre, o jogador é informado e a tentativa não conta contra seu limite.

**Lição:** seguimentos que respondem à própria lista de "limites conhecidos" do Claude são baratos e direcionados.

### 3. Mais de uma coisa gerável

> Faça com que a configuração de exemplo mostre várias coisas geráveis, por exemplo, um machado e o Ada 4x4.

A configuração já suportava isso (cada preset é seu próprio comando de chat). O Claude adicionou os presets `!ada` e `!hatchet` tanto aos padrões quanto ao `config.example.json`, e documentou como adicionar mais.

### 4. Comparando com modelos locais

> Peça ao meu modelo local Qwen para escrever o mesmo mod, depois compare sua versão com a sua: quão diferentes são e algum deles tem erros?

O Claude enviou os mesmos requisitos combinados a um modelo Qwen local, e posteriormente revisou uma segunda tentativa do Qwen fornecida pelo usuário. Nenhum funcionaria:

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| API Real do DayZ | Maiitariamente inventada (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Maioritariamente real |
| Carregaria | Não: sem módulos de script, nunca conecta ao `MissionServer` | Não: `config.cpp` não tem `CfgMods` |
| Compilaria | Não | Não: operador ternário, iteradores estilo C++, `ToLower()` usado como se retornasse uma string |
| "Norte" | 6 m acima no ar | 6 m ao sul |
| Limite de taxa | Em memória, blocos fixos | Uptime do servidor, perdido na reinicialização |

**Lição:** para o Enforce Script, modelos de propósito geral escrevem confidentemente construções C++/Java que o DayZ não suporta. Obtenha uma segunda opinião tendo um modelo revisar o código de outro contra uma lista de verificação (carregar, compilar, comportamento), em vez de confiar em qualquer um deles.

### 5. Pegando uma boa ideia emprestada

> Traga a ideia da segunda versão do Qwen de uma lista de veículos/itens para o ChillSpawn.

O Qwen #2 tinha um subcomando `/vspawn list`. O Claude adicionou `!list` (cada preset com quantos spawns o jogador tem restantes nesta hora) e `!help` como comandos embutidos na estrutura do ChillSpawn.

### 6. Publicação

> Faça commit disso no meu repositório público do GitHub (licença MIT; substitua o README se quiser). Certifique-se de que nenhuma credencial seja incluída. Adicione um documento mostrando como o prompting foi feito e quais prompts alguém poderia usar para criar um mod similar.

Isso produziu este repositório e esta página.

### 7. Verificando contra a atualização 1.30

> Verifique as notas de patch da 1.30 e o rastreador de feedback da Bohemia para qualquer coisa que possa quebrar nossa lógica.

Nada nas notas tocava nas APIs que o mod usa. A 1.30 trouxe um compilador de script mais rigoroso, então duas construções que ele poderia rejeitar foram alteradas (um método declarado sem corpo e uma conversão de float para int).

### 8. Construindo no Linux

> Veja se as RaG DayZ Tools podem construir isso no Linux.

As RaG DayZ Tools são focadas em Windows, mas seu builder de PBO é Python puro e roda no Linux. `tools/build-linux.sh` o usa para empacotar o mod. Binarizar, converter configuração e assinar ainda precisam de programas do Windows, e um mod de servidor apenas com scripts não precisa deles.

## Prompts que você pode reutilizar

### Iniciando um novo mod de servidor

Copie isso, preencha os colchetes e cole-o no Claude (o Claude Code funciona melhor, já que ele escreve os arquivos para você):

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

A linha "O Enforce Script não tem…" vem diretamente dos erros que os modelos locais cometeram. Deixá-la lá não custa nada e evita os erros mais comuns.

### Seguintes que funcionaram bem

- "Faça o veículo nascer totalmente dirigível: cada parte de que precisa, bateria carregada e todos os fluidos cheios."
- "Adicione uma verificação de obstáculos antes de gerar. Se o local estiver bloqueado ou na água, tente locais próximos; se nenhum estiver livre, avise o jogador e não conte contra seu limite."
- "Adicione um comando `!list` mostrando cada preset gerável e quantos o jogador tem restantes."
- "Revise este código [cole ou anexe] para coisas que não compilarão ou carregarão no DayZ, e para erros de lógica contra estes requisitos: [lista]."
- "Escreva um plano de teste que eu possa seguir em um servidor local para verificar cada requisito."

### Dicas

- **Seja exato sobre objetos do jogo.** "Olga vermelho" tornou-se `CivilianSedan_Wine`. Se você souber os nomes das classes, forneça-os; se não, peça ao Claude para listar aqueles que usou para que você possa verificá-los no `types.xml` do servidor ou com uma ferramenta de admin.
- **Diga o que "por hora" significa.** "Por hora de tempo de jogo" e "por hora de tempo do servidor" são recursos diferentes, e apenas um deles sobrevive a uma reconexão.
- **Peça uma seção de "limites conhecidos"** e trate-a como sua lista de testes.
- **Teste em um servidor local antes de ir para produção.** Observe o log `.rpt` do servidor em busca de erros de script e linhas como `[ChillSpawn] Could not attach …`.
- **No Windows com DayZ Tools instalado**, o plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) para o Claude Code adiciona agentes e habilidades específicas do DayZ que constroem PBOs e lançam um servidor de teste local, o que fecha a lacuna de "não testado".

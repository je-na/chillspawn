# Como o ChillSpawn foi criado com o Claude

[English](PROMPTING.md) · [Slovenčina](PROMPTING.sk.md) · [Deutsch](PROMPTING.de.md) · [Українська](PROMPTING.uk.md) · [Polski](PROMPTING.pl.md) · [Français](PROMPTING.fr.md) · [Español](PROMPTING.es.md) · **Português**

> Tradução automática. Em caso de diferenças, vale o [original em inglês](PROMPTING.md).

O ChillSpawn foi escrito em uma única sessão do Claude Code (Claude Opus 5.5) em uma máquina Linux sem instalação do DayZ. O Claude escreveu todos os arquivos. A pessoa que o orientava definiu os requisitos, pediu alterações e comparou o resultado com dois modelos locais. A primeira versão foi escrita sem compilar ou executar nada. Mais tarde, na mesma sessão, um servidor dedicado no Windows executando o DayZ 1.30 experimental foi configurado, o Claude acessou-o via SSH e o mod foi implantado, testado no jogo e expandido a partir daí.

Esta página percorre os pedidos em ordem (reescritos e organizados, não citados palavra por palavra), o que cada um alterou e o que foi aprendido. Termina com prompts que você pode reutilizar para criar um mod semelhante.

## Os prompts, em ordem

### 1. O primeiro pedido

> Escreva um mod de servidor do DayZ que permita a um jogador gerar um carro com um comando de chat:
>
> - O carro é o Olga vermelho, com quatro rodas, uma vela de ignição e um radiador anexados.
> - Ele aparece 6 metros ao norte do jogador.
> - O jogador recebe uma mensagem privada informando que o carro desaparecerá dentro de 15 minutos.
> - Cada jogador pode gerar 3 carros por hora de tempo gasto no jogo.
> - Inclua uma estrutura para que o mod possa ser expandido posteriormente.
> - Uma configuração JSON permite ao proprietário do servidor escolher o que gerar: um carro mais seus acessórios, ou um item.

O Claude produziu o mod inteiro: `config.cpp`, a divisão de scripts em 3_Game/4_World/5_Mission, uma configuração JSON escrita em `$profile:` na primeira inicialização, um registro de comandos (a "estrutura"), rastreamento de tempo de jogo salvo por jogador e um README. Ele também sinalizou o que não tinha certeza, em vez de esconder:

- que o servidor recebe o chat através de `MissionServer.OnEvent` com `ChatMessageEventTypeID`, e recebe apenas o nome do remetente;
- que um Olga apenas com rodas, vela de ignição e radiador **não dirigirá** (sem bateria ou fluidos);
- que os temporizadores de desaparecimento são perdidos na reinicialização;
- que não havia verificação de obstáculos.

**Lição:** uma lista numerada de requisitos concretos (veículo exato, distância, direção, limites) obtém uma resposta completa de uma vez. Peça ao Claude para listar suas suposições e qualquer coisa que ele não pudesse verificar.

### 2. Torná-lo dirigível e verificar obstáculos

> Adicione tudo o mais que o carro precisa para nascer pronto para dirigir. Também verifique se há obstáculos antes de gerá-lo.

Adicionou bateria (carregada), faróis, portas, capô e porta-malas, e preencheu combustível, fluido de arrefecimento, óleo e fluido de freio (tudo configurável). A verificação de obstáculos usa `IsBoxColliding` com uma caixa do tamanho de um carro mais uma verificação de mar/piscina. Tenta 10 direções em três distâncias. Se nada estiver livre, o jogador é informado e a tentativa não conta contra seu limite.

**Lição:** follow-ups que respondem à própria lista de "limites conhecidos" do Claude são baratos e direcionados.

### 3. Mais de uma coisa gerável

> Faça com que a configuração de exemplo mostre várias coisas geráveis, por exemplo, um machado e o Ada 4x4.

A configuração já suportava isso (cada preset é seu próprio comando de chat). O Claude adicionou presets `!ada` e `!hatchet` tanto aos padrões quanto ao `config.example.json`, e documentou como adicionar mais.

### 4. Comparando com modelos locais

> Faça meu modelo local Qwen escrever o mesmo mod, depois compare sua versão com a sua: quão diferentes são, e algum deles tem erros?

O Claude enviou os mesmos requisitos combinados para um modelo Qwen local, e depois revisou uma segunda tentativa do Qwen fornecida pelo usuário. Nenhuma teria funcionado:

| | Local Qwen #1 | Local Qwen #2 |
|---|---|---|
| API Real do DayZ | Majoritariamente inventada (`InstallPart`, `TestSphere`, `player.ChatMessage`) | Majoritariamente real |
| Carregaria | Não: sem módulos de script, nunca conecta ao `MissionServer` | Não: `config.cpp` não tem `CfgMods` |
| Compilaria | Não | Não: operador ternário, iteradores estilo C++, `ToLower()` usado como se retornasse uma string |
| "Norte" | 6 m acima no ar | 6 m ao sul |
| Limite de taxa | Em memória, blocos fixos | Uptime do servidor, perdido na reinicialização |

**Lição:** para o Enforce Script, modelos de propósito geral escrevem confidentemente C++/Java-isms que o DayZ não suporta. Obtenha uma segunda opinião tendo um modelo revisar o código de outro contra uma lista de verificação (carregamento, compilação, comportamento), em vez de confiar em qualquer um deles.

### 5. Pegando uma boa ideia emprestada

> Traga a ideia da versão do segundo Qwen de uma lista de veículos/itens para o ChillSpawn.

O Qwen #2 tinha um subcomando `/vspawn list`. O Claude adicionou `!list` (cada preset com quantos spawns o jogador tem restantes esta hora) e `!help` como comandos integrados na estrutura do ChillSpawn.

### 6. Publicação

> Faça commit disso no meu repositório público do GitHub (licença MIT; substitua o README se quiser). Certifique-se de que nenhuma credencial seja incluída. Adicione um documento mostrando como a interação foi feita e quais prompts outra pessoa poderia usar para fazer um mod semelhante.

Isso produziu este repositório e esta página.

### 7. Verificando contra a atualização 1.30

> Verifique as notas de patch do 1.30 e o rastreador de feedback da Bohemia para qualquer coisa que possa quebrar nossa lógica.

Nada nas notas tocava nas APIs que o mod usa. O 1.30 trouxe um compilador de script mais rigoroso, então duas construções que ele poderia rejeitar foram alteradas (um método declarado sem corpo e uma conversão de float para int).

### 8. Construindo no Linux

> Veja se o RaG DayZ Tools pode construir isso no Linux.

O RaG DayZ Tools é focado em Windows, mas seu construtor de PBO é Python puro e roda no Linux. `tools/build-linux.sh` usa-o para empacotar o mod. Binarizar, converter configurações e assinar ainda precisam de programas do Windows, e um mod de servidor apenas de scripts não precisa deles.

### 9. Testando em um servidor real

> Configurei um servidor dedicado no Windows com CFTools Architect e abri SSH para você. Implante o mod lá e verifique se funciona.

O servidor executava o DayZ 1.30 experimental, gerenciado pelo CFTools Architect. Via SSH, o Claude:

- copiou o `scripts.pbo` próprio do jogo do servidor, descompactou-o e verificou cada função chamada pelo mod contra os scripts reais do 1.30 experimental. Todos existiam com parâmetros correspondentes;
- construiu o PBO no Linux, copiou `@ChillSpawn` para a pasta raiz do servidor e pediu ao proprietário para adicioná-lo no Architect com **Add Local Mod**, tipo **Server**;
- leu o `.rpt` do servidor e os logs de script após cada reinicialização para confirmar que os scripts foram compilados e o mod carregou.

`!help` e `!car` funcionaram na primeira tentativa. O hook de chat, a parte mais propensa a falhar, funciona em um servidor sem mod de cliente.

**Lições:**

- **Dê ao Claude os scripts reais do jogo.** Descompactar o `scripts.pbo` do jogo instalado transforma "essa API provavelmente existe" em uma verificação rápida, e resolve questões que as wikis erram.
- **Troque o PBO com o servidor totalmente parado.** Um servidor em execução bloqueia o arquivo do mod, e a reinicialização do Architect é rápida demais para capturar.

### 10. Administradores, limites, carros protegidos e kits

> Permita que administradores gerem tanto quanto quiserem. Limite os jogadores a um de cada carro a cada 15 minutos. Os carros gerados podem ser feitos à prova de dano, ou colisões param de machucar as pessoas dentro, sem um mod de cliente? Adicione um `!adminkit` apenas para admin com [uma lista de armas, armadura e roupas], e um kit `!survive` que os jogadores podem usar uma vez a cada 2 horas reais com [comida, água, itens médicos].

O Claude leu primeiro o código do carro 1.30. O dano da colisão à tripulação é aplicado por script no lado do servidor (`CarScript.DamageCrew`), então um mod de servidor pode desativá-lo. O dano ao próprio carro é desativado com `SetAllowDamage(false)`. Então ele adicionou:

- uma lista `Admins` de IDs Steam64, sem limites e acesso a comandos apenas para admin, que outros jogadores não podem ver;
- limites como `MaxSpawns` por `LimitMinutes`, contados no tempo de jogo, ou em tempo real para kits que pedem isso;
- opções `Invulnerable` e `ProtectCrew` para presets;
- kits: itens aninhados (uma arma com seu carregador e silenciador, uma mochila com seus conteúdos), carregadores cheios e água, entregues no inventário ou no chão.

Tudo funcionou no jogo na primeira implantação.

**Lição:** perguntar "isso pode ser feito apenas no lado do servidor?" antes de pedir o recurso obtém uma resposta fundamentada no código real do jogo, além de seus limites: aqui, que as pessoas atingidas pelo carro ainda são machucadas, e a proteção é perdida na reinicialização.

## Prompts que você pode reutilizar

### Iniciando um novo mod de servidor

Copie isso, preencha os colchetes e cole no Claude (o Claude Code funciona melhor, já que ele escreve os arquivos para você):

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

A linha "Enforce Script has no…" vem diretamente dos erros que os modelos locais cometeram. Deixá-la lá não custa nada e previne os erros mais comuns.

### Follow-ups que funcionaram bem

- "Torne o veículo gerado totalmente dirigível: todas as partes de que precisa, bateria carregada e todos os fluidos cheios."
- "Adicione uma verificação de obstáculos antes de gerar. Se o local estiver bloqueado ou na água, tente locais próximos; se nenhum estiver livre, avise o jogador e não conte contra seu limite."
- "Adicione um comando `!list` mostrando cada preset gerável e quantos restam ao jogador."
- "Revise este código [cole ou anexe] por coisas que não compilarão ou carregarão no DayZ, e por erros de lógica contra estes requisitos: [lista]."
- "Escreva um plano de teste que eu possa seguir em um servidor local para verificar cada requisito."
- "Aqui está o acesso SSH ao meu servidor. Verifique todas as APIs que o mod usa contra os scripts do jogo antes de implantar."
- "[Recurso] pode ser feito apenas no lado do servidor? Verifique os scripts do jogo e me diga o que ele não cobre."

### Dicas

- **Seja exato sobre objetos do jogo.** "Olga vermelho" tornou-se `CivilianSedan_Wine`. Se você souber os nomes das classes, forneça-os; se não, peça ao Claude para listar as que usou para que você possa verificá-las no `types.xml` do servidor ou com uma ferramenta de admin.
- **Diga o que "por hora" significa.** "Por hora de tempo de jogo" e "por hora de tempo do servidor" são recursos diferentes, e apenas um deles sobrevive a uma reconexão.
- **Peça uma seção de "limites conhecidos"** e trate-a como sua lista de testes.
- **Teste em um servidor local antes de ir ao vivo.** Observe o log `.rpt` do servidor por erros de script e linhas como `[ChillSpawn] Could not attach …`.
- **Dê ao Claude acesso SSH a um servidor de teste** se puder. Ele pode então implantar, ler os logs e verificar os scripts próprios do jogo, transformando palpites em verificações.
- **No Windows com o DayZ Tools instalado**, o plugin [Agentic-Z](https://github.com/DayZ-n-Chill/Agentic-Z) para o Claude Code adiciona agentes e habilidades específicos do DayZ que constroem PBOs e lançam um servidor de teste local, fechando a lacuna de "não testado".

## Mesh Network – mobilt trådløst mesh-nettverk
This was our submission for the one week project in the course Network Programming and Computer Communication. This was a voluntary exam and accounted for half the grade basis in the course, as we had a mandatory computer com. exam as well. It was a two man project with a farily open case. The only demand was to produce something regarding mesh networks. Since we wanted to prioritize the learning experience, we decided to try our hands on some C++ programming. The learning curve was quite steap as we had never programmed in C++ before, but we are happy with our little program(even though it's not the most refined code we have created).

## Introduksjon

Mesh Network er et mobilt trådløst mesh-nettverk designet for kontrollering av noder i en dimensjon. Programmet legger til rette for tilkobling av noder til en sentral server. Denne styrer nodens plassering i mesh-nettverket og hvilken prioritet noden har. Programmet gjør det også mulig å erstatte noder i nettverket. Formålet med programmet er å skape et nettverk der mobile enheter danner et mesh-nettverk og kan kommunisere gjennom hverandre. Dette kan benyttes i for eksempel droner og andre nettverksstyrte mobile enheter. I full skala skal programmet gjøre det mulig å plassere enheter over større områder, der kommunikasjon mellom nabo-noder er nødvendig på grunn av avstanden til hovedserveren.  

### Implementert funksjonalitet

Beskrivelsen av implementerte funksjonaliteten er delt inn i to deler. Første beskrives implementert funksjonaliteten som er implementert i serveren, etterfulgt av neste seksjon som tar for seg funksjonaliteten implementert i noden. 

#### Server 

Serveren til applikasjonen har to hovedfunksjoner, den har mulighet for tilkobling av noder og organisering av noder.

Den første hovedfunksjonaliteten er nodens plassering ved tilkobling til serveren. Serveren bruker trådprogrammering for å håndtere innkommende forbindelser fra klientnoder. Når server mottar en forbindelse, tildeles en tråd for å håndtere denne. Tråden sjekker statusen på mesh-nettverket og bestemmer om noden skal plasseres i mesh-nettverket eller i en pool. Noden vil plasseres i mesh-nettverket dersom mesh-nettverket ikke har nådd sin definerte kapasitet. Hvis maksimal kapasitet er nådd blir nye noder plasser i en pool.  

Den andre hovedfunksjonaliteten er erstatning av noder i mesh-nettverket. Dette oppnås ved å kommunisere med serveren hvilken node som skal erstattes i nettverket. Serveren sender deretter en forespørsel til noden i mesh nettverket med høy prioritet. Masternoden i mesh nettverket identifiserer hvilken node i nettverket som skal erstatte noden som ønskes fjernet, og sender denne informasjonen til serveren. Serveren flytter erstatningsnoden til riktig lokasjonen og sette tilhørende prioritet. Noden som blir erstattet, blir nullstilt og plassert i poolen. 

#### Node

En node er en enhet i mesh-nettverket. Den fungerer både som en tjener for andre noder i nettverket, og som en klient enten i en annen node eller i hovedtjeneren.   

#### Hovedtråd
Hovedtrådens oppgave er først å initiere node-objektet og sette i gang servertråden. Siden ClientWorker og ServerWorker er avhengig av mye delt data, opprettes dette av hovedtråden slik at referanser og pekere kan benyttes i konstruksjonen av disse Worker klassene. Deretter venter den på at bruker-input og setter opp klienttråden med ønsket port nummer. Den har også ansvar for å stoppe applikasjonen dersom brukeren velger dette. 

#### ClientWorker
Noden har en dedikert tråd som kjører i ClientWorker klassen. Denne kobler seg opp mot enten hovedserveren eller en annen node i nettverket. Når tilkoblingen er etablert, lytter klientnoden etter instruksjoner fra servernoden og behandler dem. Hvis klienttråden mottar en instruksjon som ikke er til seg selv, legges instruksjonen til i en delt kø som ServerWorker kan behandle (se neste avsnitt). Dersom instruksjonen er ment for noden, så behandles den i HandleAction metoden. 

#### ServerWorker
Noden har en dedikert tråd som starter en tjener i ServerWorker klassen. I oppsetsfasen setter den opp tjeneren, og dedikerer en arbeidertråd som kjører i HandleInstructions. Deretter lytter tjener-tråden på nye tilkoblinger. Hvis den mottar en tilkobling, delegeres det en ny tråd for å legge til noden i nettverket. En node blir kun lagt til hvis den bekreftes som en gyldig node ved å sende en melding med action "HELLO" til servernode. Dermed blir noden lagt til i en liste og behandlings-tråden avsluttes. 

Servernoden kan håndtere oppgaver som blir gitt av ClientWorker tråden. Arbeidertråden bruker condition_variable til å vente på oppgaver i messageQueue køen, som er en delt kø mellom ClientWorker og ServerWorker. Når en oppgave mottas, sjekker node-serveren om oppgaven er til seg selv eller tilhører en annen klient i nettverket. Hvis oppgaven er til seg selv, behandler servernoden oppgaven. Dersom oppgaven er ment for en annen klient, vil servernoden kringkaste instruksjonen til alle oppkoblede klienter. 

#### Worker
Superklasse som de andre workerklassene arver av. Definerer delt data og enkelte metoder for å jobbe på disse. 

## Nåværende mangler og svakheter

Levert program er den første versjonen av programmet og har derav noen mangler og svakheter. Her er en liste over identifiserte mangler og svakheter:

- Simulert Bevegelse: Løsningen vår tar ikke høyde for å styre de mobile enhetene. Hver node har en verdi for sin plassering langs x-aksen for å simulere bevegelsen og kommunikasjonen mellom entitetene som avgjør denne posisjonen. Det benyttes også en metode for å simulere bevegelse som kaller på trådens sleep_for metode. 

- Mulighet for å plassere noder ut igjen: Vi har ikke noen mulighet for å gjenplassere noder som ligger i poolen. 

- Manglende kommunikasjon om behov for erstatning: En node har for øyeblikket ikke mulighet til å kommunisere når den trenger å bli erstattet. 

- Begrenset node plassering: Nåværende implementasjon av server plasserer noder i en dimensjon.

- Oppdatering av noder ved erstatning: Når en node erstattes i nettverket, vil serveren gå gjennom alle noder og oppdatere deres posisjon og prioritet. Dette kan bli en tidkrevende prosess dersom nettverket tillater et stort antall noder. 

- Manglende brukervennlig applikasjon: Det mangler et grafisk brukergrensesnitt for applikasjon som kan gjøre det mer brukervennlig.

- Utvidet format og mengde: Per nå er formatet på nettverket en-dimensjonelt med 5 posisjoner, der de sentrale har høyest prioritet. 

## Fremtidig arbeid

Ved fremtidig arbeid er det mulig å ta for seg følgende punkter for å imøtekomme nevnte mangler og svakheter:

- Mulighet for å plassere noder ut igjen: I denne versjonen blir noden automatisk plassert ut i nettverket når den kobles til. Deretter kan man fjerne den igjen om man ønsker og da legges den tilbake i poolen. Det er behov for en oversikt over nodene som er koblet til tjeneren men som ikke er plassert ut i nettverket. Her må brukeren kunne velge hvilken han ønsker å plassere ut igjen. 

- Selvinitiert kommunikasjon for noden: En mulig forbedring er å gi noden evnen til å kommunisere når den trenger å bli erstattet. For eksempel kan noden automatisk sende en forespørsel om å bli erstattet når batteristatusen faller under en bestemt prosentandel. Dette vil kreve en utvidelse av noden som tillater at klienten også kan sende meldinger mot en tjener.  

- Utvidelse til flere dimensjoner: For å skape en mer realistisk implementasjon. 

- Optimalisering av oppdateringsfunksjonalitet: For å gjøre programmet mer effektivt kan funksjonaliteten for oppdatering av posisjon og prioritet til noder forbedres. De nåværende kompleksiteten er O(n), det er iallfall mulig å redusere den til O(n/2) ved å utføre optimaliseringer.

- Utvikling av brukervennlig applikasjon: For å gjøre programmet mer brukervennlig, er det mulig å utvikle en tilhørende applikasjon for brukerne. 

- Begrenset node plassering: Legge til rette for at bruker velger hvordan nodene skal plasseres, og legge til rette for å bestemme hvor mange noder som skal ut i nettverket. 

## Kjøring og installasjon

**Merk: Dette programmet er designet for å kjøre i et Unix-terminalmiljø. Hvis du bruker Windows, må du sørge for at du har et Linux miljø installert før du fortsetter.**

1. Klone repositoriet fra GitLab eller pakk ut programmet fra zip-fil.
- [**Gitlab repo:**](https://gitlab.stud.idi.ntnu.no/hmasheim/mesh-network)

### Server

1. Kompiler serveren og tilhørende filer. Dersom du har g++ kompilatoren installert, kan du fra rotkatalogen bruke kommandoen: 
`g++ -o Server service/Server.cpp service/IpUtils.cpp model/DoubleLinkedList.cpp model/Node.cpp model/NodeList.cpp model/enums/ActionType.cpp -pthread`

2.	For å kjøre den kompilerte serveren: `./Server`

### Node
1. Kompiler serveren og tilhørende filer. Dersom du har g++ kompilatoren installert, kan du fra rotkatalogen bruke kommandoen:
`g++ -o Node node/Node.cpp node/Worker.cpp node/ClientWorker.cpp node/ServerWorker.c
pp model/enums/ActionType.cpp  -pthread`

2.	For å kjøre den kompilerte serveren: `./Node`

## Instruksjoner for å bruke løsningen

### Server 
Når serveren kjøres starter serveren automatisk opp. I terminalvinduet får man opp hvilken port serveren kjøres på. Dette portnummeret brukes i Noden for å koble seg til serveren. Deretter kan man skrive inn 'r' for å fjerne en node fra mesh-nettverket eller 'q' for å avslutte serveren 

### Node
Når noden kjøres representerer den en enhet i applikasjonen. Man kjører flere parallele noder for å simulere flere enheter i mesh-nettverket. Når man kjører en node kan man skrive inn 'q' for å avslutte noden og 'c' for å koble seg opp mot serveren. Dersom man velger 'c', blir man bedt om å skrive inn portnummeret til hovedserveren. Dette printes i terminalen når serveren kjøres.  

## Team medlemmer
- `Kristian Vaula Jensen`
- `Hans Magne Asheim`

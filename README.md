# SceneCapture – mobilt trådløst mesh-nettverk

## Introduksjon

SceneCapture er et mobilt trådløst mesh-nettverk designet for dronefilming av scener. Programmet muliggjør tilkobling av noder til en sentral server som styrer dronenes plassering og prioritet. Programmet tar også hensyn til muligheten for å erstatte droner i nettverket. Formålet med programmet er å muliggjøre filming av scener over store områder der kommunikasjon mellom nabo-noder er nødvendig på grunn av avstanden til serveren. Med dette programmet er det mulig å ha en avstand til serveren som overstiger den maksimale kommunikasjonsrekkevidden. 

## Implementert funksjonalitet

Beskrivelsen av implementerte funksjonaliteten er delt inn i to deler. Første beskrives implementert funksjonaliteten som er implementert i serveren, etterfulgt av neste seksjon som tar for seg funksjonaliteten implementert i noden. 

### Server - SceneCapture

Serveren til SceneCapture har to hovedfunksjoner, den har mulighet for tilkobling av noder og organisering av noder.

Den første hovedfunksjonaliteten er nodens plassering ved tilkobling til serveren. Serveren bruker trådprogrammering for å håndtere innkommende forbindelser fra klientnoder. Når server mottar en forbindelse, tildeles en tråd for å håndtere denne. Tråden sjekker statusen på mesh-nettverket og bestemmer om noden skal plasseres i mesh-nettverket eller i en pool. Noden vil plasseres i mesh-nettverket dersom mesh-nettverket ikke har nådd sin definerte kapasitet. Hvis maksimal kapasitet er nådd blir nye noder plasser i en pool.  

Den andre hovedfunksjonaliteten er erstatning av noder i mesh-nettverket. Dette oppnås ved å kommunisere med serveren hvilken node skal erstattes i nettverket. Serveren sender deretter en forespørsel til noden i mesh nettverket med høy prioritet. Masternoden i mesh nettverket identifiserer hvilken node i nettverket som skal erstatte noden som ønskes fjernet, og sende denne informasjonen til serveren. Serveren flytter erstatningsnoden til riktig lokasjonen og setter tilhørende prioritet. Noden som blir erstattet, blir nullstilt og plassert i poolen. 

### Node

En node kan fungere både som en server og som en klient, og vil ha ulik funksjonalitet for hver av disse rollene.  

#### Servernode

En servernode består av to hovedfunksjonaliteter, den kan legge til en node i nettverket og håndtere oppgaver fra både klientnoder og fra SceneCapture-serveren. 

Servernoden bruker trådprogrammering og har en dedikert tråd som venter på tilkoblinger fra noder. Hvis den mottar en tilkobling, delegeres det en ny tråd for å legge til noden i nettverket. En node blir kun lagt til hvis den bekreftes som en gyldig node ved å sende en melding med action "HELLO" til servernode. 

Servernoden kan også håndtere oppgaver fra både klienter og SceneCapture-serveren. Instruksjonstråden bruker tilstandsvariabler for å holde oversikt over tilgjengelige oppgaver. Når en oppgave mottas, sjekker node-serveren om oppgaven er til seg selv eller tilhører en annen klient i nettverket. Hvis oppgaven er til seg selv, behandler servernoden oppgaven ved mulighet. Mens hvis oppgaven tilhører en annen klient vil servernoden kringkaste instruksjonen til alle klienter. 

#### Klientnode

En klientnode har mulighet til å koble seg til servernoden. Når tilkoblingen er etablert, lytter klientnoden etter instruksjoner fra servernoden og behandle dem. Hvis klientnoden mottar en instruksjon som ikke er til seg selv, legges instruksjonen til i kø for instruksjoner i node-serveren. 

## Nåværende mangler og svakheter

Levert program er den første versjonen av SceneCapture programmet og har derav noen mangler og svakheter. Her er en liste over identifiserte mangler og svakheter av et ferdig program av SceneCapture og har derav mangler og svakheter:

- Manglende kommunikasjon om behov om erstatning: En node har for øyeblikket ikke mulighet til å kommunisere når den trenger å bli erstattet. 

- Begrenset node plassering: Nåværende implementasjon av server plasserer noder i en dimensjon. Dette begrenser muligheten for at droner kan dekke flere kameravinkler. 

- Oppdatering av noder ved erstatning: Når en node erstattes i nettverket, vil serveren gå gjennom alle noder og oppdatere deres posisjon og prioritet. Dette kan bli en tidkrevende prosess dersom nettverket tillater et stort antall noder. 

- Manglende brukervennlig applikasjon: Det mangler en dedikert applikasjon for programmet som kan gjøre det mer brukervennlig for brukere. 

## Fremtidig arbeid

Ved fremtidig arbeid er det mulig å ta for seg følgende punkter for å imøtekomme nevnte mangler og svakheter:

- Selvinitiert kommunikasjon for noden: En mulig forbedring er å gi noden evnen til å kommunisere når den trenger å bli erstattet. For eksempel kan noden automatisk sende en forespørsel om å bli erstattet når batteristatusen faller under en bestemt prosentandel.  

- Utvidelse til flere dimensjoner: For å dekke flere kameravinkler og optimalisere plasseringen av noder i scenen er det mulig å implementere funksjonalitet for plassering av noder i flere dimensjoner. 

- Optimalisering av oppdateringsfunksjonalitet: For å gjøre programmet mer effektivt kan funksjonaliteten for oppdatering av posisjon og prioritet til noder forbedres. De nåværende kompleksiteten er O(n), det er iallfall mulig å redusere den til O(n/2) ved å utføre optimaliseringer.

- Utvikling av brukervennlig applikasjon: For å gjøre programmet mer brukervennlig, er det mulig å utvikle en tilhørende applikasjon for brukerne. 

## Installasjonsinstruksjoner

For å kjøre programmet er det nødvendig å kjøre det i en Linux terminal. Deretter kan en følge trinnene nedenfor:

1. Klone repositoriet fra GitLab eller pakk ut programmet fra zip-filen.

### Server

1. Kompiler serveren og tilhørende filer. Hvis du står i rotkatalogen og har g++ kompilatoren installert, kan du bruke følgende kommando: `g++ -o Server service/Server.cpp service/IpUtils.cpp model/DoubleLinkedList.cpp model/Node.cpp model/NodeList.cpp model/enums/ActionType.cpp -pthread`

2.	Kjør serveren. Hvis du brukte kommandoen ovenfor for kompilering, kan du gjøre følgende: `./Server`

### Klient


## Instruksjoner for å bruke løsningen

## Team medlemmer
- `Kristian Vaula Jensen`
- `Hans Magne Asheim`

Dans le vaste domaine des protocoles Internet, le protocole de transfert hypertexte (HTTP) constitue l'épine dorsale de la communication pour le World Wide Web. Pour vraiment comprendre les subtilités de la communication web, 42 nous a présenté un projet formidable sur lequel nous embarquons, qui met au défi nos compétences et notre compréhension comme toujours… Oui, nous parlons du projet "Webserv", qui implique la création d'un serveur HTTP personnalisé en C++98, avec un accent particulier sur le comportement non bloquant.

**Aperçu du projet :**

— Le projet, dans son essence, consiste à construire un serveur en C++98. Pourquoi C++98 ? Je pense que 42 le fait exprès, pour que les étudiants aient une compréhension plus profonde de l'implémentation du C++ (et de son histoire).

— Il doit s'agir d'un serveur non bloquant. Construire un serveur web en C++ comporte divers défis, et un problème courant est lié aux opérations de blocage. Dans le contexte des serveurs web, le blocage fait référence à la situation où un thread attend qu'une opération se termine avant de pouvoir continuer. Cela peut entraîner des goulots d'étranglement en termes de performance, surtout dans les scénarios où le serveur doit gérer plusieurs connexions simultanées.

Le problème de blocage le plus significatif survient lors de la gestion des opérations d'entrée/sortie (I/O), telles que la lecture ou l'écriture sur des sockets. Dans un modèle de blocage traditionnel, un thread attendra que les données soient disponibles ou qu'une opération d'écriture soit terminée. Pendant cette période d'attente, le thread est essentiellement inactif, et le serveur peut avoir du mal à gérer d'autres requêtes entrantes.

Pour y remédier, nous utiliserons l'I/O asynchrone ; l'I/O asynchrone permet à un serveur d'initier une opération d'I/O et de continuer avec d'autres tâches en attendant que l'opération se termine. De cette façon, un seul thread peut gérer plusieurs connexions simultanément sans être bloqué.

L'I/O asynchrone peut être réalisée en utilisant divers appels système disponibles tels que `select`, `poll`, `epoll` (Linux)…

`poll` et `epoll` sont des alternatives plus simples/meilleures à `select`, mais `select` est plus portable (peut être utilisé sur différentes plateformes), alors parlons-en un peu :

`select` est un appel système dans les systèmes d'exploitation de type Unix qui facilite la surveillance asynchrone de plusieurs descripteurs de fichier pour des événements spécifiques, tels que la lecture, l'écriture, ou les erreurs. Il permet à un programme d'attendre efficacement et de gérer les événements sur un ensemble de descripteurs. Voici un aperçu de son fonctionnement :

— Initialisation de `fd_set` : Nous initialisons des structures `fd_set` séparées pour chaque type d'événement (lecture ou écriture...) que nous souhaitons surveiller. Ces structures servent de masques de bits représentant des ensembles de descripteurs de fichiers (afin que nous puissions savoir quels FD sont prêts pour la lecture/écriture, bit 1 pour prêt et 0 pour non).

— Peuplement du `fd_set` : Les descripteurs de fichiers d'intérêt sont ajoutés au `fd_set` respectif en utilisant des macros comme `FD_SET`.

— Réglage du délai d'attente : Nous pouvons définir une valeur de délai d'attente, pour déterminer la durée maximale que `select` doit attendre un événement. Si le délai expire sans aucun événement, `select` retourne, permettant au programme de continuer.

— Appel de `select` : Le programme invoque la fonction `select`, fournissant la "valeur de descripteur de fichier la plus élevée" + 1 (man select), les trois `fd_sets`, et le délai d'attente ; Nous pouvons définir NULL comme valeur pour les événements qui ne nous intéressent pas, ou pour le délai d'attente si nous souhaitons que `select` bloque indéfiniment jusqu'à ce qu'un événement se produise.

— Blocage ou retour : `select` entre dans un état de blocage, en attendant des événements sur les descripteurs de fichier surveillés ou jusqu'à l'expiration du délai. Lorsqu'un événement ou un délai se produit, `select` rend le contrôle au programme.

— Vérification du `fd_set` : Après le retour, le programme examine les `fd_sets` pour identifier quels descripteurs de fichier sont prêts pour les événements spécifiés, en utilisant des macros comme `FD_ISSET`.

— Gestion des événements : Nous pouvons répondre aux événements en exécutant les actions requises en fonction des descripteurs de fichiers identifiés. Par exemple, nous pouvons lire/écrire des données depuis/vers un socket ou gérer des erreurs.

— Bouclage ou sortie : Nous employons souvent une boucle pour répéter le processus de surveillance, permettant la gestion asynchrone continue des événements sur plusieurs descripteurs de fichier.

Bien que `select` soit connu pour sa portabilité sur différentes plateformes, il présente des limitations, comme un nombre maximum de descripteurs de fichier qu'il peut gérer. C'est pourquoi des alternatives modernes comme `poll` et `epoll` sont recommandées à la place, comme le manuel l'indique :
![[Pasted image 20240501195727.png]]

— Mise en œuvre de HTTP/1.1.

Après avoir configuré le serveur pour les opérations de base de lecture et d'écriture, nous pouvons maintenant passer à l'analyse de ce que nous échangeons avec les clients (c'est-à-dire les messages).

Une communication HTTP typique consiste à comprendre les requêtes et à générer des réponses appropriées, et pour ce faire, il est crucial de comprendre la structure des messages HTTP.

Je vais légèrement aborder HTTP/1.1 ici, mais si vous recherchez plus de profondeur, je vous conseille de lire les RFC officiels, Il est essentiel de se concentrer sur les versions les plus récentes — j'ai investi beaucoup de temps à lire rfc2616, seulement pour réaliser qu'il est obsolète depuis très longtemps ; au moment de la rédaction de cet article, les actuels sont rfc9110 et rfc9112.

Les messages HTTP se composent d'une ligne de requête ou de réponse, d'en-têtes, d'une ligne vide (CRLF ou \r\n), et d'un corps de message facultatif.

Voici une illustration utilisant la forme Backus-Naur augmentée :

```
HTTP-message   = start-line CRLF  
                   *( field-line CRLF )  
                   CRLF  
                   [ message-body ]  
  
start-line     = request-line / status-line
```

1. Ligne de requête :

```
request-line = method SP request-target SP HTTP-version
```

La ligne de requête dans un message de requête HTTP contient la méthode, l'URI, et la version HTTP. Analyser cette ligne est la première étape pour comprendre la demande du client. Par exemple :

```
GET /path/to/resource HTTP/1.1
```

Dans l'exemple ci-dessus, "GET" est la méthode, "/path/to/resource" est l'URI, et "HTTP/1.1" est la version.

2. Ligne de statut / Ligne de réponse

```
status-line = HTTP-version SP status-code SP [ reason-phrase ]
```

De même, dans un message de réponse HTTP, la ligne de statut/réponse contient la version HTTP, le code de statut, et une phrase de raison. Extraire ces informations est crucial pour générer des réponses significatives. Par exemple :

HTTP/1.1 200 OK

```
Ici, "HTTP/1.1" est la version, "200" est le code de statut, et "OK" est la phrase de raison (ou la description du code de statut).
```

3. En-têtes :

Les requêtes et les réponses peuvent inclure des en-têtes, fournissant des informations supplémentaires sur le message. L'analyse des en-têtes implique l'extraction de paires clé-valeur pour comprendre divers aspects tels que le type de contenu, la longueur du contenu, et plus. Par exemple :

```
Host: example.com 
Content-Type: text/html 
Content-Length: 256
```
L'analyse de ces en-têtes permet au serveur d'interpréter le contenu de manière appropriée.

4. Corps du message :

Le corps du message, bien que facultatif, peut contenir des données pertinentes pour la requête ou la réponse. L'analyse du corps du message dépend de facteurs tels que le type de contenu et la longueur, le codage de transfert, etc...

Après avoir terminé l'analyse des messages HTTP, les étapes suivantes impliquent la mise en œuvre de la logique pour gérer différentes méthodes HTTP, traiter les demandes, et générer des réponses appropriées.

Voici un exemple de diagramme de flux que j'ai créé, pour avoir un aperçu du processus de réponse : Note : Il peut ne pas être exhaustif ou impeccable ; son but est de fournir un aperçu simple plutôt que d'englober tous les détails.

![[Pasted image 20240501200203.png]]
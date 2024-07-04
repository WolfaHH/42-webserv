
# Eval
Commençons par suivre les directives et répondre à chaque étape une par une. Nous examinerons chaque point du projet et je vous expliquerai ce qu'il faut faire et pourquoi, pour vous aider à évaluer le projet de vos pairs en conformité avec les règles fournies.

### Étape 1: Introduction

#### Règles à respecter

- Soyez poli, courtois, respectueux et constructif tout au long du processus d'évaluation.
- Identifiez les dysfonctionnements possibles dans le projet évalué, discutez et débattez des problèmes identifiés.
- Gardez à l'esprit les différences potentielles d'interprétation des instructions du projet.
- Évaluez honnêtement en tenant compte de ces éléments.

### Étape 2: Directives pour l'évaluation

- Évaluez uniquement le travail rendu dans le dépôt Git de l'étudiant ou du groupe.
- Vérifiez que le dépôt Git appartient bien aux étudiants évalués et que le projet est conforme aux attentes.
- Assurez-vous qu'aucun alias malveillant n'a été utilisé pour fausser votre évaluation.
- Si vous n'avez pas complété l'assignation, lisez entièrement le sujet avant de commencer l'évaluation.
- Utilisez les drapeaux appropriés pour signaler les problèmes tels qu'un dépôt vide, un programme non fonctionnel, une erreur de norme, etc.
- Rappelez-vous que tout crash du programme ou terminaison inattendue résulte en une note de 0, sauf en cas de tricherie où la note est de -42.

### Étape 3: Partie obligatoire

#### Vérifications à effectuer

- Demandez au groupe d'expliquer les bases d'un serveur HTTP et comment ils ont utilisé `select()` ou un équivalent pour la multiplexation des I/O.
- Vérifiez que `select()` est utilisé correctement pour gérer à la fois les opérations de lecture et d'écriture. Si ce n'est pas le cas, la note est de 0.
- Vérifiez que toutes les lectures et écritures sur les sockets gèrent correctement les valeurs de retour et les erreurs sans vérifier `errno` après les appels, sinon la note est de 0.
- Assurez-vous que le projet compile sans erreurs et qu'il respecte la norme C++ 98.
- Faites des tests de configuration, comme configurer plusieurs serveurs sur différents ports ou hôtes, et vérifier les pages d'erreur par défaut.

### Étape 4: Tests de base et CGI

#### Tests

- Utilisez `telnet`, `curl` et les fichiers préparés pour tester les requêtes GET, POST et DELETE.
- Assurez-vous que le CGI fonctionne correctement, notamment en exécutant des scripts avec des erreurs pour tester la gestion des erreurs.

### Étape 5: Test avec un navigateur et gestion des ports

- Vérifiez que le serveur est bien configuré pour gérer plusieurs ports et qu'il fonctionne correctement avec les configurations spécifiées.

### Étape 6: Siege & test de stress

- Exécutez des tests de stress avec Siege pour vérifier la stabilité et la gestion de la mémoire du serveur.

Chaque étape de cette évaluation nécessite une attention particulière aux détails et une compréhension des exigences techniques du projet. Si vous avez des questions spécifiques sur une partie du projet ou besoin d'explications sur un concept, n'hésitez pas à demander.

# general
Le projet "Webserv" que vous décrivez est un exercice avancé de programmation C++98 qui consiste à développer un serveur HTTP non bloquant. Voici une explication détaillée des différents aspects du projet, basée sur le tutoriel que vous avez trouvé ainsi que sur le document du projet que vous avez fourni.

### Compréhension de Base du HTTP

Le protocole HTTP est la pierre angulaire de la communication sur le Web, permettant l'échange de documents hypertextes. Un serveur HTTP typique reçoit des requêtes de clients (navigateurs, robots, etc.), les traite et renvoie des réponses adéquates, souvent sous forme de pages web contenant du texte, des images et d'autres types de données.

### Spécificités de l'Implémentation en C++98

L'accent est mis sur l'utilisation de C++98 pour des raisons pédagogiques, afin de bien comprendre les fondamentaux du C++ et les pratiques de codage qui étaient courantes à l'époque. Vous devez respecter strictement cette norme et utiliser les fonctionnalités de C++ où cela est possible, évitant les bibliothèques modernes ou les extensions plus récentes.

### Serveur Non Bloquant

Le serveur doit être non bloquant, ce qui signifie qu'il doit être capable de gérer simultanément plusieurs connexions sans qu'une opération d'entrée/sortie (I/O) sur une connexion ne bloque les autres. Cela est généralement réalisé en utilisant `select`, `poll`, ou `epoll` pour surveiller les événements sur les sockets sans se bloquer en attendant que ces événements se produisent.

#### Système d'Asynchronisme avec `select`

- **Initialisation :** Préparation des ensembles de descripteurs de fichiers (`fd_set`) pour les opérations de lecture, d'écriture et les erreurs.
- **Utilisation :** Appel à `select()` qui surveille ces ensembles pour des changements d'état, permettant de gérer les requêtes entrantes et les réponses sortantes de manière asynchrone.

### Gestion des Requêtes HTTP et Réponses

- **Parsing des Requêtes :** Analyser la ligne de requête (ex: `GET /index.html HTTP/1.1`), les headers et le corps de la requête pour déterminer l'action à effectuer.
- **Construction des Réponses :** Créer une réponse appropriée basée sur la requête, qui peut inclure des headers et un corps de réponse, et renvoyer cela au client.

### Configuration et Utilisation

Votre serveur doit être configurable via un fichier de configuration similaire à ceux utilisés par NGINX. Ce fichier doit permettre de définir les ports et les routes, ainsi que les comportements spécifiques comme les redirections ou le traitement des CGI.

### Considérations Techniques

- **Sécurité et Robustesse :** Le serveur doit gérer les erreurs élégamment et ne jamais crasher.
- **Compatibilité :** Le serveur doit être conforme à HTTP/1.1, et son comportement peut être validé en le comparant à NGINX.

### Conclusion

Le projet est complexe et exige une bonne compréhension des réseaux, de la programmation asynchrone, et de la norme C++98. Il teste non seulement vos compétences en programmation mais aussi votre capacité à travailler avec des protocoles et des normes bien établies. En respectant les directives fournies, vous développerez un serveur capable de gérer efficacement les communications Web en mode non bloquant.
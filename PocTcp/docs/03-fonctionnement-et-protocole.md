# Fonctionnement et protocole

## Établissement de la session

Le contrôleur initialise Winsock, crée une socket TCP, l’associe à `192.168.56.1:5000`, puis attend une connexion avec `listen` et `accept`.

L’implant initialise Winsock, crée une socket TCP et appelle `connect` vers la même adresse. Si la connexion réussit, les deux programmes entrent dans leur boucle de communication.

## Cycle d’une commande

1. Le contrôleur lit une ligne dans le terminal.
2. Le premier espace sépare le nom de la commande de son argument.
3. Le contrôleur envoie la commande structurée.
4. L’implant reçoit les champs de la commande.
5. L’implant vérifie le nom dans sa liste de commandes autorisées.
6. L’action est exécutée localement.
7. Le résultat ou l’erreur est envoyé au contrôleur.
8. Le contrôleur affiche le contenu et attend une nouvelle commande.

## Format d’une commande

Une commande est transmise selon la structure suivante :

```text
┌─────────────────────┬──────────────┬─────────────────────┐
│ Taille du nom       │ Nom          │ Taille de l'argument│
│ 4 octets            │ N octets     │ 4 octets            │
└─────────────────────┴──────────────┴─────────────────────┘
┌─────────────────────┐
│ Argument            │
│ M octets            │
└─────────────────────┘
```

Le récepteur lit d’abord la taille, puis le nombre exact d’octets indiqué. Les fonctions `sendAll` et `receiveAll` sont nécessaires car TCP est un flux d’octets et ne conserve pas les limites des appels à `send` et `recv`.

Les tailles reçues sont limitées à `MAX_MESSAGE_SIZE`, fixé à 1 MiB.

## Format d’une réponse

Les réponses utilisent un en-tête commun :

| Champ | Taille | Description |
|---|---:|---|
| `version` | 1 octet | Version du format, actuellement `1` |
| `type` | 1 octet | `Command`, `Response` ou `Error` |
| `length` | 4 octets | Taille du contenu |

Le contenu suit immédiatement l’en-tête.

```text
┌─────────┬────────┬───────────────┬─────────────────────┐
│ Version │ Type   │ Longueur      │ Contenu             │
│ 1 octet │ 1 octet│ 4 octets      │ N octets            │
└─────────┴────────┴───────────────┴─────────────────────┘
```

Le type `Response` indique une exécution réussie. Le type `Error` indique une erreur de commande ou d’exécution.

Les entiers sont actuellement transmis dans leur représentation binaire locale. Le format n’utilise pas de conversion réseau standard, ce qui limite son interopérabilité.

## Commandes disponibles

| Commande | Fonctionnement |
|---|---|
| `dir` | Exécute `dir` et retourne la sortie |
| `whoami` | Retourne l’utilisateur Windows courant |
| `ipconfig` | Retourne la configuration réseau |
| `pwd` | Retourne le répertoire courant |
| `cd <chemin>` | Change le répertoire courant |
| `exit` | Ferme la session depuis le contrôleur |

Les commandes `dir`, `whoami` et `ipconfig` utilisent `_popen` pour récupérer leur sortie standard. `pwd` et `cd` utilisent les API Windows `GetCurrentDirectoryA` et `SetCurrentDirectoryA`.

Une commande inconnue produit une erreur et n’est pas exécutée.

## Sécurité du protocole actuel

TCP fournit uniquement un transport fiable des octets. Le protocole applicatif personnalisé de PocTcp ajoute la structure des messages, mais ne fournit pas actuellement :

* d’authentification du contrôleur ou de l’implant ;
* de chiffrement ;
* de vérification cryptographique de l’intégrité ;
* de protection contre la relecture ;
* d’échange ou de gestion de clés.

Une personne capable d’observer le réseau peut donc lire les commandes et les résultats. Une personne capable d’injecter du trafic peut également tenter d’interagir avec la session.

Cette limite ne signifie pas qu’aucun protocole ne peut offrir ces garanties. Des protocoles ou couches comme TLS et SSH les fournissent selon leur configuration. Le problème concerne spécifiquement le format personnalisé actuellement utilisé par le prototype.

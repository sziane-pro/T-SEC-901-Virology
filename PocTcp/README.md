# PocTcp

La documentation complète du prototype, de ses décisions d’architecture et de sa transition vers la solution finale est disponible dans [`DOCUMENTATION.md`](./DOCUMENTATION.md).

## Présentation

PocTcp est une preuve de concept de communication TCP entre un contrôleur et un implant Windows. Le contrôleur permet de saisir des commandes dans un terminal. L’implant reçoit ces commandes, les exécute localement lorsqu’elles sont autorisées, puis renvoie le résultat au contrôleur.

Le projet est développé en C++17 et utilise l’API Winsock pour les communications réseau. Il est destiné à être utilisé dans un laboratoire isolé et contrôlé.

## Architecture

Le projet est composé de trois parties principales.

| Élément | Rôle |
|---|---|
| `controller` | Serveur TCP qui attend la connexion de l’implant et permet à l’utilisateur d’envoyer des commandes |
| `implant` | Client TCP qui se connecte au contrôleur et traite les commandes reçues |
| `common/protocol` | Définition du protocole applicatif partagé entre les deux programmes |

Le contrôleur écoute sur l’adresse `192.168.56.1` et le port TCP `5000`. Cette adresse correspond généralement à une interface réseau privée utilisée par une machine virtuelle.

## Déroulement d’une communication

Le fonctionnement général est le suivant :

1. Le contrôleur initialise Winsock.
2. Il crée une socket TCP, l’associe à `192.168.56.1:5000`, puis attend une connexion.
3. L’implant initialise Winsock et tente de se connecter à cette même adresse.
4. Une fois la connexion établie, le contrôleur lit une commande saisie par l’utilisateur.
5. La commande est séparée en un nom et un argument éventuel.
6. Le contrôleur transmet la commande à l’implant.
7. L’implant vérifie le nom de la commande et exécute uniquement les commandes prévues par le code.
8. L’implant renvoie le résultat avec un type indiquant une réponse normale ou une erreur.
9. Le contrôleur affiche le résultat et attend la commande suivante.

La connexion est maintenue pendant toute la session. Le contrôleur accepte actuellement un seul client.

## Protocole applicatif

Le protocole est défini dans `common/protocol`. Les fonctions `sendAll` et `receiveAll` garantissent que tous les octets attendus sont envoyés ou reçus malgré le fonctionnement fragmenté des sockets TCP.

Les réponses utilisent un en-tête composé des champs suivants :

| Champ | Taille | Description |
|---|---:|---|
| `version` | 1 octet | Version du protocole, actuellement `1` |
| `type` | 1 octet | Type du message |
| `length` | 4 octets | Taille du contenu en octets |

Les types de message disponibles sont :

| Type | Valeur | Utilisation |
|---|---:|---|
| `Command` | 1 | Commande |
| `Response` | 2 | Résultat d’une commande réussie |
| `Error` | 3 | Erreur de traitement ou d’exécution |

Une commande est transmise avec la taille de son nom, le nom, la taille de son argument, puis l’argument. La taille maximale acceptée pour un contenu est de 1 MiB.

Le protocole utilise actuellement les représentations binaires locales des entiers. Il n’effectue pas de conversion d’endianness et ne chiffre pas les échanges.

## Commandes disponibles

Les commandes sont traitées dans `implant/src/commands.cpp`.

| Commande | Fonction |
|---|---|
| `dir` | Affiche le contenu du répertoire courant |
| `whoami` | Affiche l’utilisateur Windows courant |
| `ipconfig` | Affiche la configuration réseau |
| `pwd` | Affiche le répertoire courant |
| `cd <chemin>` | Change le répertoire courant |
| `exit` | Ferme la session depuis le contrôleur |

La commande `cd` nécessite un chemin en argument. Les arguments des autres commandes ne sont pas utilisés dans la version actuelle.

Les commandes `dir`, `whoami` et `ipconfig` sont exécutées avec `_popen`, puis leur sortie standard est récupérée et renvoyée au contrôleur. Une commande inconnue produit un message d’erreur sans être exécutée.

## Organisation des fichiers

```text
PocTcp
├── CMakeLists.txt
├── build.bat
├── common
│   └── protocol
│       ├── protocol.hpp
│       └── protocol.cpp
├── controller
│   ├── CMakeLists.txt
│   ├── include
│   │   └── network.hpp
│   └── src
│       ├── main.cpp
│       └── network.cpp
└── implant
    ├── CMakeLists.txt
    ├── include
    │   ├── commands.hpp
    │   └── network.hpp
    └── src
        ├── commands.cpp
        ├── main.cpp
        └── network.cpp
```

Les répertoires `build` contiennent les fichiers générés par CMake et les exécutables compilés. Ils ne font pas partie du code source nécessaire au fonctionnement du projet.

## Compilation

La compilation nécessite :

1. Windows
2. CMake 3.20 ou une version plus récente
3. Un compilateur C++ compatible avec C++17
4. La bibliothèque système Winsock

Depuis PowerShell, se placer dans le dossier du projet :

```powershell
cd "C:\Users\Jaures Acs\Documents\delivery\T-SEC-901-Virology\PocTcp"
```

Lancer ensuite :

```powershell
.\build.bat
```

Le script configure le projet avec CMake, compile la configuration Release et vérifie la présence des deux exécutables :

```text
build\controller\Release\controller.exe
build\implant\Release\implant.exe
```

Le script démarre ensuite le contrôleur. L’implant doit être lancé séparément depuis la machine ou l’environnement prévu pour le laboratoire.

## Utilisation dans le laboratoire

Le contrôleur doit être lancé en premier afin qu’il soit déjà en écoute sur le port `5000`. L’implant peut ensuite être démarré sur la machine virtuelle configurée pour joindre l’adresse `192.168.56.1`.

Lorsque la connexion est établie, le contrôleur affiche une invite de commande. L’utilisateur peut saisir une commande disponible, par exemple :

```text
whoami
pwd
cd C:\Temp
dir
```

Pour terminer la session depuis le contrôleur, saisir :

```text
exit
```

Le réseau de la machine virtuelle doit rester isolé et réservé aux essais du laboratoire.

## Limites actuelles

Cette version est une preuve de concept et ne fournit pas les mécanismes attendus pour un logiciel déployé en production.

Les échanges ne sont pas chiffrés et aucune authentification n’est effectuée. L’adresse IP et le port sont codés en dur. Le contrôleur ne gère qu’un seul implant à la fois et ne dispose pas de mécanisme de reconnexion.

Le protocole ne convertit pas les entiers dans un format réseau standard. Le contrôleur et l’implant doivent donc utiliser une représentation compatible.

Les erreurs de connexion interrompent la session en cours. La validation des entrées reste limitée et la gestion des sorties volumineuses ou des chemins dépassant `MAX_PATH` n’est pas approfondie.

## Évolutions possibles

Les améliorations envisageables sont l’ajout d’une authentification mutuelle, d’un chiffrement TLS, d’une configuration externe de l’adresse et du port, d’une gestion de plusieurs clients et d’un format de protocole indépendant de l’architecture.

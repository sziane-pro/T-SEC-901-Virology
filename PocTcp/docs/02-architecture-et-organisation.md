# Architecture et organisation du code

## Architecture générale

```text
Machine hôte
┌──────────────────────────┐
│ Controller               │
│ Serveur TCP              │
│ 192.168.56.1:5000       │
└────────────┬─────────────┘
             │ Réseau privé VirtualBox
             │
Machine virtuelle Windows
┌────────────▼─────────────┐
│ Implant                  │
│ Client TCP               │
└──────────────────────────┘
```

Le contrôleur est lancé sur la machine hôte. Il écoute les connexions et fournit l’interface de saisie. L’implant est lancé dans la machine virtuelle et se connecte au contrôleur.

La version actuelle accepte un seul implant. La connexion est maintenue jusqu’à la fermeture de la session ou à une erreur réseau.

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

Les répertoires `build` contiennent les fichiers générés par CMake. Ils ne sont pas nécessaires pour comprendre le code source.

## Responsabilités des composants

### Contrôleur

`controller/src/network.cpp` crée la socket IPv4 TCP, configure l’adresse locale, réalise le `bind`, active l’écoute et accepte un client.

`controller/src/main.cpp` initialise Winsock, lit les commandes dans le terminal, les transmet à l’implant et affiche les réponses.

### Implant

`implant/src/network.cpp` crée la socket et se connecte à l’adresse du contrôleur.

`implant/src/main.cpp` maintient la boucle de réception et de réponse.

`implant/src/commands.cpp` contient la liste blanche des commandes autorisées et leur exécution locale.

### Protocole commun

`common/protocol/protocol.hpp` définit les structures et les signatures utilisées par les deux composants.

`common/protocol/protocol.cpp` contient les fonctions d’envoi, de réception et de reconstruction des messages. Ce partage évite que le contrôleur et l’implant utilisent deux formats différents.

## Compilation

Le fichier CMake racine utilise C++17 et inclut les cibles `controller` et `implant`. Les deux exécutables utilisent Winsock via `ws2_32`.

Le script `build.bat` configure le projet, compile la configuration Release et vérifie la présence des fichiers suivants :

```text
build\controller\Release\controller.exe
build\implant\Release\implant.exe
```

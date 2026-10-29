# Architecture du contrôleur  s0p0wn3d

## 1. Présentation du projet

Ce document décrit l'architecture logicielle envisagée pour le contrôleur Python du projet `s0p0wn3d`.

L'objectif de cette phase est de définir une structure modulaire, de séparer les responsabilités des composants et de faciliter les tests ainsi que l'évolution du code.

## 2. Choix techniques

| Élément | Choix retenu |
|---|---|
| Langage du contrôleur | Python |
| Transport HTTP | HTTPS prévu |
| Format des messages | JSON |
| Organisation | Architecture modulaire |
| Modèles de données | Identité du client et état de communication séparés |
| Tests | Tests automatisés des modèles, du protocole et du registre |
| Journalisation | Module dédié |
| Authentification | Module dédié, conception à préciser |

Le contrôleur est développé comme un programme Python modulaire. La création d'un package installable n'est pas une priorité à ce stade.

## 3. Architecture des fichiers

```text
s0p0wn3d/
├── controller/
│   ├── __init__.py
│   ├── main.py
│   ├── network.py
│   ├── protocol.py
│   ├── auth.py
│   ├── commands.py
│   ├── command_catalog.py
│   ├── clients.py
│   ├── audit.py
│   └── models/
│       ├── __init__.py
│       ├── client_identity.py
│       ├── communication_status.py
│       ├── commmand.py
│       └── command_result.py
├── tests/
│   ├── test_protocol.py
│   ├── test_clients.py
│   └── test_models.py
├── config/
│   └── config.json
├── .gitignore
├── requirements.txt
└── README.md
```

Cette arborescence est une proposition de travail issue des décisions de conception. Aucun fichier n'a pour le moment été implémentés.

## 4. Responsabilités des modules

### `main.py`

Point d'entrée du programme.

Responsabilités prévues :
- Charger la configuration nécessaire.
- Initialiser les composants du service.
- Coordonner le démarrage et l'arrêt du programme.

### `network.py`

Couche de communication HTTP(S).

Responsabilités prévues :
- Gérer les échanges HTTP.
- Traiter les erreurs liées au transport.
- Transmettre les messages reçus à la couche protocolaire.

Cette couche reste distincte de la logique métier.

### `protocol.py`

Définition du protocole applicatif JSON.

Responsabilités prévues :
- Définir la structure des messages.
- Vérifier leur format et les champs obligatoires.
- Distinguer un message valide d'un message mal formé.

Le protocole applicatif ne doit pas dépendre directement du transport utilisé.

### `auth.py`

Module dédié aux responsabilités d'authentification.

Méthode d'authentification proposé pour le moment : "Pre-Shared Key" (Clé pré-partagée) + Agent ID

C'est la méthode la plus simple dans le cadre d'un MVP. 
L'agent génère une clé secrète (un long token aléatoire) . 
À chaque requête, l'agent envoie cette clé dans un header HTTP (ex: X-Agent-Token). 
Le serveur vérifie que la clé correspond.

### `commands.py`

Responsabilités prévues :
- Représenter une demande : identifiant, type d'action, date de création...
- Gérer son état : en attente, en cours, terminée ou en erreur.
- Associer un résultat à la demande initiale.
- Journaliser les événements pour conserver une trace des traitements.

### `command_catalog.py`

Responsabilité prévues :
- Définir les actions prédéfinies et leurs paramètres

### `clients.py`

Gestion du registre des clients simulés.

Responsabilités prévues :
- Conserver les enregistrements des clients.
- Consulter un enregistrement existant.
- Mettre à jour les métadonnées autorisées. 
- Associer un enregistrement à son état de communication.

Un client inactif n'est pas automatiquement supprimé du registre.

### `audit.py`

Journalisation des événements du service.

Responsabilités prévues :
- Enregistrer les événements importants.
- Faciliter le diagnostic des erreurs.
- Distinguer les événements techniques des résultats fonctionnels.

Les journaux doivent rester fiables et permettre de comprendre les incidents.

## 5. Modèles de données

Les informations relatives à un client sont séparées de celles concernant ses communications.

### 5.1 `ClientIdentity`

Représente les informations conservées pour un client enregistré.

| Champ | Description |
|---|---|
| `client_id` | Identifiant du client |
| `registered_at` | Date et heure du premier enregistrement |
| `ip_private` | Adresse IPv4 privée connue de l'agent |

L'adresse IP privée constitue une métadonnée réseau. Elle peut évoluer lorsqu'une machine change de réseau ou reçoit une nouvelle adresse.

### 5.2 `CommunicationStatus`

Représente l'état des communications d'un client.

| Champ | Description |
|---|---|
| `last_seen` | Date et heure de la dernière communication valide |
| `last_result` | Résultat de la dernière communication traitée |
| `consecutive_errors` | Nombre d'erreurs consécutives observées |

Les valeurs autorisées de `last_result` restent à définir. Une première convention pourrait distinguer `success`, `error` et `unknown`.

Une tentative de connexion échouée ne doit pas être assimilée à une communication valide.

### 5.3 `Command`

Représente les informations d'une commande

| Champ | Description |
|---|---|
| `command_id` | Identifiant unique de la demande |
| `client_id` | Client concerné |
| `action_type` | Type d'action demandée |
| `status` | État de la demande |
| `created_at` | Date de création |
| `send_at` | Date d'envoi, si l'envoi a eu lieu |

### 5.4 `CommandResult`

Représente les informations du résultat d'une commande

| Champ | Description |
|---|---|
| `command_id` | Identifiant unique de la demande |
| `result` | Résultat de la commande |
| `completed_at` | Date de fin |

### 5.5 Séparation des responsabilités

`ClientIdentity` décrit les informations relativement stables du client.

`CommunicationStatus` décrit son état de communication, susceptible d'évoluer au cours du temps.

`Command` décrit les informations d'une commande

`CommandResult` décrit les informations retourné par la commande

Cette séparation facilite les tests et évite de mélanger l'identité enregistrée avec l'état courant du service.

## 6. Format des échanges

Le format applicatif retenu est JSON.

Les règles précises restent à définir, notamment :
- La structure commune des messages.
- Les champs obligatoires et facultatifs.
- Les types de données attendus.
- La gestion des champs inconnus.
- Les réponses d'erreur et leurs codes.
- La validation des données reçues.

Les messages devront être validés avant d'être transmis à la logique métier.

## 7. Gestion des erreurs et des communications

Le modèle de communication prévoit de conserver :
- La date de la dernière communication valide.
- Le résultat du dernier traitement.
- Le nombre d'erreurs consécutives.

Les règles exactes de mise à jour de ces champs restent à spécifier. Elles devront notamment distinguer les erreurs réseau, les messages invalides et les erreurs de traitement.

Les enregistrements de clients ne sont pas supprimés automatiquement au seul motif qu'ils sont inactifs.

## 8. Configuration

Le fichier `config/config.json` est prévu pour les paramètres de configuration non sensibles.

La structure exacte du fichier reste à définir.

Les secrets et les clés privées ne doivent pas être ajoutés au dépôt Git. Les valeurs sensibles doivent être fournies par un mécanisme de configuration approprié à l'environnement de test.

## 9. Tests

Le dossier `tests/` regroupe les tests automatisés.

### `test_protocol.py`

Vérification du format des messages, des champs obligatoires et des entrées invalides.

### `test_clients.py`

Vérification des opérations du registre : création, consultation et mise à jour des enregistrements simulés.

### `test_models.py`

Vérification de la validité des modèles de données et de leurs valeurs.

Les tests doivent couvrir les cas normaux, les données invalides et les erreurs attendues, sans dépendre d'une infrastructure externe lorsque cela n'est pas nécessaire.

## 10. État actuel de la conception

### Décisions prises

- Le contrôleur est écrit en Python.
- Le programme suit une architecture modulaire.
- HTTPS est le transport envisagé.
- JSON est le format des messages.
- Un module dédié gère le protocole.
- Un module dédié est prévu pour l'authentification.
- Le registre des clients est distinct de l'état de communication.
- L'adresse IPv4 privée est une métadonnée du client.
- Les clients inactifs ne sont pas supprimés automatiquement.
- Les tests et la journalisation disposent de modules dédiés.

### Points restant à définir

- Le schéma précis des messages JSON.
- La structure du fichier de configuration.
- Les règles de validation du protocole.
- Les valeurs autorisées pour `last_result`.
- Les règles de mise à jour de `consecutive_errors`.
- Le format des journaux et les niveaux de journalisation.
- La stratégie de stockage du registre.
- Les dépendances Python et leur version.
- Les tests d'intégration du service HTTP(S).

## 11. Principe de conception

L'architecture privilégie la séparation des responsabilités, la simplicité et la testabilité.

Chaque module doit avoir un rôle clairement défini. Les interfaces entre les modules doivent être suffisamment simples pour pouvoir tester indépendamment le protocole, les modèles et le registre client.

Les décisions non encore validées sont conservées comme points ouverts plutôt que présentées comme des fonctionnalités déjà implémentées.

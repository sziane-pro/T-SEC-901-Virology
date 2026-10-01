# Protocoles de communication étudiés pour s0P0wn3d

## 1. Objectif

Ce document présente les protocoles mentionnés dans le sujet du projet s0P0wn3d et explique leur fonctionnement général ainsi que les concepts techniques utiles à leur étude.

Le sujet demande de créer un tunnel entre le contrôleur et la machine cible en utilisant un protocole existant. Il cite notamment HTTP, DNS, IRC, SSH, FTP et RDP, tout en laissant la possibilité d’étudier des protocoles similaires. fileciteturn2file0

Ce document présente chaque protocole séparément. Il ne constitue pas une comparaison ni une recommandation de choix.

## 2. TCP

### Présentation

TCP, Transmission Control Protocol, est un protocole de transport orienté connexion.

Il fournit notamment :

- une communication entre deux extrémités ;
- une transmission ordonnée des octets ;
- la retransmission de données perdues ;
- un contrôle de flux ;
- une gestion de l’établissement et de la fermeture de connexion.

Dans une architecture simple :

```text
Application
    |
   TCP
    |
   IP
```

### Point important : TCP est un flux d’octets

TCP ne connaît pas la notion de message applicatif.

Si une application effectue :

```text
send("dir")
```

TCP transporte les octets correspondant à cette chaîne. Le récepteur ne doit pas supposer qu’un appel `recv()` correspond exactement à un appel `send()`.

Il faut donc définir un mécanisme de framing au niveau applicatif.

### Framing

Un format simple peut être :

```text
+----------+----------------+
|  LENGTH  |    PAYLOAD     |
| 4 octets | N octets       |
+----------+----------------+
```

Le récepteur lit la longueur puis récupère exactement le nombre d’octets annoncé.

Un protocole plus riche peut ajouter un type :

```text
+------+----------+----------------+
| TYPE |  LENGTH  |    PAYLOAD     |
+------+----------+----------------+
```

TCP assure alors le transport, tandis que l’application définit la signification du contenu.

## 3. HTTP

### Présentation

HTTP, Hypertext Transfer Protocol, est un protocole applicatif utilisé pour les échanges entre clients et serveurs.

Une communication HTTP contient notamment :

- une méthode ;
- une cible ou ressource ;
- des en-têtes ;
- éventuellement un corps ;
- une réponse contenant un statut, des en-têtes et éventuellement un corps.

Exemple conceptuel :

```text
POST /command
Content-Type: application/json

{
    "command": "dir"
}
```

Réponse :

```text
200 OK
Content-Type: application/json

{
    "result": "..."
}
```

### Fonctionnement dans une architecture client/serveur

```text
Client
  |
  | HTTP request
  v
Serveur
  |
  | HTTP response
  v
Client
```

HTTP s’appuie traditionnellement sur TCP pour son transport. Les versions modernes peuvent également utiliser d’autres transports, notamment HTTP/3 avec QUIC.

### Sécurité

HTTP seul ne chiffre pas le contenu. HTTPS correspond à HTTP transporté avec TLS.

Une architecture peut donc être représentée ainsi :

```text
Application
    |
   HTTP
    |
   TLS
    |
   TCP
    |
   IP
```

## 4. DNS

### Présentation

DNS, Domain Name System, est principalement utilisé pour traduire des noms de domaine en informations réseau.

Exemple :

```text
www.example.com
        |
        v
     réponse DNS
```

DNS fonctionne principalement avec UDP et peut également utiliser TCP dans certaines situations.

### Structure générale

Une requête DNS comporte notamment :

- un identifiant de transaction ;
- des indicateurs ;
- des questions ;
- éventuellement des réponses et informations complémentaires.

### Communication

Un client DNS interroge un serveur DNS :

```text
Client
   |
   | DNS query
   v
DNS server
   |
   | DNS response
   v
Client
```

### Utilisation comme tunnel

Le sujet cite DNS parmi les protocoles pouvant être utilisés pour le tunnel. fileciteturn2file0

Dans ce contexte, l’étude porte notamment sur la manière dont des informations applicatives peuvent être transportées à travers des échanges DNS et sur les caractéristiques observables par les outils de défense.

## 5. IRC

### Présentation

IRC, Internet Relay Chat, est un protocole de communication textuelle conçu pour permettre à plusieurs utilisateurs ou clients de communiquer via des serveurs.

Son architecture peut être représentée ainsi :

```text
Client A
    |
    v
Serveur IRC
    ^
    |
Client B
```

IRC organise notamment les communications autour de commandes, réponses et canaux.

### Sessions et canaux

Un client peut établir une connexion avec un serveur puis rejoindre un canal.

Les messages peuvent ensuite être distribués aux participants concernés.

### Utilisation dans un système de communication

L'architecture IRC permet d'étudier un modèle dans lequel un serveur intermédiaire distribue des messages entre plusieurs participants.

Le sujet cite IRC parmi les protocoles utilisables pour le tunnel. fileciteturn2file0

## 6. SSH

### Présentation

SSH, Secure Shell, est un protocole conçu pour fournir des communications sécurisées entre deux machines.

Il permet notamment :

- l'authentification ;
- la confidentialité des communications ;
- l'intégrité des données ;
- l'ouverture de sessions ;
- différents mécanismes de transfert ou de redirection.

Architecture générale :

```text
Client SSH
    |
    | connexion sécurisée
    v
Serveur SSH
```

### Sécurité

SSH établit une session sécurisée comprenant notamment des mécanismes d'échange de clés, d'authentification et de chiffrement.

### Utilisation

SSH est couramment utilisé pour l'administration distante et les transferts sécurisés.

Le sujet le cite parmi les protocoles pouvant servir de tunnel. fileciteturn2file0

## 7. FTP

### Présentation

FTP, File Transfer Protocol, est conçu pour le transfert de fichiers entre un client et un serveur.

Architecture générale :

```text
Client FTP
    |
    | commandes
    v
Serveur FTP
    |
    | transfert de données
    v
Client FTP
```

FTP utilise une connexion de contrôle et un mécanisme distinct pour les transferts de données.

### Sécurité

FTP classique ne fournit pas de chiffrement du contenu.

Des variantes comme FTPS ajoutent TLS.

Il faut donc distinguer le protocole FTP lui-même de son utilisation avec une couche de sécurité.

### Utilisation

FTP est principalement orienté transfert de fichiers et fait partie des protocoles explicitement cités par le sujet pour le tunnel. fileciteturn2file0

## 8. RDP

### Présentation

RDP, Remote Desktop Protocol, est un protocole conçu pour fournir une expérience de bureau distant.

Il permet à un client d'interagir avec une session graphique exécutée sur une autre machine.

Architecture :

```text
Client RDP
    |
    | session distante
    v
Machine Windows
```

### Fonctionnement général

Une session RDP transporte différentes catégories de données nécessaires à l'expérience distante, notamment :

- entrées utilisateur ;
- affichage ;
- données de session ;
- mécanismes de contrôle et de négociation.

RDP peut utiliser plusieurs mécanismes de transport selon la configuration, notamment TCP et UDP.

### Utilisation dans le projet

Le sujet cite RDP parmi les protocoles pouvant être utilisés pour le tunnel et prévoit également une capacité `rdp` dans les fonctionnalités étendues. fileciteturn2file0

## 9. TLS

### Présentation

TLS, Transport Layer Security, est un protocole de sécurité destiné à protéger les communications.

Il apporte notamment :

- confidentialité ;
- intégrité ;
- authentification du serveur dans les scénarios utilisant des certificats ;
- négociation de paramètres cryptographiques.

Une architecture courante est :

```text
Application
    |
 TLS
    |
 TCP
    |
 IP
```

### Dans le projet

TLS peut être utilisé pour protéger une communication TCP ou un protocole applicatif tel que HTTP.

Il faut distinguer :

```text
TCP
```

qui transporte un flux d'octets,

de :

```text
TLS
```

qui protège les données transportées,

et de :

```text
protocole applicatif
```

qui définit la signification des messages.

## 10. Framing et protocole applicatif

Quel que soit le transport choisi, il est utile de distinguer :

```text
Transport
    |
    v
Framing
    |
    v
Protocole applicatif
    |
    v
Logique des commandes
```

Avec TCP brut, le framing doit être défini par l'application.

Exemple :

```text
+------+----------+----------------+
| TYPE |  LENGTH  |    PAYLOAD     |
+------+----------+----------------+
```

Le protocole peut ensuite définir la signification des types :

```text
COMMAND
RESULT
ERROR
CLOSE
```

Cette séparation permet de distinguer clairement les responsabilités entre transport, format des messages et logique applicative.

## 11. Relation avec l'architecture s0P0wn3d

Le sujet décrit s0P0wn3d comme un système C2 avec un modèle implant/controller et demande notamment un tunnel utilisant un protocole existant ainsi que des communications chiffrées. fileciteturn2file1

Une architecture conceptuelle peut donc être organisée ainsi :

```text
+-------------------------------+
|       Command layer           |
+-------------------------------+
|       Protocol layer          |
+-------------------------------+
|     Security / TLS layer      |
+-------------------------------+
|       Transport layer         |
+-------------------------------+
|             IP                |
+-------------------------------+
```

Cette séparation permet d'étudier chaque mécanisme indépendamment et de documenter les choix techniques de l'équipe.

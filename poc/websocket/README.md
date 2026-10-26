# POC : Communication WebSocket sécurisée avec TLS

## 1. Présentation du projet

Ce POC a pour objectif de démontrer la mise en place d'une communication bidirectionnelle entre un hôte Windows 10 et une machine virtuelle Windows 11, à l'aide de WebSocket et du protocole TLS.

Le chiffrement des échanges repose sur OpenSSL et vise à garantir la confidentialité des données transmises sur le réseau.

## 2. Architecture

- **Serveur :** machine hôte Windows 10.
- **Client :** machine virtuelle Windows 11.
- **Protocole applicatif :** WebSocket sécurisé (`wss://`).
- **Chiffrement :** TLS via OpenSSL.
- **Bibliothèque WebSocket :** IXWebSocket.
- **Compilation :** CMake.

## 3. Technologies utilisées

- **C++20** : langage de programmation.
- **IXWebSocket** : bibliothèque de communication WebSocket.
- **TLS** : sécurisation de la connexion réseau.
- **OpenSSL** : backend cryptographique TLS, sous réserve de confirmation de sa sélection dans la configuration CMake.
- **CMake** : configuration et compilation du projet.
- **MinGW Makefiles** : générateur utilisé pour la compilation sous Windows.

IXWebSocket est récupéré via CMake FetchContent à partir d'un commit Git précis, afin de conserver une version reproductible de la dépendance.

## 4. Structure du projet

- `server/main.cpp` : implémentation du serveur.
- `certs/key.pem` : Clé privée associée.
- `certs/cert.pem` : Certificat du serveur.
- `client/main.cpp` : implémentation du client.
- `CMakeLists.txt` : configuration de la compilation.

## 5. Configuration TLS

### Côté serveur

Le serveur utilise un objet `ix::SocketTLSOptions` pour configurer TLS.

- `tls = true` active TLS pour le serveur.
- `certFile` désigne le certificat du serveur (`cert.pem`).
- `keyFile` désigne la clé privée associée (`key.pem`).
- `caFile = "NONE"` désactive la vérification des certificats clients. Le serveur n'exige donc pas de certificat client dans cette configuration.

Le serveur écoute sur le port `9090` et l'adresse `0.0.0.0`, afin d'accepter les connexions sur ses interfaces réseau.

### Côté client

Le client configure TLS à l'aide de `ix::SocketTLSOptions`.

- `tls = true` active TLS.
- `caFile` désigne le certificat de confiance utilisé pour vérifier le certificat présenté par le serveur.
- L'URL utilise le schéma `wss://`, qui correspond à WebSocket sécurisé par TLS.

Le chemin du répertoire contenant le certificat peut être fourni en argument au programme. En l'absence d'argument, le répertoire `certs` est utilisé par défaut.

### Certificats

Le répertoire de certificats contient les fichiers suivants :

- `cert.pem` : certificat présenté par le serveur et utilisé par le client pour établir la confiance.
- `key.pem` : clé privée du serveur.

La clé privée ne doit pas être publiée dans le dépôt Git.

Le certificat doit correspondre à l'adresse utilisée par le client. Lorsque la connexion utilise une adresse IP, celle-ci doit notamment être présente dans l'extension Subject Alternative Name (SAN) du certificat, sous forme d'adresse IP.

## 6. Génération des certificats TLS

Le POC utilise un certificat autosigné généré avec OpenSSL. La clé privée est créée au format PEM avec l'algorithme RSA et une taille de 2048 bits. Le certificat est ensuite généré à partir de cette clé et configuré avec des Subject Alternative Names (SAN) correspondant aux adresses utilisées pour les connexions.

### Génération de la clé privée

```bash
openssl genrsa -out certs/key.pem 2048
```

Cette commande génère une clé privée RSA de 2048 bits et l'enregistre dans `certs/key.pem`.

### Génération du certificat autosigné

```bash
openssl req -new -x509 \
  -key certs/key.pem \
  -out certs/cert.pem \
  -days 365 \
  -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:<IP_PRIVÉ_PERSO>,IP:<IP_PRIVÉ_EPITECH>,IP:::1"
```

Le certificat est autosigné et valable pendant 365 jours.

Le champ `CN` est défini à `localhost`. L'extension `subjectAltName` contient les noms DNS et adresses IP prévus pour les connexions au serveur.

### Fichiers générés

- `certs/key.pem` : clé privée du serveur, à protéger et à ne pas publier.
- `certs/cert.pem` : certificat autosigné du serveur, utilisé également côté client comme certificat de confiance.

## 7. Tests réalisés

Les tests doivent permettre de vérifier les éléments suivants :

- Établissement de la connexion WebSocket sécurisée.
- Échange de messages entre le client et le serveur.
- Réception de plusieurs messages sans fermeture prématurée de la connexion.
- Arrêt du client à la réception de la commande `quit`.
- Validation du certificat serveur par le client.
- Échec de la connexion lorsque le certificat n'est pas approuvé ou ne correspond pas à l'adresse utilisée.

### État des tests TLS

Le POC a été testé en local, avec le client et le serveur exécutés dans deux terminaux distincts sur l'hôte Windows 10. Les échanges ont fonctionné dans cette configuration.

Lors d'un test distant, avec le serveur exécuté sur l'hôte Windows 10 et le client sur une machine virtuelle Windows 11, la connexion a échoué avec l'erreur suivante :

`CRL, CA or signature check failed`

Cette erreur indique un échec de vérification du certificat TLS. Sa cause exacte n'a pas été identifiée à ce stade.

**Limite connue :** le fonctionnement de la communication TLS entre deux terminaux sur la même machine a été observé, mais la connexion TLS entre l'hôte et la machine virtuelle reste à valider. La configuration de confiance du certificat et sa validation dans l'environnement distant nécessitent des investigations complémentaires.

## 8. Compilation

Depuis la racine du projet, configurer puis compiler les exécutables avec CMake :

```powershell
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Les exécutables produits sont :

- `WebSocketServer` : serveur WebSocket.
- `WebSocketClient` : client WebSocket.

## 9. Limites et améliorations

### Limitations et améliorations possibles

### Limitations identifiées

- **Validation TLS en environnement distant :** les échanges sécurisés ont fonctionné lors des tests en local, avec le client et le serveur exécutés sur l'hôte Windows 10. En revanche, un test avec le client sur une machine virtuelle Windows 11 a échoué avec l'erreur `CRL, CA or signature check failed`. La cause exacte n'a pas été identifiée.
- **Certificat autosigné :** le POC utilise un certificat autosigné, adapté à une expérimentation contrôlée. Sa configuration et sa validation devront être réévaluées pour tout autre environnement.
- **Gestion des connexions :** le POC permet l'établissement d'une connexion et l'échange de messages. La gestion des déconnexions inattendues, des reconnexions et des erreurs réseau reste à approfondir.
- **Périmètre expérimental :** le POC a pour objectif de valider la faisabilité technique des échanges WebSocket sécurisés. Il ne constitue pas une implémentation prête pour la production.

### Améliorations possibles

- Identifier la cause de l'échec de validation TLS entre l'hôte et la machine virtuelle.
- Vérifier la configuration du backend TLS utilisé par IXWebSocket et les paramètres de validation du certificat.
- Compléter les tests avec des cas d'échec : certificat non approuvé, identité du serveur non conforme et interruption de la connexion.
- Ajouter une gestion explicite des erreurs, des déconnexions et des reconnexions.
- Évaluer les besoins en matière d'authentification, de journalisation et de supervision avant toute réutilisation dans un environnement réel.

## 10. Références

- Documentation officielle d'IXWebSocket.
- Documentation officielle d'OpenSSL.
- Documentation du protocole TLS et de WebSocket.

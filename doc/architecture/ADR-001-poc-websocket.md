# ADR-001 — Non-rétention de WebSocket

- **Date :** 15 octobre 2026
- **Statut :** Rejeté
- **Projet :** sOPown3d

## 1. Contexte

Dans le cadre du projet sOPown3d, un prototype (POC) a été développé en C++ à l'aide de la bibliothèque IXWebSocket afin d'évaluer la faisabilité d'une communication WebSocket sécurisée par TLS.

Cette expérimentation a permis de tester l'établissement d'une connexion persistante et bidirectionnelle, ainsi que l'échange de messages entre un client et un serveur.

À l'issue de cette étude, WebSocket n'a pas été retenu pour l'architecture cible du projet.

## 2. Motifs de la décision

WebSocket repose sur une connexion persistante et bidirectionnelle. Son utilisation nécessite notamment de prendre en compte le cycle de vie de la connexion, les déconnexions et les mécanismes de reconnexion.

La sécurité de cette communication dépend également de la configuration TLS et de la validation des certificats. Le caractère persistant d'une connexion ne constitue toutefois pas, à lui seul, une vulnérabilité.

Pour le projet sOPown3d, une architecture fondée sur des requêtes HTTPS indépendantes a été privilégiée. Ce modèle est considéré comme plus adapté aux contraintes retenues pour le projet.

La décision ne résulte pas d'une impossibilité technique de mettre en œuvre WebSocket, mais d'un choix d'architecture effectué à l'issue de l'expérimentation.

## 3. Conséquences

- Le POC WebSocket est conservé dans `poc/websocket/`.
- Sa documentation technique est disponible dans `poc/websocket/README.md`.
- La solution retenue pour la suite du projet est documentée dans `ADR-002-canal-https.md`.

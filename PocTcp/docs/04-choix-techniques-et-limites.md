# Choix techniques, avantages et limites

## C++17

C++ a été choisi pour son accès direct aux API Windows et à Winsock, sa compilation native et son contrôle précis des ressources réseau.

Ce choix est adapté à une première preuve de concept bas niveau. Il implique cependant une responsabilité importante pour la gestion des ressources, des erreurs, de la mémoire et de la durée de vie des objets.

## Winsock

Winsock est l’API réseau native de Windows. Elle permet d’observer clairement les opérations `socket`, `bind`, `listen`, `accept`, `connect`, `send` et `recv`.

Son principal avantage est l’absence de couche réseau supplémentaire. Son principal inconvénient est son faible niveau d’abstraction : la fragmentation des données, les erreurs, les délais et les fermetures doivent être gérés par le projet.

## CMake

CMake rend la compilation reproductible et sépare les deux exécutables. Le script `build.bat` simplifie la construction sous Windows.

Les répertoires `build` générés par CMake devront être exclus de la livraison finale avec les autres artefacts temporaires ou binaires non nécessaires.

## Avantages du prototype

Le prototype est simple à compiler, à observer et à déboguer. La séparation entre réseau, protocole et commandes rend les responsabilités compréhensibles.

Le framing applicatif prend en compte le fonctionnement réel de TCP. Les tailles reçues sont contrôlées avant les allocations et les commandes sont limitées à une liste connue.

Le réseau privé VirtualBox permet de reproduire les essais dans un environnement contrôlé sans exposer la communication à un réseau de production.

## Inconvénients du prototype

L’adresse IP et le port sont codés en dur. Le contrôleur ne gère qu’un client et l’implant ne possède pas de mécanisme de reconnexion.

Le format binaire est spécifique au prototype et dépend de la représentation locale des entiers. Il n’existe pas de négociation complète de version ou de compatibilité.

Le protocole applicatif ne fournit aucune authentification, aucun chiffrement et aucune protection cryptographique de l’intégrité. Cette absence est une limite du protocole développé, et non une propriété de tous les protocoles réseau.

L’utilisation de `_popen` et de commandes Windows connues augmente la surface de détection. Le prototype ne démontre pas la discrétion, la persistance, la résilience ou la résistance aux solutions de sécurité.

La gestion des sorties volumineuses, des délais, des arrêts propres et des erreurs de reconnexion doit être renforcée avant toute évolution.

## Positionnement par rapport au projet final

PocTcp valide la boucle minimale suivante :

```text
Saisie
  → Transmission
  → Vérification
  → Exécution
  → Réponse
```

Il ne doit pas être présenté comme une solution finale conforme à toutes les exigences du sujet. Les fonctionnalités de sécurité, de tunnel applicatif, de résilience et de discrétion nécessitent une phase de conception séparée.

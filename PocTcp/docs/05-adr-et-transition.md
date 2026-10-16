# ADR et transition vers la solution finale

## Pourquoi ne pas retenir le protocole personnalisé

### Clarification

TCP est un protocole de transport. Il fournit un flux fiable entre deux sockets, mais ne définit pas les commandes, les sessions applicatives, l’authentification ou le chiffrement.

Dans PocTcp, le format écrit par l’équipe est un protocole applicatif personnalisé placé au-dessus de TCP. Il définit un framing, des longueurs et des types de messages. Il ne s’agit pas d’un nouveau protocole de transport.

### Raisons techniques

Un protocole personnalisé devrait traiter au minimum :

* la négociation et la compatibilité des versions ;
* l’authentification des deux extrémités ;
* l’échange et la rotation des clés ;
* le chiffrement et l’intégrité des messages ;
* la protection contre la relecture ;
* les délais et les reconnexions ;
* la désynchronisation et les messages incomplets ;
* les tests d’interopérabilité.

Le prototype montre déjà les difficultés du framing manuel. Il est donc pertinent comme étape de validation, mais pas comme format final.

### Décision

Le protocole personnalisé est conservé uniquement pour la preuve de concept. La solution finale utilisera un protocole applicatif existant et une couche de sécurité adaptée. Le choix devra être justifié selon la conformité au sujet, la faisabilité sous Windows, la qualité des bibliothèques, la testabilité et l’analyse défensive du trafic.

## ADR-001 : séparer le contrôleur et l’implant

**Statut :** accepté

**Contexte :** le sujet impose une architecture implant/controller.

**Décision :** le contrôleur gère l’interface opérateur et l’envoi ; l’implant gère la connexion, l’exécution locale et le retour.

**Conséquence :** les responsabilités sont séparées, mais les deux composants doivent partager un protocole compatible.

## ADR-002 : utiliser C++17 et Winsock pour le prototype

**Statut :** accepté pour le prototype

**Contexte :** la première étape nécessite une communication fonctionnelle sur Windows.

**Décision :** utiliser C++17, Winsock et CMake.

**Conséquence :** le code est proche des API Windows, mais la gestion des ressources et des erreurs reste à la charge de l’équipe.

## ADR-003 : ajouter un framing applicatif au-dessus de TCP

**Statut :** accepté pour le prototype, rejeté comme solution finale

**Contexte :** TCP ne conserve pas les limites des messages applicatifs.

**Décision :** utiliser des champs de longueur et un type de message pour reconstruire les commandes et les réponses.

**Conséquence :** la communication est suffisamment structurée pour la validation, mais le format reste spécifique, non chiffré et non conforme à l’exigence de protocole existant.

## ADR-004 : ne pas utiliser le protocole personnalisé dans la solution finale

**Statut :** accepté

**Contexte :** le sujet demande un protocole existant et des communications protégées.

**Décision :** remplacer la couche personnalisée par un protocole applicatif existant, avec une sécurité correctement intégrée.

**Conséquence :** la couche réseau devra être réécrite, mais la solution sera plus conforme au sujet et plus facile à justifier.

## ADR-005 : utiliser une liste blanche de commandes

**Statut :** accepté pour le prototype

**Contexte :** il fallait démontrer l’échange de commandes sans introduire immédiatement un shell arbitraire.

**Décision :** reconnaître uniquement `dir`, `whoami`, `ipconfig`, `pwd`, `cd` et `exit`.

**Conséquence :** le comportement est prévisible et testable, mais le prototype ne fournit pas encore un shell distant complet.

## ADR-006 : utiliser une VM Windows isolée

**Statut :** accepté

**Contexte :** le sujet demande une VM Windows disposable, un snapshot et un environnement reproductible.

**Décision :** exécuter le contrôleur sur l’hôte et l’implant dans une VM reliée par un réseau privé VirtualBox.

**Conséquence :** les essais restent contrôlés et restaurables, mais la configuration de la VM doit être documentée.

## Transition vers la solution finale

Les prochaines étapes sont :

1. conserver PocTcp comme preuve de concept ;
2. sélectionner un protocole applicatif existant ;
3. remplacer le framing personnalisé ;
4. ajouter l’authentification, le chiffrement et l’intégrité ;
5. externaliser l’adresse et les paramètres ;
6. ajouter des délais et une stratégie de reconnexion ;
7. analyser le trafic avec Wireshark ;
8. documenter les observations côté défense ;
9. associer chaque fonctionnalité implémentée à une technique MITRE ATT&CK ;
10. nettoyer les artefacts de compilation avant la livraison.

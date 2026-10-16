# Contexte et périmètre

## Contexte du projet

Le sujet Virology demande de concevoir un outil de type Command and Control dans un laboratoire maîtrisé. L’architecture attendue repose sur un contrôleur et un implant, avec une démonstration sur une machine virtuelle Windows jetable.

Le sujet demande notamment :

* une communication entre le contrôleur et la cible ;
* un tunnel utilisant un protocole applicatif existant ;
* le chiffrement des communications ;
* un shell distant ;
* de la persistance ;
* l’extraction de secrets ;
* une démonstration reproductible et documentée ;
* une analyse du point de vue de l’attaquant et du défenseur.

## Objectif de PocTcp

PocTcp est une preuve de concept destinée à valider la communication de base entre un contrôleur et un implant Windows. Le prototype permet de vérifier la connexion TCP, le framing applicatif, l’envoi de commandes, l’exécution d’un nombre limité d’actions et le retour des résultats.

Il ne constitue pas encore l’implémentation finale de `s0P0wn3d`. Il sert de base technique avant l’intégration d’un protocole applicatif existant et des mécanismes de sécurité requis par le projet.

## Périmètre réalisé

Les éléments suivants sont fonctionnels dans le prototype :

* initialisation de Winsock dans les deux programmes ;
* écoute du contrôleur sur `192.168.56.1:5000` ;
* connexion de l’implant au contrôleur ;
* transmission d’un nom de commande et d’un argument ;
* retour d’une réponse ou d’une erreur ;
* commandes `dir`, `whoami`, `ipconfig`, `pwd`, `cd` et `exit` ;
* compilation avec CMake en C++17 ;
* test prévu dans un réseau privé VirtualBox.

## Éléments non couverts

| Exigence | État |
|---|---|
| Protocole applicatif existant pour le tunnel | Non couvert |
| Chiffrement et authentification | Non implémentés |
| Shell distant complet | Non implémenté |
| Persistance et redémarrage | Non implémentés |
| Extraction de secrets | Non implémentée |
| Fonctionnement discret | Non démontré |
| Reconnexion et résilience | Non implémentées |
| Gestion de plusieurs implants | Non implémentée |

Cette distinction évite de présenter le prototype comme une solution complète alors qu’il valide principalement la boucle de communication.

# Comparaison des systèmes d’exploitation pour s0P0wn3d

## 1. Objectif

Ce document présente les éléments à prendre en compte pour choisir le système d’exploitation de la machine virtuelle cible utilisée dans le projet s0P0wn3d.

Le sujet impose que l’outil soit testé au minimum sur une machine virtuelle Windows. Il laisse à l’équipe le choix du système d’exploitation et de sa version, en indiquant qu’une version récente de Windows constitue un choix raisonnable. Le sujet demande également de disposer d’une machine virtuelle jetable, de réaliser un snapshot et de rendre l’environnement reproductible. fileciteturn3file0

Ce document ne retient aucune version de Windows. Il sert à structurer la réflexion avant le choix définitif.

## 2. Contraintes imposées par le projet

Le choix de l’OS doit prendre en compte plusieurs contraintes :

- l’environnement doit être une machine virtuelle contrôlée par l’équipe ;
- l’outil doit fonctionner sur Windows ;
- la machine doit être disposable et pouvoir être restaurée à partir d’un snapshot ;
- l’environnement doit être suffisamment stable pour une démonstration ;
- l’environnement doit pouvoir être décrit dans un manifest reproductible ;
- les mécanismes de sécurité présents sur le système doivent pouvoir être étudiés dans le laboratoire ;
- les fonctionnalités développées doivent pouvoir être expliquées du point de vue offensif et défensif.

Le sujet précise que l’équipe choisit et maîtrise son propre environnement et qu’une version récente de Windows est une option raisonnable, sans imposer une édition ou une version précise. fileciteturn3file0

## 3. Windows 10

### Présentation

Windows 10 constitue une génération de Windows adaptée à l'étude des mécanismes Windows classiques.

### Intérêt pour le projet

Une VM Windows 10 permet notamment de travailler avec :

- les API Windows ;
- Winsock ;
- les services Windows ;
- les mécanismes de démarrage ;
- le système de fichiers NTFS ;
- les journaux Windows ;
- Windows Defender et les mécanismes de sécurité disponibles dans l’environnement.

### Points à étudier

L’équipe doit documenter précisément :

- l’édition utilisée ;
- la version et le build ;
- les mises à jour installées ;
- les composants de sécurité actifs ;
- la configuration réseau ;
- les outils d’observation utilisés.

### Démonstration

Une version précise doit être conservée dans l’environnement manifest afin que les membres de l’équipe puissent reproduire la même configuration.

## 4. Windows 11

### Présentation

Windows 11 constitue une génération plus récente de Windows et correspond directement à la recommandation générale du sujet concernant l’utilisation d’un Windows récent. fileciteturn3file0

### Intérêt pour le projet

Une VM Windows 11 permet d'étudier :

- les API Windows modernes ;
- les mécanismes réseau Windows ;
- les services et mécanismes de démarrage ;
- les journaux système ;
- les mécanismes de sécurité présents dans l’environnement ;
- les outils de détection et d’observation disponibles sur une version récente.

### Points à étudier

L’équipe doit documenter :

- l’édition ;
- le numéro de version ;
- le build ;
- les mises à jour ;
- les protections actives ;
- la configuration réseau ;
- la configuration de la VM.

### Démonstration

Comme pour toute autre version, la reproductibilité est importante. Le sujet demande un environnement maîtrisé, une VM disposable et un snapshot. fileciteturn3file0

## 5. Édition de Windows

Le choix ne porte pas uniquement sur le numéro de version. L’édition Windows peut également modifier les fonctionnalités disponibles.

Lors de la comparaison, il faut donc vérifier :

- les fonctionnalités système nécessaires au projet ;
- les outils d’administration disponibles ;
- les fonctionnalités de sécurité ;
- les possibilités de configuration ;
- les restrictions éventuelles liées à l’édition ;
- la disponibilité de la licence dans l’environnement de laboratoire.

L’équipe doit éviter de choisir une édition uniquement parce que son nom est plus récent ou plus complet. La justification doit être liée aux besoins réels du projet.

## 6. Version et build

Pour une VM de laboratoire, écrire simplement :

```text
Windows 11
```

n’est pas suffisamment précis pour assurer la reproductibilité.

Il est préférable de documenter au minimum :

```text
Operating System : Windows 11
Edition          : <édition>
Version          : <version>
Build            : <build>
Architecture     : x64
```

Les autres éléments importants peuvent être ajoutés dans l’environnement manifest :

```text
RAM
CPU
Disque
VirtualBox version
Configuration réseau
Snapshot
Protections actives
Logiciels installés
```

## 7. Machine virtuelle

Le sujet demande explicitement une machine virtuelle Windows et recommande une machine disposable accompagnée d’un snapshot. fileciteturn3file1

Une organisation possible est :

```text
Host
 |
 +--- Controller
 |
 +--- VirtualBox
       |
       +--- Windows VM
              |
              +--- Snapshot propre
              |
              +--- PocTcp
```

Le snapshot permet de restaurer l’environnement après une expérimentation.

Cela permet également de séparer clairement :

```text
État propre
    ↓
Test
    ↓
Observation
    ↓
Restauration
    ↓
Nouveau test
```

## 8. Configuration réseau

Le choix de l’OS doit également être étudié avec la configuration réseau de la VM.

Dans le laboratoire, il est utile de distinguer :

```text
Réseau de laboratoire
        |
        +--- Controller
        |
        +--- Windows VM
```

La configuration exacte doit être documentée dans le manifest.

Les éléments à conserver sont notamment :

- type d’adaptateur VirtualBox ;
- adresse IP ;
- masque ;
- passerelle si nécessaire ;
- DNS ;
- règles de pare-feu ;
- ports utilisés par le projet.

L’objectif est que les membres de l’équipe puissent reproduire l’environnement sans dépendre d’une configuration implicite.

## 9. Sécurité de l’OS

Le projet demande d'étudier le comportement de l’outil dans un environnement disposant de mécanismes de sécurité et de pouvoir discuter de la détection. fileciteturn3file1

La configuration de la VM doit donc documenter au minimum les protections pertinentes pour le laboratoire.

Exemples :

```text
Windows Defender
Pare-feu Windows
Journalisation Windows
Contrôles de compte utilisateur
Services actifs
Outils d’analyse
```

La configuration doit rester contrôlée et reproductible.

## 10. Critères de décision

Avant de retenir une version, l’équipe peut utiliser les critères suivants :

| Critère | Question à poser |
|---|---|
| Compatibilité | L’outil fonctionne-t-il correctement sur cette version ? |
| API Windows | Les API nécessaires sont-elles disponibles ? |
| Réseau | Winsock et les mécanismes réseau nécessaires sont-ils disponibles ? |
| Sécurité | Les protections présentes permettent-elles une observation pertinente ? |
| Débogage | Les outils nécessaires sont-ils disponibles ? |
| Virtualisation | La VM fonctionne-t-elle correctement dans l’environnement de l’équipe ? |
| Reproductibilité | Peut-on reconstruire exactement l’environnement ? |
| Stabilité | La démonstration peut-elle être reproduite sans modification imprévisible ? |
| Documentation | Peut-on documenter précisément la version et la configuration ? |
| Licence | L’utilisation de l’édition choisie est-elle compatible avec le laboratoire ? |

## 11. Manifest d’environnement

Le sujet demande un environnement manifest afin de rendre la démonstration reproductible. fileciteturn3file0

Un manifest peut contenir :

```text
OS:
  name:
  edition:
  version:
  build:
  architecture:

Virtualisation:
  hypervisor:
  version:
  cpu:
  memory:
  disk:

Network:
  adapter:
  ip:
  subnet:
  gateway:
  dns:

Security:
  antivirus:
  firewall:
  logging:

Project:
  controller:
  implant:
  ports:
  snapshot:
```

Le fichier peut ensuite être versionné avec le projet.

## 12. Validation avant choix définitif

Avant de retenir l’OS, l’équipe peut effectuer une validation minimale :

### Étape 1

Installer l’OS dans une VM propre.

### Étape 2

Documenter la version exacte et le build.

### Étape 3

Configurer le réseau du laboratoire.

### Étape 4

Créer le snapshot de référence.

### Étape 5

Vérifier la compilation et l’exécution du projet.

### Étape 6

Tester la communication entre le controller et l’implant dans le laboratoire.

### Étape 7

Vérifier les outils d’observation nécessaires, notamment l’analyse du trafic réseau.

### Étape 8

Documenter les résultats et les éventuelles limitations.

## 13. Conclusion

Le sujet ne prescrit pas une version particulière de Windows. Il impose une machine virtuelle Windows et laisse à l’équipe le choix du système et de sa version, en indiquant qu’un Windows récent constitue un choix raisonnable. fileciteturn3file0

Le choix final doit donc être documenté à partir de critères techniques mesurables :

- compatibilité avec le code ;
- disponibilité des API nécessaires ;
- comportement réseau ;
- mécanismes de sécurité ;
- outils de diagnostic ;
- stabilité de la VM ;
- reproductibilité ;
- configuration exacte de l’environnement.

L’objectif est de pouvoir expliquer non seulement quelle version a été utilisée, mais également comment elle a été configurée et pourquoi cette configuration permet une démonstration reproductible du projet.

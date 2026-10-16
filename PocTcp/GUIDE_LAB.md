# Guide de mise en oeuvre du laboratoire

## 1. Objectif

Ce document décrit la procédure complète pour compiler le contrôleur et l'implant, partager l'exécutable avec une machine virtuelle Windows dans VirtualBox, puis établir la connexion dans un environnement de laboratoire isolé.

Le contrôleur s'exécute sur la machine hôte. L'implant s'exécute dans la machine virtuelle. Les deux programmes communiquent sur le réseau privé VirtualBox à l'adresse `192.168.56.1` et sur le port `5000`.

## 2. Pré-requis

La machine hôte doit disposer des éléments suivants :

- Windows
- CMake
- Visual Studio ou les outils de compilation MSVC
- VirtualBox
- Une machine virtuelle Windows 10
- Les Guest Additions VirtualBox installées dans la machine virtuelle

La machine virtuelle doit être utilisée uniquement dans le cadre du laboratoire prévu. Le réseau utilisé doit rester isolé et contrôlé.

## 3. Compilation

Ouvrir PowerShell à la racine du projet :

```powershell
cd "C:\Users\Jaures Acs\Documents\delivery\T-SEC-901-Virology\PocTcp"
```

Lancer le script de compilation :

```powershell
.\build.bat
```

Le script configure CMake puis compile les configurations Debug et Release.

Avec la configuration CMake racine, les exécutables Release sont générés ici :

```text
build\controller\Release\controller.exe
build\implant\Release\implant.exe
```

L'exécutable à partager avec la machine virtuelle est :

```text
build\implant\Release\implant.exe
```

Si le dossier n'existe pas, vérifier que la compilation s'est terminée sans erreur et que CMake a bien été exécuté depuis la racine du projet.

## 4. Installation des Guest Additions

Les Guest Additions sont nécessaires pour utiliser les dossiers partagés VirtualBox.

1. Démarrer la machine virtuelle.
2. Dans la fenêtre de la machine virtuelle, ouvrir le menu `Périphériques`.
3. Sélectionner `Insérer l'image CD des additions invité`.
4. Dans Windows invité, ouvrir `Ce PC`.
5. Ouvrir le lecteur DVD `VirtualBox Guest Additions`.
6. Lancer `VBoxWindowsAdditions.exe`.
7. Accepter l'installation des pilotes et conserver les options par défaut.
8. Redémarrer la machine virtuelle.

Si VirtualBox indique que la machine ne possède pas de lecteur CD/DVD, éteindre la machine virtuelle, puis ajouter un lecteur optique vide dans :

```text
Configuration
Stockage
Ajouter un lecteur optique
```

Après cette étape, redémarrer la machine virtuelle et insérer à nouveau l'image CD des Guest Additions.

## 5. Configuration du dossier partagé

Il est recommandé d'éteindre complètement la machine virtuelle avant de configurer le partage.

Dans le gestionnaire VirtualBox :

1. Sélectionner la machine virtuelle.
2. Ouvrir `Configuration`.
3. Ouvrir `Dossiers partagés`.
4. Cliquer sur l'icône d'ajout de dossier.
5. Sélectionner le dossier qui contient réellement `implant.exe`.

Avec le build racine, utiliser :

```text
C:\Users\Jaures Acs\Documents\delivery\T-SEC-901-Virology\PocTcp\build\implant\Release
```

Renseigner les champs suivants :

```text
Nom du dossier : implant-release
Point de montage : laisser vide
Montage automatique : coché
Lecture seule : coché
```

La lecture seule empêche la machine virtuelle de modifier ou supprimer les fichiers présents sur la machine hôte.

Valider avec `OK`, puis valider à nouveau les paramètres de la machine virtuelle.

## 6. Vérification du partage dans Windows invité

Démarrer la machine virtuelle et ouvrir l'Explorateur de fichiers avec `Win + E`.

Dans la barre d'adresse, saisir :

```text
\\VBOXSVR\implant-release
```

Le fichier suivant doit être visible :

```text
implant.exe
```

Si le partage n'est pas accessible, vérifier les points suivants :

- Les Guest Additions sont installées.
- La machine virtuelle a été redémarrée après l'installation.
- Le nom du partage est exactement `implant-release`.
- Le chemin hôte sélectionné contient bien `implant.exe`.
- Le compte Windows utilisé dans la VM possède les droits nécessaires.

## 7. Démarrage du contrôleur

Sur la machine hôte, ouvrir PowerShell à la racine du projet et lancer :

```powershell
.\build\controller\Release\controller.exe
```

Le contrôleur doit rester ouvert. Il attend une connexion sur :

```text
192.168.56.1:5000
```

Si Windows affiche une alerte du pare-feu, autoriser l'application uniquement sur le réseau privé du laboratoire.

## 8. Démarrage de l'implant

Dans la machine virtuelle, ouvrir le dossier partagé :

```text
\\VBOXSVR\implant-release
```

Lancer ensuite `implant.exe`.

L'implant tente de se connecter au contrôleur. Le contrôleur doit donc être démarré avant l'implant.

Une fois la connexion établie, le contrôleur affiche une invite permettant de saisir les commandes prévues par le projet.

## 9. Ordre complet des opérations

1. Ouvrir PowerShell à la racine du projet.
2. Lancer `build.bat`.
3. Vérifier la présence de `build\implant\Release\implant.exe`.
4. Vérifier que les Guest Additions sont installées dans la VM.
5. Configurer le dossier partagé `implant-release`.
6. Démarrer la VM.
7. Vérifier l'accès à `\\VBOXSVR\implant-release`.
8. Démarrer `build\controller\Release\controller.exe` sur l'hôte.
9. Lancer `implant.exe` dans la VM.

## 10. Dépannage

### Le bouton OK du dossier partagé est grisé

Le chemin sélectionné n'existe probablement pas ou ne correspond pas à un dossier accessible. Sélectionner le dossier avec le bouton de navigation et vérifier qu'il contient `implant.exe`. Si nécessaire, éteindre complètement la machine virtuelle avant de modifier les dossiers partagés.

### Le partage n'apparaît pas dans Windows

Vérifier l'installation des Guest Additions, redémarrer la VM, puis ouvrir :

```text
\\VBOXSVR\implant-release
```

### L'implant ne se connecte pas

Vérifier les éléments suivants :

- Le contrôleur est démarré avant l'implant.
- L'adresse réseau de l'hôte est bien `192.168.56.1`.
- Le port `5000` n'est pas bloqué par le pare-feu.
- La VM possède une interface réseau Host-only correspondant à cette adresse.
- L'implant et le contrôleur proviennent de la même compilation.

### Le fichier Release est introuvable

Relancer :

```powershell
.\build.bat
```

Puis vérifier :

```text
build\implant\Release\implant.exe
```

## 11. Arrêt propre

Pour arrêter le laboratoire :

1. Fermer l'implant dans la VM.
2. Arrêter le contrôleur avec `Ctrl + C`.
3. Fermer ou suspendre la machine virtuelle.
4. Ne pas laisser le contrôleur ou l'implant actifs en dehors du réseau de laboratoire.

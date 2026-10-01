# Comparaison des langages de programmation pour s0P0wn3d

## 1. Objectif

Ce document présente plusieurs langages de programmation susceptibles d’être utilisés pour développer s0P0wn3d. Il s’agit d’une comparaison technique destinée à aider l’équipe à documenter son choix.

Aucun langage n’est retenu dans ce document. Le choix final doit être effectué en fonction des contraintes du projet, des compétences de l’équipe, des besoins bas niveau, de la maintenabilité et des capacités de démonstration dans le laboratoire.

Le projet impose notamment de construire l’outil à partir de ses propres composants et autorise l’utilisation de bibliothèques bas niveau, notamment pour la cryptographie et le réseau. Il demande également une implémentation testable sur une machine virtuelle Windows et un code maintenable. 

## 2. Critères étudiés

Les critères pertinents pour le projet sont :

- accès aux API et mécanismes Windows ;
- contrôle bas niveau du réseau et du système ;
- bibliothèques réseau et cryptographiques disponibles ;
- facilité d’implémentation du C2 ;
- gestion de la mémoire ;
- performances ;
- facilité de débogage ;
- portabilité ;
- maintenabilité ;
- rapidité de développement ;
- facilité d’intégration avec des bibliothèques existantes ;
- adéquation avec une architecture implant/controller.

## 3. C++

### Présentation

C++ est un langage compilé offrant un contrôle important sur la mémoire, les ressources système et les API natives.

### Accès système

C++ permet d’utiliser directement les API Windows et les bibliothèques natives. Il est donc adapté aux composants nécessitant un contrôle précis du système.

### Réseau

Les API Windows comme Winsock peuvent être utilisées directement. Il est également possible d’intégrer des bibliothèques de plus haut niveau pour HTTP, TLS ou d’autres protocoles.

### Cryptographie

C++ dispose de nombreuses bibliothèques cryptographiques reconnues. Il est possible de déléguer les primitives cryptographiques à une bibliothèque tout en conservant le contrôle de l’architecture.

### Mémoire

Le langage permet une gestion fine de la mémoire, mais cette liberté implique également une responsabilité importante concernant les erreurs de mémoire, les ressources et la durée de vie des objets.

### Développement

Le développement demande davantage de code et de rigueur qu’avec certains langages plus haut niveau. En contrepartie, le comportement du programme peut être contrôlé avec précision.

### Maintenabilité

Une architecture claire est importante. La séparation entre réseau, protocole, chiffrement et logique applicative permet de limiter la complexité.

### Points à examiner

- gestion manuelle ou semi-automatique des ressources ;
- complexité du code ;
- risques liés à la mémoire ;
- temps de développement ;
- accès aux API Windows.

## 4. C#

### Présentation

C# est un langage fortement intégré à l’écosystème .NET et à Windows.

### Accès système

C# permet d’utiliser de nombreuses API Windows et bibliothèques .NET. L’interopérabilité avec des API natives est également possible lorsque cela est nécessaire.

### Réseau

.NET fournit des abstractions réseau de haut niveau ainsi que des API pour TCP, HTTP, TLS et d’autres mécanismes courants.

### Cryptographie

Le framework .NET fournit des API cryptographiques standardisées, ce qui simplifie l’intégration de mécanismes de chiffrement et de certificats.

### Mémoire

La gestion mémoire est principalement assurée par le runtime .NET et le garbage collector. Cela réduit une partie des erreurs de gestion manuelle de mémoire.

### Développement

C# permet généralement d’implémenter rapidement une architecture client/serveur et de manipuler des structures de données complexes.

### Maintenabilité

Le typage fort, les outils .NET, les bibliothèques standard et l’écosystème Visual Studio facilitent la structuration et le débogage du projet.

### Points à examiner

- dépendance au runtime .NET selon le mode de déploiement ;
- niveau de contrôle inférieur à C++ sur certains mécanismes bas niveau ;
- intégration avec les API natives lorsque des besoins spécifiques apparaissent.

## 5. Rust

### Présentation

Rust est un langage compilé orienté sécurité mémoire et performances.

### Accès système

Rust peut appeler les API Windows et utiliser des bibliothèques natives. Son écosystème permet également d’accéder à des fonctionnalités système de bas niveau.

### Réseau

Des bibliothèques permettent de travailler avec TCP, HTTP, TLS et d’autres protocoles.

### Cryptographie

L’écosystème Rust comprend plusieurs bibliothèques cryptographiques, avec un intérêt particulier pour la sécurité mémoire et les abstractions modernes.

### Mémoire

Le modèle de propriété et d’emprunt de Rust cherche à empêcher de nombreuses erreurs de mémoire à la compilation.

### Développement

Le principal coût peut être la courbe d’apprentissage du modèle de propriété, particulièrement lorsqu’un programme manipule de nombreuses ressources système ou des structures concurrentes.

### Maintenabilité

Le compilateur impose de nombreuses garanties structurelles. Cela peut augmenter le temps de développement initial tout en réduisant certaines catégories de défauts.

### Points à examiner

- courbe d’apprentissage ;
- maturité des bibliothèques nécessaires au projet ;
- intégration avec les API Windows ;
- temps de développement ;
- modèle de gestion mémoire.

## 6. Go

### Présentation

Go est un langage compilé conçu pour simplifier le développement de logiciels réseau et de services.

### Accès système

Go peut interagir avec les API système et utiliser des bibliothèques natives, mais son abstraction standard est généralement plus élevée que celle de C++.

### Réseau

La bibliothèque standard de Go fournit des outils importants pour TCP, HTTP, concurrence et services réseau.

### Cryptographie

La bibliothèque standard contient notamment des primitives et protocoles cryptographiques courants, ce qui facilite la construction de services sécurisés.

### Mémoire

La mémoire est gérée automatiquement par le runtime et le garbage collector.

### Développement

Go permet de construire rapidement des composants réseau et des services concurrents.

### Maintenabilité

La simplicité syntaxique et l’outillage intégré peuvent faciliter la maintenance et la compilation de projets réseau.

### Points à examiner

- contrôle bas niveau inférieur à C++ ;
- dépendance au runtime Go ;
- interaction avec les API Windows natives ;
- adéquation avec les fonctionnalités nécessitant un contrôle système très précis.

## 7. Python

### Présentation

Python est un langage interprété de haut niveau, particulièrement adapté au prototypage et à l’automatisation.

### Accès système

Python peut utiliser de nombreuses API système et bibliothèques externes. Des mécanismes d’interopérabilité permettent également d’appeler du code natif.

### Réseau

Python dispose de bibliothèques pour TCP, HTTP, DNS, TLS et de nombreux autres protocoles.

### Cryptographie

Des bibliothèques reconnues permettent d’utiliser des mécanismes cryptographiques modernes sans implémenter les primitives directement.

### Mémoire

La gestion mémoire est automatisée.

### Développement

Python permet de prototyper rapidement une architecture client/serveur et de tester des concepts.

### Maintenabilité

La rapidité de développement est un avantage, mais la gestion des dépendances, de l’environnement Python et du déploiement sur Windows doit être prise en compte.

### Points à examiner

- dépendance à l’environnement Python ;
- performances ;
- accès aux mécanismes très bas niveau ;
- déploiement ;
- dépendances externes.

## 8. Java

### Présentation

Java est un langage fortement structuré fonctionnant généralement au-dessus de la JVM.

### Accès système

Java propose des API système et des mécanismes d’interopérabilité avec du code natif.

### Réseau

La bibliothèque standard fournit des mécanismes pour TCP, HTTP, TLS et d’autres communications.

### Cryptographie

Java dispose d’un ensemble important d’API cryptographiques dans son écosystème standard.

### Mémoire

La JVM gère automatiquement la mémoire.

### Développement

Java permet de structurer des applications réseau importantes et multiplateformes.

### Points à examiner

- présence et gestion de la JVM ;
- accès bas niveau ;
- complexité du déploiement ;
- adéquation avec les composants Windows natifs.

## 9. Tableau de comparaison

| Critère | C++ | C# | Rust | Go | Python | Java |
|---|---|---|---|---|---|---|
| Contrôle bas niveau | Très élevé | Élevé | Très élevé | Moyen | Faible à moyen | Moyen |
| API Windows | Très bon | Très bon | Bon | Bon | Bon via bibliothèques | Moyen |
| Réseau | Très bon | Très bon | Très bon | Très bon | Très bon | Très bon |
| Cryptographie | Très bon via bibliothèques | Très bon | Très bon | Très bon | Très bon via bibliothèques | Très bon |
| Gestion mémoire | Manuelle/RAII | Automatique | Contrôlée par le compilateur | Automatique | Automatique | Automatique |
| Rapidité de prototypage | Moyenne | Bonne | Moyenne | Très bonne | Très bonne | Bonne |
| Portabilité | Bonne | Bonne | Bonne | Très bonne | Très bonne | Très bonne |
| Déploiement Windows | Bon | Très bon | Bon | Bon | À gérer | À gérer |
| Accès système fin | Très élevé | Élevé | Très élevé | Moyen à élevé | Variable | Moyen |
| Courbe d’apprentissage | Élevée | Moyenne | Élevée | Faible à moyenne | Faible | Moyenne |

## 10. Conclusion

Ce document ne retient aucun langage.

Les différences importantes concernent principalement le niveau de contrôle du système, la gestion mémoire, l’écosystème réseau et cryptographique, le coût de développement et le déploiement sur Windows.

Le choix final doit être relié aux objectifs effectivement retenus par l’équipe et aux composants que celle-ci souhaite implémenter elle-même.

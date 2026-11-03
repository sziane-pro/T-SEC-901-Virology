# Proposition d'architecture : isolation de l'infrastructure via un serveur tampon

## 1. Contexte
Dans le cadre du projet, l'infrastructure de notre outil doit impérativement répondre à deux exigences majeures dictées par le cahier des charges : la furtivité face aux solutions de surveillance et la résilience sur la durée. L'exposition directe du serveur principal au client contrevient à cet objectif de discrétion absolue, car elle facilite l'identification et la neutralisation de l'outil.

## 2. Décision
Pour garantir le camouflage et la persistance de l'infrastructure, nous proposons l'évolution architecturale suivante :

*   **Mise en place d'une architecture tampon (VIP et relais multiples) :** Le serveur principal ne sera jamais exposé directement. Le client enverra ses requêtes vers un point d'entrée unique de type VIP (Virtual IP) ou Répartiteur de charge. Cette VIP se chargera de redistribuer le trafic vers l'un des serveurs tampons disponibles en frontal, qui relaiera enfin la requête au serveur principal situé dans une zone masquée.

## 3. Conséquences

### Impacts positifs
*   **Furtivité renforcée (OpSec) :** L'identité et la localisation du serveur principal demeurent totalement invisibles pour le client compromis et les éventuelles solutions de défense[cite: 9].
*   **Haute disponibilité et résilience :** La présence de plusieurs serveurs tampons derrière une VIP permet d'assurer la survie des communications[cite: 9]. Si un relais tombe hors service (HS) ou est bloqué, la VIP redirige automatiquement et de façon transparente le trafic vers le serveur suivant.

### Impacts négatifs
*   **Complexité d'infrastructure :** Cette architecture tripartite nécessite de configurer, de déployer et de maintenir en condition opérationnelle des nœuds réseaux supplémentaires (répartiteur, relais multiples) au lieu d'un simple serveur autonome.

## Proposition 1 : utilisation d'un serveur tampon

### Topologie réseau
```mermaid
graph LR
    subgraph "Zone visible par le client"
        C[Client]
        T[Serveur Tampon]
    end

    subgraph "Zone masquée"
        P[Serveur Principal]
    end

    C -- "Requête HTTPS" --> T
    T -- "Redirection" --> P
    P -- "Réponse" --> T
    T -- "Réponse HTTPS" --> C
```
### Diagramme de séquence
```mermaid
sequenceDiagram
    participant C as Client
    participant T as Serveur Tampon
    participant P as Serveur Principal

    Note over C,T: Le client interagit uniquement avec le serveur tampon
    C->>+T: Requête HTTPS
    T->>+P: Transfert / Redirection de la requête
    P-->>-T: Traitement et réponse
    T-->>-C: Transfert de la réponse en HTTPS
```
## Proposition 2 : utilisation de plusieurs serveurs tampons avec basculement

### Topologie réseau
```mermaid
graph LR
    subgraph "Zone visible par le client"
        C[Client]
        T1[Serveur Tampon 1 - HS]
        T2[Serveur Tampon 2 - Actif]
    end

    subgraph "Zone masquée"
        P[Serveur Principal]
    end

    C -. "1. Requête HTTPS (Échec)" .-> T1
    C -- "2. Requête HTTPS (Succès)" --> T2
    
    T1 -.-x P
    T2 -- "Redirection" --> P
    
    P -- "Réponse" --> T2
    T2 -- "Réponse HTTPS" --> C

    style T1 fill:#f9dbdb,stroke:#e74c3c,stroke-width:2px,color:#a94442
    style T2 fill:#d4edda,stroke:#28a745,stroke-width:2px,color:#155724
```
### Diagramme de séquence
```mermaid
sequenceDiagram
    participant C as Client
    participant T1 as Serveur Tampon 1 (HS)
    participant T2 as Serveur Tampon 2
    participant P as Serveur Principal

    Note over C, T2: Le client dispose d'une liste de serveurs tampons (ex: T1, T2)
    
    C->>T1: Requête HTTPS (Tentative 1)
    Note over T1: Le serveur est éteint ou injoignable
    T1--xC: Timeout ou Erreur de connexion
    
    Note over C: Le client détecte l'échec et passe au serveur suivant
    
    C->>+T2: Requête HTTPS (Tentative 2)
    T2->>+P: Transfert / Redirection de la requête
    P-->>-T2: Traitement et réponse
    T2-->>-C: Transfert de la réponse en HTTPS
```
## Proposition 3: utilisation de serveurs tampons avec VIP

### Topologie réseau
```mermaid
graph LR
    subgraph "Zone visible par le client"
        C[Client]
        V((VIP / Répartiteur))
    end

    subgraph "Zone tampon"
        T1[Serveur Tampon 1]
        T2[Serveur Tampon 2]
    end

    subgraph "Zone masquée"
        P[Serveur Principal]
    end

    C -- "Requête HTTPS" --> V
    V -- "Redirection (Si T1 dispo)" --> T1
    V -. "Redirection (Si T1 HS)" .-> T2
    
    T1 -- "Transfert" --> P
    T2 -- "Transfert" --> P
    
    P -- "Réponses" --> T1
    P -. "Réponses" .-> T2
    
    T1 -- "Retour" --> V
    T2 -. "Retour" .-> V
    V -- "Réponse HTTPS" --> C

    style V fill:#e1bee7,stroke:#8e24aa,stroke-width:2px,color:#4a148c
```

### Diagramme de séquence
```mermaid
sequenceDiagram
    participant C as Client
    participant V as VIP (Répartiteur)
    participant T1 as Serveur Tampon 1
    participant T2 as Serveur Tampon 2
    participant P as Serveur Principal

    Note over C, V: Le client ne connaît que l'adresse de la VIP
    
    C->>+V: Requête HTTPS 1
    Note over V: La VIP sélectionne un serveur disponible (ex: T1)
    V->>+T1: Routage de la requête 1
    T1->>+P: Transfert au serveur principal
    P-->>-T1: Réponse 1
    T1-->>-V: Retour au répartiteur
    V-->>-C: Réponse HTTPS 1 au client

    %% Deuxième requête pour montrer la répartition
    C->>+V: Requête HTTPS 2
    Note over V: T1 est occupé ou HS, la VIP choisit T2
    V->>+T2: Routage de la requête 2
    T2->>+P: Transfert au serveur principal
    P-->>-T2: Réponse 2
    T2-->>-V: Retour au répartiteur
    V-->>-C: Réponse HTTPS 2 au client
```
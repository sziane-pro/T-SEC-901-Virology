# ADR-002 : Choix du protocole de communication C2 — HTTPS vs WebSocket

## Statut
**Accepté** — remplace la décision initiale (WebSocket).

## Contexte

**sOPown3d** est un outil C2 pédagogique (implant ↔ contrôleur). Ses qualités premières sont, dans l'ordre : **furtivité**, **résilience**, **efficacité**. Le cahier des charges impose :

- un tunnel via un **protocole existant** ;
- des **communications chiffrées**, idéalement **asymétriquement**, prouvables dans Wireshark ;
- la capacité à **couvrir ses traces** dans les logs réseau.

Le choix initial s'était porté sur **WebSocket**. Il est réévalué ici face à **HTTPS**.

## Décision

**Nous adoptons HTTPS (TLS + HTTP) avec beaconing à jitter et User-Agent personnalisable**, en remplacement de WebSocket.

## Comparaison HTTPS vs WebSocket

| Critère | WebSocket | HTTPS (retenu) |
|---|---|---|
| **Chiffrement** | `wss://` = TLS, mais handshake `Upgrade` distinctif | TLS natif (X.509 + chiffrement de session), preuve immédiate dans Wireshark |
| **Furtivité réseau** | Handshake `Upgrade: websocket` **rare** dans un LAN → signature exploitable (NGFW, Zeek, Suricata) | Trafic **indistinguable** d'une navigation web normale, noyé dans le bruit ambiant |
| **Logs** | Logs applicatifs isolables, faciles à corréler | Logs HTTP mélangés au bruit → couverture par **rollback ciblé** plutôt qu'effacement |
| **Traversée proxy/firewall** | Souvent bloqué ou non inspecté | Traverse les proxys d'entreprise, améliore la résilience |
| **Interactivité** | Full-duplex natif, faible latence | Requête/réponse → **beaconing + jitter** compense (latence acceptable pour un shell de test) |
| **PKI** | Même PKI + handshake applicatif supplémentaire | Certificat X.509 suffit (auto-signé pour le lab) |

## Justifications

1. **Chiffrement asymétrique natif et démontrable** — TLS combine échange de clés asymétrique et chiffrement symétrique de session ; le handshake est visible dans Wireshark, le contenu illisible. Satisfait directement l'exigence du sujet.

2. **Camouflage dans le bruit ambiant** — HTTPS/443 est le trafic dominant de tout réseau d'entreprise. Le flux WebSocket pur est rare et donc détectable. La furtivité étant la qualité première, HTTPS l'emporte.

3. **Beaconing à jitter** — testé en POC HTTP. Le jitter casse la périodicité détectable par analyse comportementale (NDR) et simule un trafic applicatif légitime.

4. **User-Agent personnalisable** — les headers HTTP permettent d'aligner l'implant sur un client légitime (navigateur, agent interne), renforçant la discrétion côté logs applicatifs et proxy.

5. **Couverture des traces** — logs HTTP volumineux → rollback de lignes ciblées plutôt qu'effacement total (exigence explicite).

6. **Résilience** — meilleure traversée des proxys et politiques de filtrage sortant.

## Compromis assumé

- **Latence** : HTTP est requête/réponse → shell moins instantané que WebSocket. Compensé par beaconing jitteré.
- **Gestion de session** : token/cookie opaque côté contrôleur (reste dans le profil d'un serveur web légitime).
- **Certificat auto-signé** : peut être flaggé par une inspection TLS stricte → documenté comme **faiblesse connue** (étape 1 du sujet : outil connu ; étape 2 : implémentation plus furtive, ex. certificat CA interne).

## Conséquences

### Positives
- Chiffrement asymétrique **natif et démontrable** (Wireshark).
- Trafic **indistinguable** d'une navigation HTTPS ; logs noyés dans le bruit.
- **User-Agent personnalisable** → alignement sur un client légitime.
- Meilleure **traversée des proxys/firewalls**.
- Mapping MITRE ATT&CK : **T1071.001** (Web Protocols) + **T1573.002** (Asymmetric Cryptography).

### Négatives / risques
- Latence supérieure → shell moins « instantané » (acceptable en lab).
- Beaconing périodique détectable par analyse comportementale → **atténué par jitter**.
- Certificat auto-signé flaggable en inspection TLS stricte → faiblesse connue documentée.

## Alternatives rejetées
- **WebSocket** : handshake `Upgrade` trop marqué, logs applicatifs isolables, pas d'avantage de chiffrement (wss = TLS aussi), traversée proxy moins fiable.

## Références
- RFC 6455 (WebSocket), RFC 8446 (TLS 1.3), RFC 9110 (HTTP Semantics).
- MITRE ATT&CK : T1071.001, T1573.002.
- Cahier des charges *Virology — sOPown3d*.
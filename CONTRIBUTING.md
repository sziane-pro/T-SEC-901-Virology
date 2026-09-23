# Contribution

## Branches

- `main` contient la version de production et de démonstration.
- `dev` contient l'intégration en cours.
- Les branches de travail suivent le format `<type>/<description-courte>` :
  `feat/`, `fix/`, `docs/`, `refactor/`, `test/` ou `chore/`.
- Toute modification est intégrée dans `main` par pull request depuis `dev`.

## Commits

Les messages suivent [Conventional Commits](https://www.conventionalcommits.org/) :

```text
<type>(<issues>): <description>
```

Types courants : `feat`, `fix`, `docs`, `refactor`, `test`, `chore` et `ci`.

Exemple :

```text
feat(T-002): add file signature detection
```
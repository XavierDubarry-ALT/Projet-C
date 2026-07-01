# Projet Dictionnaire — FIPA28 ENSTA

Projet C réalisé par **Julien Fischer** et **Xavier Dubarry**.

Le projet se compose de deux parties :

- **libdico** : une bibliothèque implémentant un dictionnaire (table de hachage) en C, avec un programme de benchmark.
- **wc** : un programme de comptage de mots/lignes/caractères (inspiré de `wc` Unix), utilisant `libdico` pour compter les mots uniques.

---

## Structure

```
projet_dico/
├── Makefile
├── libdico/
│   ├── Makefile
│   ├── src/
│   │   ├── dico.h        # API du dictionnaire
│   │   ├── dico.c        # Implémentation (table de hachage)
│   │   └── benchmark.c   # Programme de benchmark
│   ├── lib/              # libdico.so (généré)
│   ├── bin/              # bench_dico (généré)
│   └── mots_17479.txt    # Dataset de mots pour le benchmark
└── wc/
    ├── Makefile
    ├── src/
    │   ├── main.c
    │   ├── wc.h
    │   └── wc_core.c
    ├── bin/              # WordCount (généré)
    └── VHUGO/            # Textes de Victor Hugo pour les tests
```

---

## Compilation

### Tout compiler (libdico + wc) depuis la racine

```bash
make
```

### Compiler uniquement libdico

```bash
cd libdico
make
```

Produit :
- `libdico/lib/libdico.so` — bibliothèque partagée
- `libdico/bin/bench_dico` — programme de benchmark

### Compiler uniquement wc

```bash
cd wc
make
```

> **Attention :** `libdico` doit être compilé en premier (la bibliothèque partagée est nécessaire).

Produit :
- `wc/bin/WordCount` — programme de comptage

### Nettoyer les fichiers générés

```bash
make clean
```

---

## Lancer le benchmark (libdico)

```bash
./libdico/bin/bench_dico
```

Le programme charge automatiquement `libdico/mots_17479.txt` et mesure les performances de la table de hachage :

- Temps d'insertion de tous les mots
- Temps de recherche positive (mots existants)
- Temps de recherche négative (mots absents)
- Facteur de charge de la table

---

## Lancer WordCount

```bash
LD_LIBRARY_PATH=libdico/lib ./wc/bin/WordCount [OPTIONS] [fichier...]
```

Si la bibliothèque est déjà installée dans `/usr/lib`, la variable `LD_LIBRARY_PATH` n'est pas nécessaire.

### Options

| Option | Description |
|--------|-------------|
| `-l`   | Affiche le nombre de **lignes** |
| `-w`   | Affiche le nombre de **mots** |
| `-c`   | Affiche le nombre de **caractères** |
| `-S`   | Affiche le **dictionnaire complet** des mots (fréquence de chaque mot) ainsi que le nombre de mots uniques |

Sans option, le comportement par défaut est équivalent à `-lwc` (lignes + mots + caractères).

### Exemples

```bash
# Compter lignes, mots et caractères d'un fichier
LD_LIBRARY_PATH=libdico/lib ./wc/bin/WordCount -lwc wc/VHUGO/pg6838.txt

# Lister tous les mots uniques et leur fréquence
LD_LIBRARY_PATH=libdico/lib ./wc/bin/WordCount -S wc/VHUGO/pg6838.txt

# Traiter plusieurs fichiers (affiche un total)
LD_LIBRARY_PATH=libdico/lib ./wc/bin/WordCount -w wc/VHUGO/pg6838.txt wc/VHUGO/pg8186.txt

# Depuis l'entrée standard
echo "On est pas contre une bonne note" | LD_LIBRARY_PATH=libdico/lib ./wc/bin/WordCount -w
```
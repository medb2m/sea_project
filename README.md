# File Processing Manager - Mono vs Multi-thread

Projet de comparaison entre traitement mono-thread et multi-thread en C avec interface web.

## 📋 Description

Ce projet démontre les avantages et défis du multi-threading en comparant deux implémentations d'un système de traitement de fichiers :
- **Mono-thread** : Traitement séquentiel sur un seul thread
- **Multi-thread** : Traitement parallèle avec plusieurs threads (pthread)

## 🚀 Démarrage rapide

### 1. Installation des dépendances

```bash
sudo apt-get update
sudo apt-get install -y build-essential python3-flask python3-matplotlib python3-psutil python3-numpy
```

### 2. Compilation

```bash
cd /root/sea_projects
make
```

### 3. Génération des fichiers de test

```bash
./generate_test_files.sh
```

### 4. Démarrage du serveur web

```bash
./start_server.sh
```

Puis accédez à `http://VPS_IP:4321` dans votre navigateur.

## 📝 Utilisation en ligne de commande

**Mono-thread :**
```bash
./mono input output
```

**Multi-thread :**
```bash
./multi input output 4 lock    # 4 threads avec mutex
./multi input output 4 nolock   # 4 threads sans mutex (démo bug)
```

**Benchmark :**
```bash
./benchmark 5 4  # 5 exécutions, 4 threads pour multi
```

## 🔧 Configuration du firewall

Le port 4321 doit être ouvert :

```bash
sudo ufw allow 4321/tcp
# ou
sudo iptables -A INPUT -p tcp --dport 4321 -j ACCEPT
```

## 📊 Fonctionnalités

- Traitement de fichiers avec transformation XOR
- Multi-threading avec pthread
- Synchronisation avec mutex (mode lock)
- Démonstration de race conditions (mode nolock)
- Benchmark avec statistiques
- Interface web interactive
- Graphiques de performance

## 🎥 Démo vidéo suggérée

1. Montrer 100 fichiers dans `input/`
2. Lancer mono → lent (afficher le temps)
3. Lancer multi → rapide (comparer le temps)
4. Ouvrir Task Manager / htop (CPU usage)
5. Afficher graphique speedup
6. Montrer bug sans lock (compteurs incorrects)
7. Conclusion sur les avantages et défis

## 📚 Structure du projet

```
sea_projects/
├── input/              # Fichiers d'entrée
├── output/             # Fichiers de sortie
├── static/             # Graphiques générés
├── templates/          # Templates HTML
├── mono.c              # Version mono-thread
├── multi.c             # Version multi-thread
├── benchmark.c         # Script de benchmark
├── graphs.py           # Génération de graphiques
├── web_server.py       # Serveur web Flask
├── Makefile            # Compilation
├── generate_test_files.sh  # Génération de fichiers de test
├── start_server.sh     # Script de démarrage
└── README.md           # Ce fichier
```

## 🐛 Démonstration des bugs

Le projet inclut une démonstration de race conditions :
- Mode `lock` : Utilise des mutex pour éviter les conflits
- Mode `nolock` : Montre les problèmes de synchronisation (compteurs incorrects)

## 📄 Licence

Ce projet est fourni à des fins éducatives.

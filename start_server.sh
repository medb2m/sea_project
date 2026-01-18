#!/bin/bash
# Script pour démarrer le serveur web

cd /root/sea_projects

echo "Vérification des dépendances..."
python3 << 'PYEOF'
import sys
critical_missing = []
warnings = []

# Dépendances critiques (requises pour le serveur)
try:
    import flask
except ImportError:
    critical_missing.append("flask")

try:
    import psutil
except ImportError:
    critical_missing.append("psutil")

# Dépendances optionnelles (pour les graphiques)
try:
    import matplotlib
except ImportError as e:
    warnings.append(f"matplotlib (graphiques désactivés): {e}")

try:
    import numpy
except ImportError:
    warnings.append("numpy (peut affecter matplotlib)")

if critical_missing:
    print(f"ERREUR: Dépendances critiques manquantes: {', '.join(critical_missing)}")
    print("Installez avec: sudo apt-get install python3-flask python3-psutil")
    sys.exit(1)

if warnings:
    print("AVERTISSEMENTS:")
    for w in warnings:
        print(f"  - {w}")
    print("Le serveur démarrera mais certaines fonctionnalités peuvent être limitées.")
    print("")

sys.exit(0)
PYEOF

if [ $? -ne 0 ]; then
    exit 1
fi

echo "Vérification des exécutables..."
if [ ! -f "./mono" ] || [ ! -f "./multi" ]; then
    echo "Compilation des programmes C..."
    make
fi

echo ""
echo "=========================================="
echo "🚀 Démarrage du serveur web"
echo "=========================================="
echo "Interface accessible sur: http://VPS_IP:4321"
echo "Appuyez sur Ctrl+C pour arrêter"
echo "=========================================="
echo ""

python3 web_server.py

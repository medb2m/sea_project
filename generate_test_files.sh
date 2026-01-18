#!/bin/bash
# Script pour générer des fichiers de test

echo "Génération de fichiers de test dans input/..."

# Créer le répertoire input s'il n'existe pas
mkdir -p input

# Nettoyer les anciens fichiers
rm -f input/*

# Générer 100 fichiers de différentes tailles
for i in {1..100}; do
    # Tailles variées : 100KB à 2MB
    size=$((100 + (i % 20) * 100))
    dd if=/dev/urandom of=input/file_$(printf "%03d" $i).dat bs=1K count=$size 2>/dev/null
    echo -n "."
done

echo ""
echo "✅ 100 fichiers générés dans input/"
ls -lh input/ | head -10

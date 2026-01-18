#!/usr/bin/env python3
"""
Script pour générer des graphiques de performance
"""

import os
import sys

# Essayer d'importer matplotlib avec gestion d'erreur
try:
    import matplotlib
    matplotlib.use('Agg')  # Backend non-interactif pour serveur
    import matplotlib.pyplot as plt
    MATPLOTLIB_AVAILABLE = True
except ImportError as e:
    MATPLOTLIB_AVAILABLE = False
    print(f"AVERTISSEMENT: matplotlib non disponible - {e}", file=sys.stderr)
    print("Les graphiques ne pourront pas être générés.", file=sys.stderr)

try:
    import numpy as np
except ImportError:
    np = None

import json

def read_benchmark_results():
    """Lire les résultats du benchmark depuis le fichier"""
    results = {
        'mono': {'mean': 0, 'std': 0},
        'multi': {'mean': 0, 'std': 0},
        'speedup': 0
    }
    
    try:
        with open('benchmark_results.txt', 'r') as f:
            for line in f:
                parts = line.strip().split(',')
                if parts[0] == 'mono':
                    results['mono']['mean'] = float(parts[1])
                    results['mono']['std'] = float(parts[2])
                elif parts[0] == 'multi':
                    results['multi']['mean'] = float(parts[1])
                    results['multi']['std'] = float(parts[2])
                elif parts[0] == 'speedup':
                    results['speedup'] = float(parts[1])
    except FileNotFoundError:
        pass
    
    return results

def generate_comparison_chart():
    """Générer un graphique de comparaison mono vs multi"""
    if not MATPLOTLIB_AVAILABLE:
        return None
        
    results = read_benchmark_results()
    
    if results['mono']['mean'] == 0:
        return None
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    categories = ['Mono-thread', 'Multi-thread']
    means = [results['mono']['mean'], results['multi']['mean']]
    stds = [results['mono']['std'], results['multi']['std']]
    
    bars = ax.bar(categories, means, yerr=stds, capsize=10, 
                  color=['#ff6b6b', '#4ecdc4'], alpha=0.7, edgecolor='black')
    
    ax.set_ylabel('Temps (secondes)', fontsize=12)
    ax.set_title('Comparaison Mono-thread vs Multi-thread', fontsize=14, fontweight='bold')
    ax.grid(axis='y', alpha=0.3)
    
    # Ajouter les valeurs sur les barres
    for i, (mean, std) in enumerate(zip(means, stds)):
        ax.text(i, mean + std + 0.1, f'{mean:.3f}s', 
                ha='center', va='bottom', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig('static/comparison.png', dpi=150, bbox_inches='tight')
    plt.close()
    
    return 'static/comparison.png'

def generate_speedup_chart():
    """Générer un graphique de speedup"""
    if not MATPLOTLIB_AVAILABLE:
        return None
        
    results = read_benchmark_results()
    
    if results['speedup'] == 0:
        return None
    
    fig, ax = plt.subplots(figsize=(8, 6))
    
    speedup = results['speedup']
    ax.bar(['Speedup'], [speedup], color='#95e1d3', alpha=0.7, edgecolor='black')
    ax.axhline(y=1, color='r', linestyle='--', label='Référence (1x)')
    ax.set_ylabel('Speedup (x)', fontsize=12)
    ax.set_title(f'Speedup: {speedup:.2f}x', fontsize=14, fontweight='bold')
    ax.grid(axis='y', alpha=0.3)
    ax.legend()
    
    # Ajouter la valeur
    ax.text(0, speedup + 0.1, f'{speedup:.2f}x', 
            ha='center', va='bottom', fontweight='bold', fontsize=14)
    
    plt.tight_layout()
    plt.savefig('static/speedup.png', dpi=150, bbox_inches='tight')
    plt.close()
    
    return 'static/speedup.png'

def generate_scalability_chart(num_threads_list, times_list):
    """Générer un graphique de scalabilité"""
    if not MATPLOTLIB_AVAILABLE:
        return None
        
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(num_threads_list, times_list, marker='o', linewidth=2, 
            markersize=8, color='#4ecdc4', label='Temps d\'exécution')
    ax.set_xlabel('Nombre de threads', fontsize=12)
    ax.set_ylabel('Temps (secondes)', fontsize=12)
    ax.set_title('Scalabilité Multi-thread', fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend()
    
    # Ajouter les valeurs
    for x, y in zip(num_threads_list, times_list):
        ax.text(x, y + 0.05, f'{y:.2f}s', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig('static/scalability.png', dpi=150, bbox_inches='tight')
    plt.close()
    
    return 'static/scalability.png'

if __name__ == '__main__':
    # Créer le dossier static s'il n'existe pas
    os.makedirs('static', exist_ok=True)
    
    if not MATPLOTLIB_AVAILABLE:
        print("ERREUR: matplotlib n'est pas disponible. Les graphiques ne peuvent pas être générés.")
        sys.exit(1)
    
    generate_comparison_chart()
    generate_speedup_chart()
    print("Graphiques générés avec succès!")

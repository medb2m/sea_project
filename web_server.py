#!/usr/bin/env python3
"""
Serveur web Flask pour l'interface graphique du File Processing Manager
"""

from flask import Flask, render_template, jsonify, request, send_from_directory
import subprocess
import os
import json
import time
import threading
import psutil
import glob

app = Flask(__name__)
app.config['SECRET_KEY'] = 'file-processing-manager-secret-key'

# Variables globales pour le suivi des processus
current_process = None
process_lock = threading.Lock()

def get_system_info():
    """Obtenir les informations système"""
    return {
        'cpu_count': psutil.cpu_count(),
        'cpu_percent': psutil.cpu_percent(interval=0.1),
        'memory_percent': psutil.virtual_memory().percent,
        'memory_available': psutil.virtual_memory().available / (1024**3)  # GB
    }

def count_files(directory):
    """Compter les fichiers dans un répertoire"""
    try:
        return len([f for f in os.listdir(directory) 
                   if os.path.isfile(os.path.join(directory, f))])
    except:
        return 0

@app.route('/')
def index():
    """Page principale"""
    return render_template('index.html')

@app.route('/api/system')
def api_system():
    """API pour obtenir les informations système"""
    return jsonify(get_system_info())

@app.route('/api/files')
def api_files():
    """API pour obtenir le nombre de fichiers"""
    return jsonify({
        'input': count_files('input'),
        'output': count_files('output')
    })

@app.route('/api/run/mono')
def api_run_mono():
    """Exécuter le traitement mono-thread"""
    global current_process
    
    with process_lock:
        if current_process and current_process.poll() is None:
            return jsonify({'error': 'Un processus est déjà en cours'}), 400
        
        try:
            current_process = subprocess.Popen(
                ['./mono', 'input', 'output'],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            return jsonify({'success': True, 'pid': current_process.pid})
        except Exception as e:
            return jsonify({'error': str(e)}), 500

@app.route('/api/run/multi')
def api_run_multi():
    """Exécuter le traitement multi-thread"""
    global current_process
    
    num_threads = request.args.get('threads', default=4, type=int)
    use_lock = request.args.get('lock', default='true', type=str) == 'true'
    
    with process_lock:
        if current_process and current_process.poll() is None:
            return jsonify({'error': 'Un processus est déjà en cours'}), 400
        
        try:
            lock_arg = 'lock' if use_lock else 'nolock'
            current_process = subprocess.Popen(
                ['./multi', 'input', 'output', str(num_threads), lock_arg],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            return jsonify({'success': True, 'pid': current_process.pid})
        except Exception as e:
            return jsonify({'error': str(e)}), 500

@app.route('/api/run/benchmark')
def api_run_benchmark():
    """Exécuter le benchmark"""
    global current_process
    
    num_runs = request.args.get('runs', default=5, type=int)
    num_threads = request.args.get('threads', default=4, type=int)
    
    with process_lock:
        if current_process and current_process.poll() is None:
            return jsonify({'error': 'Un processus est déjà en cours'}), 400
        
        try:
            current_process = subprocess.Popen(
                ['./benchmark', str(num_runs), str(num_threads)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            return jsonify({'success': True, 'pid': current_process.pid})
        except Exception as e:
            return jsonify({'error': str(e)}), 500

@app.route('/api/status')
def api_status():
    """Obtenir le statut du processus en cours"""
    global current_process
    
    with process_lock:
        if current_process is None:
            return jsonify({'running': False})
        
        is_running = current_process.poll() is None
        
        result = {
            'running': is_running,
            'pid': current_process.pid if current_process else None
        }
        
        # Si le processus est en cours, essayer de lire la sortie en temps réel
        if is_running:
            try:
                # Lire la dernière ligne de sortie sans bloquer
                import select
                import fcntl
                
                # Configurer le stdout en mode non-bloquant
                flags = fcntl.fcntl(current_process.stdout, fcntl.F_GETFL)
                fcntl.fcntl(current_process.stdout, fcntl.F_SETFL, flags | os.O_NONBLOCK)
                
                # Lire les dernières lignes
                try:
                    lines = current_process.stdout.readlines()
                    if lines:
                        result['progress'] = lines[-1].strip() if lines else ''
                except:
                    pass
            except:
                pass
        
        if not is_running:
            # Récupérer la sortie
            stdout, stderr = current_process.communicate()
            result['stdout'] = stdout
            result['stderr'] = stderr
            result['returncode'] = current_process.returncode
            
            # Lire les résultats
            try:
                if os.path.exists('mono_results.txt'):
                    with open('mono_results.txt', 'r') as f:
                        lines = f.readlines()
                        result['mono_results'] = {
                            'time': float(lines[0].strip()),
                            'files': int(lines[1].strip()),
                            'bytes': int(lines[2].strip())
                        }
            except:
                pass
            
            try:
                if os.path.exists('multi_results.txt'):
                    with open('multi_results.txt', 'r') as f:
                        lines = f.readlines()
                        result['multi_results'] = {
                            'time': float(lines[0].strip()),
                            'files': int(lines[1].strip()),
                            'bytes': int(lines[2].strip()),
                            'threads': int(lines[3].strip()) if len(lines) > 3 else 0
                        }
            except:
                pass
        
        return jsonify(result)

@app.route('/api/generate-graphs')
def api_generate_graphs():
    """Générer les graphiques"""
    try:
        subprocess.run(['python3', 'graphs.py'], check=True, capture_output=True)
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/static/<path:filename>')
def static_files(filename):
    """Servir les fichiers statiques"""
    return send_from_directory('static', filename)

if __name__ == '__main__':
    # Créer les dossiers nécessaires
    os.makedirs('static', exist_ok=True)
    os.makedirs('input', exist_ok=True)
    os.makedirs('output', exist_ok=True)
    
    # Vérifier que les exécutables existent
    if not os.path.exists('./mono') or not os.path.exists('./multi'):
        print("ATTENTION: Les exécutables mono et multi n'existent pas.")
        print("Exécutez 'make' pour les compiler.")
    
    print("Démarrage du serveur web sur http://0.0.0.0:4321")
    print("Accédez à l'interface depuis votre navigateur")
    app.run(host='0.0.0.0', port=4321, debug=False)

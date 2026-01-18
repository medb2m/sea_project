#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_PATH 512
#define BUFFER_SIZE 8192
#define MAX_THREADS 8

// Structure pour les arguments des threads
typedef struct {
    char input_path[MAX_PATH];
    char output_path[MAX_PATH];
    int thread_id;
} ThreadArgs;

// Structure pour les statistiques globales
typedef struct {
    int files_processed;
    long total_bytes;
    pthread_mutex_t mutex;
    bool use_lock; // Pour démontrer les bugs sans lock
} GlobalStats;

GlobalStats global_stats;

// Fonction pour traiter un fichier (identique à mono.c)
void process_file(const char* input_path, const char* output_path) {
    FILE* input = fopen(input_path, "rb");
    FILE* output = fopen(output_path, "wb");
    
    if (!input || !output) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir les fichiers %s ou %s\n", input_path, output_path);
        return;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, input)) > 0) {
        // Simulation de traitement
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] = buffer[i] ^ 0xAA;
        }
        
        // Simulation de calcul intensif
        volatile int dummy = 0;
        for (int j = 0; j < 1000; j++) {
            dummy += j;
        }
        
        fwrite(buffer, 1, bytes_read, output);
    }
    
    fclose(input);
    fclose(output);
}

// Fonction exécutée par chaque thread
void* thread_worker(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    struct stat st;
    if (stat(args->input_path, &st) == 0 && S_ISREG(st.st_mode)) {
        process_file(args->input_path, args->output_path);
        
        // Mise à jour des statistiques (avec ou sans lock selon le mode)
        if (global_stats.use_lock) {
            pthread_mutex_lock(&global_stats.mutex);
            global_stats.files_processed++;
            global_stats.total_bytes += st.st_size;
            pthread_mutex_unlock(&global_stats.mutex);
        } else {
            // VERSION BUGGÉE: Race condition intentionnelle pour démo
            global_stats.files_processed++;
            global_stats.total_bytes += st.st_size;
        }
        
        printf("Thread %d: Traité %s (%ld octets)\n", 
               args->thread_id, args->input_path, st.st_size);
    }
    
    free(args);
    return NULL;
}

// Traitement multi-thread
double process_files_multi(const char* input_dir, const char* output_dir, 
                          int num_threads, bool use_lock) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Initialiser les statistiques globales
    global_stats.files_processed = 0;
    global_stats.total_bytes = 0;
    global_stats.use_lock = use_lock;
    pthread_mutex_init(&global_stats.mutex, NULL);
    
    // Collecter tous les fichiers
    char** file_paths = malloc(MAX_PATH * sizeof(char*));
    char** output_paths = malloc(MAX_PATH * sizeof(char*));
    int file_count = 0;
    
    DIR* dir = opendir(input_dir);
    if (!dir) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le répertoire %s\n", input_dir);
        return 0.0;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && file_count < 100) {
        if (entry->d_name[0] == '.') continue;
        
        char input_path[MAX_PATH];
        snprintf(input_path, MAX_PATH, "%s/%s", input_dir, entry->d_name);
        
        struct stat st;
        if (stat(input_path, &st) == 0 && S_ISREG(st.st_mode)) {
            file_paths[file_count] = malloc(MAX_PATH);
            output_paths[file_count] = malloc(MAX_PATH);
            strcpy(file_paths[file_count], input_path);
            snprintf(output_paths[file_count], MAX_PATH, "%s/%s", output_dir, entry->d_name);
            file_count++;
        }
    }
    closedir(dir);
    
    // Créer les threads
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    int file_index = 0;
    
    // Lancer les threads
    while (file_index < file_count) {
        // Attendre qu'un thread se libère si on a atteint le max
        if (thread_count >= num_threads) {
            // Attendre le premier thread disponible
            pthread_join(threads[0], NULL);
            // Décaler les threads
            for (int i = 0; i < thread_count - 1; i++) {
                threads[i] = threads[i + 1];
            }
            thread_count--;
        }
        
        // Créer un nouveau thread
        ThreadArgs* args = malloc(sizeof(ThreadArgs));
        strcpy(args->input_path, file_paths[file_index]);
        strcpy(args->output_path, output_paths[file_index]);
        args->thread_id = thread_count;
        
        pthread_create(&threads[thread_count], NULL, thread_worker, args);
        thread_count++;
        file_index++;
    }
    
    // Attendre que tous les threads se terminent
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Libérer la mémoire
    for (int i = 0; i < file_count; i++) {
        free(file_paths[i]);
        free(output_paths[i]);
    }
    free(file_paths);
    free(output_paths);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double processing_time = (end.tv_sec - start.tv_sec) + 
                            (end.tv_nsec - start.tv_nsec) / 1e9;
    
    pthread_mutex_destroy(&global_stats.mutex);
    
    return processing_time;
}

int main(int argc, char* argv[]) {
    const char* input_dir = (argc > 1) ? argv[1] : "input";
    const char* output_dir = (argc > 2) ? argv[2] : "output";
    int num_threads = (argc > 3) ? atoi(argv[3]) : 4;
    bool use_lock = (argc > 4) ? (strcmp(argv[4], "lock") == 0) : true;
    
    if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    if (num_threads < 1) num_threads = 1;
    
    printf("=== Traitement Multi-thread ===\n");
    printf("Répertoire d'entrée: %s\n", input_dir);
    printf("Répertoire de sortie: %s\n", output_dir);
    printf("Nombre de threads: %d\n", num_threads);
    printf("Utilisation de mutex: %s\n", use_lock ? "Oui" : "Non (BUG DÉMONSTRATIF)");
    printf("Démarrage du traitement...\n\n");
    
    double processing_time = process_files_multi(input_dir, output_dir, num_threads, use_lock);
    
    printf("\n=== Résultats ===\n");
    printf("Fichiers traités: %d\n", global_stats.files_processed);
    printf("Octets traités: %ld\n", global_stats.total_bytes);
    printf("Temps de traitement: %.3f secondes\n", processing_time);
    printf("Débit: %.2f Mo/s\n", (global_stats.total_bytes / 1024.0 / 1024.0) / processing_time);
    
    // Écrire les résultats dans un fichier pour le benchmark
    FILE* result_file = fopen("multi_results.txt", "w");
    if (result_file) {
        fprintf(result_file, "%.3f\n%d\n%ld\n%d\n", processing_time, 
                global_stats.files_processed, global_stats.total_bytes, num_threads);
        fclose(result_file);
    }
    
    return 0;
}

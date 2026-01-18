#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PATH 512
#define BUFFER_SIZE 8192

typedef struct {
    int files_processed;
    long total_bytes;
    double processing_time;
} Stats;

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
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] = buffer[i] ^ 0xAA;
        }
        
        volatile int dummy = 0;
        for (int j = 0; j < 1000; j++) {
            dummy += j;
        }
        
        fwrite(buffer, 1, bytes_read, output);
    }
    
    fclose(input);
    fclose(output);
    
    // Petit délai pour simuler traitement séquentiel (mono-thread seulement)
    usleep(20000);  // 5ms de délai par fichier
}

Stats process_files_mono(const char* input_dir, const char* output_dir) {
    Stats stats = {0, 0, 0.0};
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    DIR* dir = opendir(input_dir);
    if (!dir) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le répertoire %s\n", input_dir);
        return stats;
    }
    
    struct dirent* entry;
    char input_path[MAX_PATH];
    char output_path[MAX_PATH];
    
    int file_num = 0;
    int total_files = 0;
    
    // Compter d'abord le nombre total de fichiers
    struct dirent* entry_count;
    rewinddir(dir);
    while ((entry_count = readdir(dir)) != NULL) {
        if (entry_count->d_name[0] == '.') continue;
        char temp_path[MAX_PATH];
        snprintf(temp_path, MAX_PATH, "%s/%s", input_dir, entry_count->d_name);
        struct stat st;
        if (stat(temp_path, &st) == 0 && S_ISREG(st.st_mode)) {
            total_files++;
        }
    }
    rewinddir(dir);
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        snprintf(input_path, MAX_PATH, "%s/%s", input_dir, entry->d_name);
        snprintf(output_path, MAX_PATH, "%s/%s", output_dir, entry->d_name);
        
        struct stat st;
        if (stat(input_path, &st) == 0 && S_ISREG(st.st_mode)) {
            file_num++;
            printf("Traitement fichier %d/%d: %s (%.2f Mo)...\r", 
                   file_num, total_files, entry->d_name, st.st_size / 1024.0 / 1024.0);
            fflush(stdout);
            
            process_file(input_path, output_path);
            stats.files_processed++;
            stats.total_bytes += st.st_size;
        }
    }
    printf("\n");
    
    closedir(dir);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    stats.processing_time = (end.tv_sec - start.tv_sec) + 
                           (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return stats;
}

int main(int argc, char* argv[]) {
    const char* input_dir = (argc > 1) ? argv[1] : "input";
    const char* output_dir = (argc > 2) ? argv[2] : "output";
    
    printf("=== Traitement Mono-thread ===\n");
    printf("Répertoire d'entrée: %s\n", input_dir);
    printf("Répertoire de sortie: %s\n", output_dir);
    printf("Démarrage du traitement...\n\n");
    
    Stats stats = process_files_mono(input_dir, output_dir);
    
    printf("=== Résultats ===\n");
    printf("Fichiers traités: %d\n", stats.files_processed);
    printf("Octets traités: %ld\n", stats.total_bytes);
    printf("Temps de traitement: %.3f secondes\n", stats.processing_time);
    printf("Débit: %.2f Mo/s\n", (stats.total_bytes / 1024.0 / 1024.0) / stats.processing_time);
    
    FILE* result_file = fopen("mono_results.txt", "w");
    if (result_file) {
        fprintf(result_file, "%.3f\n%d\n%ld\n", stats.processing_time, 
                stats.files_processed, stats.total_bytes);
        fclose(result_file);
    }
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define MAX_RUNS 10

// Structure pour les résultats de benchmark
typedef struct {
    double times[MAX_RUNS];
    int runs;
    double mean;
    double std_dev;
    int files_processed;
    long total_bytes;
} BenchmarkResult;

// Calculer la moyenne
double calculate_mean(double* values, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += values[i];
    }
    return sum / n;
}

// Calculer l'écart-type
double calculate_std_dev(double* values, int n, double mean) {
    double sum_sq_diff = 0.0;
    for (int i = 0; i < n; i++) {
        double diff = values[i] - mean;
        sum_sq_diff += diff * diff;
    }
    return sqrt(sum_sq_diff / n);
}

// Lire les résultats depuis un fichier
int read_results(const char* filename, BenchmarkResult* result) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    double time;
    if (fscanf(file, "%lf", &time) == 1) {
        result->times[result->runs] = time;
        fscanf(file, "%d", &result->files_processed);
        fscanf(file, "%ld", &result->total_bytes);
        result->runs++;
        fclose(file);
        return 1;
    }
    
    fclose(file);
    return 0;
}

int main(int argc, char* argv[]) {
    int num_runs = (argc > 1) ? atoi(argv[1]) : 5;
    int num_threads = (argc > 2) ? atoi(argv[2]) : 4;
    
    if (num_runs > MAX_RUNS) num_runs = MAX_RUNS;
    
    printf("=== Benchmark: Mono-thread vs Multi-thread ===\n");
    printf("Nombre d'exécutions: %d\n", num_runs);
    printf("Threads pour multi: %d\n\n", num_threads);
    
    BenchmarkResult mono_result = {0};
    BenchmarkResult multi_result = {0};
    
    // Exécuter les benchmarks
    printf("Exécution des benchmarks mono-thread...\n");
    for (int i = 0; i < num_runs; i++) {
        printf("Run %d/%d...\n", i + 1, num_runs);
        system("./mono input output > /dev/null 2>&1");
        read_results("mono_results.txt", &mono_result);
    }
    
    printf("\nExécution des benchmarks multi-thread...\n");
    for (int i = 0; i < num_runs; i++) {
        printf("Run %d/%d...\n", i + 1, num_runs);
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./multi input output %d lock > /dev/null 2>&1", num_threads);
        system(cmd);
        read_results("multi_results.txt", &multi_result);
    }
    
    // Calculer les statistiques
    mono_result.mean = calculate_mean(mono_result.times, mono_result.runs);
    mono_result.std_dev = calculate_std_dev(mono_result.times, mono_result.runs, mono_result.mean);
    
    multi_result.mean = calculate_mean(multi_result.times, multi_result.runs);
    multi_result.std_dev = calculate_std_dev(multi_result.times, multi_result.runs, multi_result.mean);
    
    // Afficher les résultats
    printf("\n=== Résultats du Benchmark ===\n\n");
    
    printf("Mono-thread:\n");
    printf("  Temps moyen: %.3f ± %.3f secondes\n", mono_result.mean, mono_result.std_dev);
    printf("  Fichiers traités: %d\n", mono_result.files_processed);
    printf("  Octets traités: %ld\n", mono_result.total_bytes);
    printf("  Débit moyen: %.2f Mo/s\n\n", 
           (mono_result.total_bytes / 1024.0 / 1024.0) / mono_result.mean);
    
    printf("Multi-thread (%d threads):\n", num_threads);
    printf("  Temps moyen: %.3f ± %.3f secondes\n", multi_result.mean, multi_result.std_dev);
    printf("  Fichiers traités: %d\n", multi_result.files_processed);
    printf("  Octets traités: %ld\n", multi_result.total_bytes);
    printf("  Débit moyen: %.2f Mo/s\n\n", 
           (multi_result.total_bytes / 1024.0 / 1024.0) / multi_result.mean);
    
    double speedup = mono_result.mean / multi_result.mean;
    printf("Speedup: %.2fx\n", speedup);
    printf("Efficacité: %.1f%%\n", (speedup / num_threads) * 100);
    
    // Écrire les résultats dans un fichier pour les graphiques
    FILE* bench_file = fopen("benchmark_results.txt", "w");
    if (bench_file) {
        fprintf(bench_file, "mono,%.3f,%.3f\n", mono_result.mean, mono_result.std_dev);
        fprintf(bench_file, "multi,%.3f,%.3f\n", multi_result.mean, multi_result.std_dev);
        fprintf(bench_file, "speedup,%.2f\n", speedup);
        fclose(bench_file);
    }
    
    return 0;
}

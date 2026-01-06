#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

double random_double() {
    return (double)rand() / RAND_MAX * 2.0 - 1.0;
}

int main(int argc, char *argv[]) {
    long total_points = 10000000; 
    int num_processes = 4;        

    if (argc >= 2) total_points = atol(argv[1]);
    if (argc >= 3) num_processes = atoi(argv[2]);

    long points_per_process = total_points / num_processes;

    long *points_in_circle = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE, 
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *points_in_circle = 0;

    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) { 
            srand(time(NULL) ^ (getpid() << 16)); 
            long local_hits = 0;
            
            for (long j = 0; j < points_per_process; j++) {
                double x = random_double();
                double y = random_double();
                if (x * x + y * y <= 1.0) local_hits++;
            }
            
            __sync_fetch_and_add(points_in_circle, local_hits);
            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++) wait(NULL);

    double pi = 4.0 * (*points_in_circle) / total_points;
    printf("Estimated Pi = %f\n", pi);

    munmap(points_in_circle, sizeof(long));
    return 0;
}
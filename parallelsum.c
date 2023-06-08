#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void generate_random(double *input, size_t size);
double serial_sum(double *x, size_t size);
double omp_sum(double *x, size_t size);
double omp_critical_sum(double *x, size_t size);
double omp_local_sum(double *x, size_t size);
double opt_local_sum(double *x, size_t size);

#define SIZE 10000000
#define NTHREADS 128

typedef struct thread_sum { double val; char pad[128]; } thread_sum;

int main()
{
        omp_set_num_threads(NTHREADS);

        printf("### %d THREADS ###\n", NTHREADS);

        int iterations = 5;

        for (int i = 0; i < iterations; i++)
        {
                // initialize array to sum over
                double *arr = malloc(SIZE * sizeof(double));
                generate_random(arr, SIZE);

                // calculate actual sum to check correctness of other versions
                double sum;
                double control_sum = serial_sum(arr, SIZE);

                double start_time = omp_get_wtime();

                // SET SUM FUNCTION HERE
                sum = opt_local_sum(arr, SIZE);

                double end_time = omp_get_wtime();

                printf("%lf\n", end_time - start_time);
                //printf("Executed in %lf seconds\n", end_time - start_time);
                //printf("Sum was %lf - compare with control: %lf\n", sum, control_sum);

                free(arr);
        }
        return 0;
}

void generate_random(double *input, size_t size)
{
        for (size_t i = 0; i < size; i++)
        {
                input[i] = rand() / (double)(RAND_MAX);
        }

}

double serial_sum(double *x, size_t size)
{
        double sum_val = 0.0;

        for (size_t i = 0; i < size; i++)
        {
                sum_val += x[i];
        }

        return sum_val;
}

double omp_sum(double *x, size_t size)
{
        double sum_val = 0.0;
        int i;

        #pragma omp parallel
        {
                #pragma omp for
                        for (i = 0; i < size; i++)
                        {
                                sum_val += x[i];
                        }
        }

        return sum_val;
}

double omp_critical_sum(double *x, size_t size)
{
        double sum_val = 0.0;
        int i;

        #pragma omp parallel
        {
                #pragma omp for
                for (i = 0; i < size; i++)
                {
                        #pragma omp critical
                        {
                                sum_val += x[i];
                        }
                }
        }
        return sum_val;
}

double omp_local_sum(double *x, size_t size)
{
        double thread_sums[NTHREADS];

        #pragma omp parallel
        {
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();

                int chunk_size = size/num_threads;
                int i;
                int start_i = chunk_size * thread_id;
                int end_i = start_i + chunk_size;

                thread_sums[thread_id] = 0.0;

                for (i = start_i; i < end_i; i++)
                {
                        thread_sums[thread_id] += x[i];
                }
        }

        double tot_sum = 0.0;

        for (int i = 0; i < NTHREADS; i++)
        {
                tot_sum += thread_sums[i];
        }
        return tot_sum;
}

double opt_local_sum(double *x, size_t size)
{
        thread_sum thread_sums[NTHREADS];

        #pragma omp parallel
        {
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();

                int chunk_size = size/num_threads;
                int i;
                int start_i = chunk_size * thread_id;
                int end_i = start_i + chunk_size;

                thread_sums[thread_id].val = 0.0;

                for (i = start_i; i < end_i; i++)
                {
                        thread_sums[thread_id].val += x[i];
                }
        }

        double tot_sum = 0.0;

        for (int i = 0; i < NTHREADS; i++)
        {
                tot_sum += thread_sums[i].val;
        }
        return tot_sum;
}
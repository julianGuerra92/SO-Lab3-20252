/*
 *  OpenMP lecture exercises
 *  Copyright (C) 2011 by Christian Terboven <terboven@rz.rwth-aachen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Estructura para pasar datos a cada hilo
typedef struct {
    int thread_id;
    int start;
    int end;
    int n;
    double partial_sum;
} ThreadData;

double f(double a)
{
    return (4.0 / (1.0 + a*a));
}

// Función que ejecutará cada hilo
void* CalcPiThread(void* arg)
{
    ThreadData* data = (ThreadData*)arg;
    const double fH = 1.0 / (double)data->n;
    double fSum = 0.0;
    double fX;
    
    for (int i = data->start; i < data->end; i++)
    {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    
    data->partial_sum = fSum;
    pthread_exit(NULL);
}

double CalcPi(int n, int num_threads)
{
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ThreadData* thread_data = (ThreadData*)malloc(num_threads * sizeof(ThreadData));
    
    int chunk_size = n / num_threads;
    int remainder = n % num_threads;
    
    // Crear hilos
    int start = 0;
    for (int i = 0; i < num_threads; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].start = start;
        thread_data[i].end = start + chunk_size + (i < remainder ? 1 : 0);
        thread_data[i].n = n;
        thread_data[i].partial_sum = 0.0;
        
        pthread_create(&threads[i], NULL, CalcPiThread, &thread_data[i]);
        
        start = thread_data[i].end;
    }
    
    // Esperar a que todos los hilos terminen y sumar resultados parciales
    double fSum = 0.0;
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
        fSum += thread_data[i].partial_sum;
    }
    
    // Liberar memoria
    free(threads);
    free(thread_data);
    
    const double fH = 1.0 / (double)n;
    return fH * fSum;
}

double GetTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char **argv)
{
    int n = 2000000000;
    int num_threads = 1;
    const double fPi25DT = 3.141592653589793238462643;
    double fPi;
    double fTimeStart, fTimeEnd;
    
    // Leer número de hilos desde línea de comandos
    if (argc > 1)
    {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0)
        {
            printf("Number of threads must be positive\n");
            return 1;
        }
    }
    
#ifdef READ_INPUT  
    printf("Enter the number of intervals: ");
    scanf("%d",&n);
#endif

    if (n <= 0 || n > 2147483647) 
    {
        printf("\ngiven value has to be between 0 and 2147483647\n");
        return 1;
    }
    
    printf("Using %d threads\n", num_threads);
    
    // get initial time
    fTimeStart = GetTime();

    /* the calculation is done here*/
    fPi = CalcPi(n, num_threads);

    //get final time
    fTimeEnd = GetTime();
    
    printf("\npi is approximately = %.20f \nError               = %.20f\n",
           fPi, fabs(fPi - fPi25DT));
    
    // report time
    printf("Execution time: %.6f seconds\n", fTimeEnd - fTimeStart);

    return 0;
}
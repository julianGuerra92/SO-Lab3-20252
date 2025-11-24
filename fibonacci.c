#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Definimos una estructura para pasar múltiples argumentos al hilo.
// Pthreads solo acepta un argumento (void*), por lo que "empaquetamos"
// el tamaño N y el puntero al arreglo en esta estructura.
typedef struct {
    int size;             // Cantidad de números a generar (N)
    long long *fib_array; // Puntero al arreglo compartido donde se escribirán los datos
} ThreadArgs;

// Función que ejecutará el hilo trabajador
// Calcula la secuencia de Fibonacci y llena el arreglo compartido
void *fibonacci_worker(void *arg) {
    // 1. Desempaquetamos los argumentos: Cast de (void*) a (ThreadArgs*)
    ThreadArgs *args = (ThreadArgs *)arg;
    int n = args->size;
    long long *arr = args->fib_array;

    // Manejo de casos base para evitar desbordamiento de índices
    if (n > 0) {
        arr[0] = 0; // f0
    }
    if (n > 1) {
        arr[1] = 1; // f1
    }

    // 2. Cálculo del resto de la secuencia
    // Comienza desde el índice 2 ya que 0 y 1 están definidos
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i - 1] + arr[i - 2];
    }

    // Terminamos el hilo
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    // Validación de argumentos de entrada
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <numero de elementos a generar>\n", argv[0]);
        return -1;
    }

    // Convertir el argumento de línea de comandos a entero
    int n = atoi(argv[1]);

    // Validación básica: no se pueden generar cantidades negativas
    if (n < 0) {
        fprintf(stderr, "Error: El número debe ser mayor o igual a 0.\n");
        return -1;
    }
    
    // Si n es 0, no hay nada que calcular ni imprimir
    if (n == 0) {
        return 0; 
    }

    // 1. GESTIÓN DE MEMORIA (Requisito del Main)
    // Asignamos memoria en el heap para compartirla con el hilo.
    // Usamos 'long long' para soportar números más grandes que un int estándar.
    long long *fib_sequence = (long long *)malloc(n * sizeof(long long));
    if (fib_sequence == NULL) {
        perror("Error al asignar memoria");
        return -1;
    }

    // 2. PREPARACIÓN DE ARGUMENTOS
    // Llenamos la estructura con los datos necesarios
    ThreadArgs args;
    args.size = n;
    args.fib_array = fib_sequence;

    // 3. CREACIÓN DEL HILO (Requisito de pthread_create)
    pthread_t tid; // Identificador del hilo
    pthread_attr_t attr; // Atributos del hilo

    pthread_attr_init(&attr);
    // Creamos el hilo pasando la función y la dirección de nuestra estructura 'args'
    pthread_create(&tid, &attr, fibonacci_worker, &args);

    // 4. SINCRONIZACIÓN (Requisito de pthread_join)
    // El hilo principal espera aquí hasta que el trabajador termine.
    // Esto garantiza que el arreglo esté lleno antes de imprimir.
    pthread_join(tid, NULL);

    // 5. IMPRESIÓN DE RESULTADOS (Responsabilidad del Main)
    // Una vez el hilo worker termina, el main retoma el control e imprime.
    printf("La secuencia de Fibonacci de %d elementos es:\n", n);
    for (int i = 0; i < n; i++) {
        printf("%lld", fib_sequence[i]);
        if (i < n - 1) {
            printf(", "); // Formato con comas
        }
    }
    printf("\n");

    // Liberamos la memoria asignada antes de terminar
    free(fib_sequence);

    return 0;
}
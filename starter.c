#include <stdio.h>
#include <stdlib.h>
#include <omp.h> //openMp library

int main(int argc, char** argv) {
  
 #pragma omp parallel
 {
    printf("Codice parallelo eseguito da %d thread\n",
    omp_get_thread_num());
    if ( omp_get_thread_num() == 2 ) {
        printf("Il thread %d fa cose diverse\n",
        omp_get_thread_num());
    }
 } /* Fine blocco di codice parallelo */

  return 0;
}
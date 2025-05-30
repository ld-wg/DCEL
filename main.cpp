#include "DCEL.h"
#include <cstdio>

int main() {
    DCEL dcel;
    
    if (!dcel.loadFromInput()) {
        fprintf(stderr, "erro: falha ao carregar entrada\n");
        return 1;
    }
    
    if (dcel.isValidDCEL()) {
        dcel.printDCEL();
    }
    
    return 0;
}
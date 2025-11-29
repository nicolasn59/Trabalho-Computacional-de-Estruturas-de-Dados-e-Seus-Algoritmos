#include "TARVBM.h"

int main(){
    inicializar_banco();
    executar_carga_inicial();
    print("Testando busca\n");
    buscar_ano(2002);
    buscar_ano(1931);
    return 0;
}

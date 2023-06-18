#include <stdbool.h>

#include "masa.h"
#include "resorte.h"
#include "calculo.h"
#include "lista.h"
#include "malla.h"

struct masa {
    size_t id;
    int x, y, tam;           // FLOAT O INT???
    bool es_fijo;
    float masa;
    Color color;
};

struct resorte {
    size_t id;
    struct masa *masa1, *masa2;
    float longitud, k_resorte;
    Color color;
};

struct malla {
    lista_t* resortes;
    lista_t* masas;
};

static malla_t *_crear_malla(){
    malla_t *malla = malloc(sizeof(malla_t));
    if(malla == NULL) return NULL;
    return malla;
}



bool borrar_masa(malla_t *malla, masa_t *masa) {
    lista_iter_t *iter = lista_iter_crear(malla->masas);
    while (!lista_iter_al_final(iter)) {
        masa_t *masa_actual = lista_iter_ver_actual(iter);
        if (masa_actual == masa) {
            _borrar_masa(masa_actual);
            lista_iter_borrar(iter);
            lista_iter_destruir(iter);
            return true;
        }
        lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
    return false;
} 

bool insertar_masa(malla_t *malla, masa_t *masa) {
    size_t nueva_id = 1;  // Número inicial para la nueva ID
    lista_iter_t *iter = lista_iter_crear(malla->masas);

    // Buscar el primer número disponible iterando desde el menor al mayor
    while (!lista_iter_al_final(iter)) {
        masa_t *masa_actual = lista_iter_ver_actual(iter);
        if (masa_actual->id == nueva_id) {
            // El número de ID está en uso, incrementar y buscar el siguiente
            nueva_id++;
            lista_iter_avanzar(iter);
        } else {
            // Se encontró una ID disponible, actualizar la ID de la masa
            masa->id = nueva_id;
            break;
        }
    }

    bool insercion_exitosa = lista_iter_insertar(iter, masa);
    lista_iter_destruir(iter);

    return insercion_exitosa;
}

masa_t *nueva_masa(malla_t *malla, int x, int y, int tam, Color color){
    masa_t *masa = crear_masa(x, y, tam, color);
    insertar_masa(malla, masa);
    return masa;
}

masa_t *nueva_masa_fija(malla_t *malla, int x, int y, int tam, Color color){
    masa_t *masa = crear_masa_fija(x, y, tam, color);
    insertar_masa(malla, masa);
    return masa;
}

masa_t *detectar_masa(malla_t *malla, int x, int y, int tolerancia) {
    lista_iter_t *iter = lista_iter_crear(malla->masas);
    while (!lista_iter_al_final(iter)) {
        masa_t *masa_actual = lista_iter_ver_actual(iter);
        int distancia_x = abs(masa_actual->x - x);
        int distancia_y = abs(masa_actual->y - y);
        if (distancia_x <= tolerancia && distancia_y <= tolerancia) {
            lista_iter_destruir(iter);
            return masa_actual;
        }
        lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
    return NULL;
}

void mover_masa(malla_t *malla, masa_t *masa, int x, int y, float longitud_maxima) {
    lista_iter_t *iter = lista_iter_crear(malla->masas);
    if (iter == NULL) return;
    
    while (!lista_iter_al_final(iter)) {
        masa_t *masa_actual = lista_iter_ver_actual(iter);
        
        if (masa_actual->id == masa->id) {
            // Guardar la posición anterior de la masa
            int x_anterior = masa_actual->x;
            int y_anterior = masa_actual->y;
            
            // Actualizar la posición de la masa
            masa_actual->x = x;
            masa_actual->y = y;
            
            if (!verificar_longitud_resortes(malla, masa_actual, longitud_maxima)) {
                // Restaurar la posición anterior de la masa si la longitud de algún resorte se excede
                masa_actual->x = x_anterior;
                masa_actual->y = y_anterior;
            }
            
            break;
        }
        
        lista_iter_avanzar(iter);
    }
    
    lista_iter_destruir(iter);
}

resorte_t *detectar_resorte(malla_t *malla, int x, int y, float tolerancia) {
    lista_iter_t *iter = lista_iter_crear(malla->resortes);
    while (!lista_iter_al_final(iter)) {
        resorte_t *resorte_actual = lista_iter_ver_actual(iter);
        int xp1 = resorte_actual->masa1->x;
        int yp1 = resorte_actual->masa1->y;
        int xp2 = resorte_actual->masa2->x;
        int yp2 = resorte_actual->masa2->y;
        
        // Calcular la distancia del punto a la recta
        float distancia = distancia_punto_a_recta(xp1, yp1, xp2, yp2, x, y);

        // Si la distancia es menor a una tolerancia, se considera que el punto está cerca del resorte
        if (distancia < tolerancia) {
            // Verificar si el punto se encuentra entre las masas
            int x_min = xp1 < xp2 ? xp1 : xp2;
            int x_max = xp1 > xp2 ? xp1 : xp2;
            int y_min = yp1 < yp2 ? yp1 : yp2;
            int y_max = yp1 > yp2 ? yp1 : yp2;

            if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
                lista_iter_destruir(iter);
                return resorte_actual;
            }
        }

        lista_iter_avanzar(iter);
    }

    lista_iter_destruir(iter);
    return NULL;
}

bool borrar_resorte(malla_t *malla, resorte_t *resorte) {
    lista_iter_t *iter = lista_iter_crear(malla->resortes);
    while (!lista_iter_al_final(iter)) {
        resorte_t *resorte_actual = lista_iter_ver_actual(iter);
        if (resorte_actual == resorte) {
            _borrar_resorte(resorte_actual);
            lista_iter_borrar(iter);
            lista_iter_destruir(iter);
            return true;
        }
        lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
    return false;
}                           

void eliminar_resortes_conectados(malla_t *malla, masa_t *masa) {
    lista_iter_t *iter_resortes = lista_iter_crear(malla->resortes);
    if (iter_resortes == NULL) return;

    while (!lista_iter_al_final(iter_resortes)) {
        resorte_t *resorte = lista_iter_ver_actual(iter_resortes);
        
        if (resorte->masa1 == masa || resorte->masa2 == masa) {
            _borrar_resorte(resorte);
            lista_iter_borrar(iter_resortes);
        } else {
            lista_iter_avanzar(iter_resortes);
        }
    }
    
    lista_iter_destruir(iter_resortes);
}

bool insertar_resorte(malla_t *malla, resorte_t *resorte) {
    return lista_insertar_ultimo(malla->resortes, resorte);
}                             

resorte_t *nuevo_resorte(malla_t *malla, masa_t *m1, masa_t *m2, Color color){
    resorte_t *resorte = crear_resorte(m1, m2, color);
    insertar_resorte(malla, resorte);
    return resorte;
}

bool masas_conectadas(malla_t *malla, masa_t *m1, masa_t *m2) {
    lista_iter_t *iter = lista_iter_crear(malla->resortes);
    if (iter == NULL) return false;

    while (!lista_iter_al_final(iter)) {
        resorte_t *resorte = lista_iter_ver_actual(iter);
        masa_t *masa_a = resorte->masa1;
        masa_t *masa_b = resorte->masa2;

        if ((masa_a == m1 && masa_b == m2) || (masa_a == m2 && masa_b == m1)) {
            lista_iter_destruir(iter);
            return true; // Las masas están conectadas por un resorte
        }

        lista_iter_avanzar(iter);
    }

    lista_iter_destruir(iter);
    return false; // Las masas no están conectadas por un resorte
}

bool excede_max_longitud(malla_t *malla, masa_t *masa, int x, int y, float maxima_longitud) {
    float distancia = distancia_puntos(masa->x, masa->y, x, y);
    return (distancia > maxima_longitud);
}

bool verificar_longitud_resortes(malla_t *malla, masa_t *masa, float longitud_maxima) {
    lista_iter_t *iter = lista_iter_crear(malla->resortes);
    if (iter == NULL) return false;
    
    while (!lista_iter_al_final(iter)) {
        resorte_t *resorte = lista_iter_ver_actual(iter);
        masa_t *masa1 = resorte->masa1;
        masa_t *masa2 = resorte->masa2;
        
        if (masa1 == masa || masa2 == masa) {
            float longitud = distancia_puntos(masa1->x, masa1->y, masa2->x, masa2->y);
            
            if (longitud > longitud_maxima) {
                lista_iter_destruir(iter);
                return false; // La longitud de un resorte se excede, retornar false
            }
        }
        
        lista_iter_avanzar(iter);
    }
    
    lista_iter_destruir(iter);
    return true; // La longitud de todos los resortes es válida, retornar true
}

static void _renovar_longitud_resortes(malla_t *malla, masa_t *masa) {
    lista_iter_t *iter = lista_iter_crear(malla->resortes);
    while (!lista_iter_al_final(iter)) {
        resorte_t *resorte = lista_iter_ver_actual(iter);
        if (resorte->masa1 == masa || resorte->masa2 == masa) {
            float nueva_longitud = distancia_entre_masas(resorte->masa1, resorte->masa2);
            resorte->longitud = nueva_longitud;
        }
        lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
}                  

malla_t *crear_malla() {
    malla_t *nueva_malla = malloc(sizeof(malla_t));
    if (nueva_malla == NULL) {
        return NULL;
    }

    nueva_malla->resortes = lista_crear();
    if (nueva_malla->resortes == NULL) {
        free(nueva_malla);
        return NULL;
    }

    nueva_malla->masas = lista_crear();
    if (nueva_malla->masas == NULL) {
        lista_destruir(nueva_malla->resortes, NULL);
        free(nueva_malla);
        return NULL;
    }

    return nueva_malla;
}
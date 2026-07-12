/*
	FLOW
INIT matrice normale OK
creazione matrice cubica OK
 

CHANGE COST traduce le coordinate offset in coordinate cubiche OK
	acede alla i-esima cella, se non c'è la aggiunge alla hash table
	cambia il valore
		se il valore è 1 e non ci sono rotte aeree, libera la tabella

TOGGLE-AIR-ROUTE:costo uguale a quello per arrivare alla celle adiacente OK
	salva dentro l'hashmap l'altra destinazione possibile (lista dinamica di massimo 5 elementi) 
	
TRAVEL_COST OK
	implementare dijkstra sensato
		come gestisco le rotte aeree? usando dijkstra in teroia calcolo tutte le opzioni
		dijkstra testa tutte le strade vicine più quelle collegate con rotte aeree
	se una cella non è nella tabella hash è perchè ha costo 1 e non ha alcuna rotta aerea
*/


/*
TODO
	verificare con strumenti vari la velocità e se ci sono problemi
	testare i rimanenti test pubblici per raggiungere una velocità decente
*/

/*
COSE FATTE
	PASSATO EDGE_CASES E EXAMPLE
	SISTEMATO, PROBLEMA CON LE COLLISIONI travel_cost non funziona se la distanza tra i punti è maggiore di 50f
installare tool per verificare memoria e tempo (valgrind e altri suggeriti dalle slide del prof)
	RISOLTO fare check se INIT distrugge l'eventuale tabella hash preesitente (cancella rotte aree e costi default)

 */
//librerie
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h> // Include limits.h for INT_MAX



//costanti
#define INF INT_MAX
#define SIZE 524288
#define LARGE 1000000

//STRUTTURE DATI

typedef struct { //coordinate offset per check correttezza input
	int x;
	int y;
} t_std_coord;

typedef struct { //coordinate cubiche per distanza
	int q;
	int r;
	int s;
} t_axial_coord;


//FUNZIONI AUSILIARIE
t_std_coord castToOffset(t_axial_coord hex) {
	t_std_coord out;
	int parity = hex.r&1;
	int col = hex.q + (hex.r - parity) / 2;
	int row = hex.r;
	out.y = row;
	out.x = col;
	return out;

}

//OPERAZIONI SU COORDINATE CUBICHE
t_axial_coord cube(int q, int r, int s) {
	t_axial_coord cubed;
	cubed.q = q;
	cubed.r = r;
	cubed.s = s;
	return cubed;
}

bool checkValid(t_axial_coord exa, t_axial_coord limit) {
	t_std_coord end, cur;
	cur = castToOffset(exa);
	end = castToOffset(limit);
	//printf(" %d %d %d %d\n", cur.x,cur.y,end.x,end.y);

	return (cur.x < end.x && cur.y < end.y && cur.x >= 0 && cur.y >= 0);
}

//traduce coordinate con offset in coordinate cubiche
t_axial_coord castToAxial(t_std_coord mat) {
	int parity = mat.y & 1;
	return cube(mat.x - (mat.y - parity) / 2, mat.y, -(mat.x - (mat.y - parity) / 2) - mat.y);
}


//funzione di supporto alla distanza
t_axial_coord subCoord(t_axial_coord a, t_axial_coord b) {
	return cube(a.q - b.q, a.r - b.r, a.s - b.s);
}


//distanza tra coordinate cubiche
int calcDist(t_axial_coord coordA, t_axial_coord coordB) {
	t_axial_coord vec = subCoord(coordA, coordB);
	return (abs(vec.q) + abs(vec.r) + abs(vec.s)) / 2;
}

//somma tra due coordinate cubiche
t_axial_coord cubeAdd(t_axial_coord hex, t_axial_coord vec) {
	return cube(hex.q + vec.q, hex.r + vec.r, hex.s + vec.s);
}

bool cubeEqual (t_axial_coord a, t_axial_coord b) {
	return a.q == b.q && a.r == b.r;
}


//visita dei vicini
 t_axial_coord cube_direction_vectors[6] = {
	{+1, 0, -1}, {+1, -1, 0}, {0, -1, +1},
	{-1, 0, +1}, {-1, +1, 0}, {0, +1, -1}
};
t_axial_coord cubeDirection(int direction) {
	return cube_direction_vectors[direction];
}
static inline t_axial_coord cubeNeighbor(t_axial_coord cube, int direction) {
	return cubeAdd(cube, cubeDirection(direction));
}

//CACHE DELLE DISTANZE GIÀ CALCOLATE


typedef struct CH { //elemento della CACHE
	t_std_coord end;
	t_std_coord start;
	int costo;
	struct CH* next;
} elemento_cache;

typedef elemento_cache* cache; //lista delle rotte aeree

//creare un elemento della lista
cache creaElemCache(t_std_coord coord1, t_std_coord coord2, int costo) {
	cache elem = (cache)malloc(sizeof(elemento_cache));
	elem->start = coord1;
	elem->end = coord2;
	elem->costo = costo;
	elem->next = NULL;
	return elem;
}

//aggiungerlo alla lista
void addCache(cache* list_cache, t_std_coord coord1, t_std_coord coord2, int costo) {
	cache rotta = creaElemCache(coord1,coord2,costo);
	rotta->next = *list_cache;
	*list_cache = rotta;
}

cache libCache(cache cache_list) {

	cache current = cache_list;
	cache temp;

	while (current != NULL) {
		temp = current;
		current = current->next;
		free(temp);
	}
	return NULL;
}

int costoCache(cache list_cache, t_std_coord start, t_std_coord end) {
	cache current = list_cache;

	while(current != NULL) {
		if(current->start.x == start.x && current->start.y == start.y) {
			if(current->end.x == end.x && current->end.y == end.y) {
				return current->costo;
			}
		}
		current = current-> next;
	}
	return -1;
}


//LISTA PER ROTTE AEREE E OPERAZIONI: OK

typedef struct RA { //elemento della lista di rotte aeree
	t_std_coord dest;
	struct RA* next;
} rotta_aerea;

typedef rotta_aerea* lista_rotte; //lista delle rotte aeree

//creare un elemento della lista
lista_rotte creaRotta(t_std_coord coord) {
	lista_rotte rotta = (lista_rotte)malloc(sizeof(rotta_aerea));
	rotta->dest = coord;
	rotta->next = NULL;
	return rotta;
}

//aggiungerlo alla lista
void addRotta(lista_rotte* prima_rotta, t_std_coord coord) {
	lista_rotte rotta = creaRotta(coord);
	rotta->next = *prima_rotta;
	*prima_rotta = rotta;
}


//rimuovere un valore
lista_rotte deleteRic(lista_rotte lista, t_std_coord coord) {
	lista_rotte temp;
	if(lista != NULL) {
		if(lista->dest.x == coord.x && lista->dest.y == coord.y) {
			temp = lista->next;
			free(lista);
			return temp;
		}
		else
			lista->next = deleteRic(lista->next, coord);

		return lista;
	}
	return NULL;

}

//controllo valori
bool checkRotta(lista_rotte temp, t_std_coord coord) {

	while (temp != NULL) {
		if(coord.x == temp->dest.x  && temp->dest.y == coord.y) {
			return true;
		}
        temp = temp->next;
    }
	return false;
}

int contaRotte(lista_rotte temp) {
	int a=0;
	while (temp != NULL) {
		a++;
        temp = temp->next;
    }
	return a;
}



//stampa lista  [debug] (accedere ai valori/modificare un valore)
void stampaRotte(lista_rotte temp) {
	while (temp != NULL) {
		printf("%d, %d -> ", temp->dest.x, temp->dest.y);
        temp = temp->next;
    }
    //printf("NULL\n");
}

//CODA PER LE CELLE DA VISITARE (CHANGE COST)

typedef struct queue_node {
    t_axial_coord cell;              // La cella esagonale
    struct queue_node* next; // Puntatore al nodo successivo
} queue_node;

typedef struct {
    queue_node* front; // Puntatore al primo nodo della coda
    queue_node* rear;  // Puntatore all'ultimo nodo della coda
} Queue;

Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

int isEmpty(Queue* queue) {
    return queue->front == NULL;
}

void enqueue(Queue* queue, t_axial_coord cell) {
    queue_node* new_node = (queue_node*)malloc(sizeof(queue_node));
    new_node->cell = cell;
    new_node->next = NULL;

    if (isEmpty(queue)) {
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}

queue_node* dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        return NULL; // La coda è vuota
    }
    queue_node* temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL; // Se la coda diventa vuota
    }
    return temp;
}

void freeQueue(Queue* queue) {
    while (!isEmpty(queue)) {
        queue_node* node = dequeue(queue);
        free(node);
    }
    free(queue);
}


//TABELLA HASH PER MAPPA E OPERAZIONI: OK
//tabella hash per costi
	//chiave: somma pesata delle coordinate (ogni coordinata è moltiplicata con un numero primo molto grande)
	//funzione di hash: metodo della moltiplicazione con numero di Knuth
	//non devo salvare tutto per forza, devo solo salvare se cambia il costo o se viene aggiunta una rotta aerea
	
typedef struct { //valore della tabella hash
	int costo;
	lista_rotte rotte; //lista delle rotte
	t_axial_coord coord;
	//int dist; per algoritmo che non funziona
	//bool used; per algoritmo che non funziona
} t_uscita;


struct Nodo { //nodo della tabella hash
	int key;
	t_uscita u;  
	struct Nodo* next;
};

struct HashTable {
    struct Nodo* bucket[SIZE];
};


int createKey(t_axial_coord mat) {
	int primoR, primoQ, primoS;
	primoR = 6131;
	primoQ = 6011;
	primoS = 6317;
	return mat.q*primoQ + mat.r*primoR + mat.s*primoS;

}

int createHash (int key) {

	key = ((key >> 16) ^ key) * 0x45d9f3b;
	key = ((key >> 16) ^ key) * 0x45d9f3b;
	key = (key >> 16) ^ key;
	return key & (SIZE - 1); // SIZE deve essere potenza di 2
}

void libRotte(lista_rotte lista) {
	
    lista_rotte current = lista;
    lista_rotte temp;
    
    while (current != NULL) {
        temp = current;
        current = current->next;
        
        // Se i nodi della lista contengono altri dati dinamici, liberali qui
        // Esempio: se c'è un campo char* nome:
        // if (temp->nome != NULL) {
        //     free(temp->nome);
        // }
        
        free(temp);
    }
}

// Funzione per liberare una singola catena di nodi in un bucket
void liberaCatenaBucket(struct Nodo* nodo) {
    while (nodo != NULL) {
        struct Nodo* temp = nodo;
        nodo = nodo->next;
        
        // Libera la lista_rotte contenuta nel nodo
        libRotte(temp->u.rotte);
        
        // Libera il nodo stesso
        free(temp);
    }
}

void liberaTabella(struct HashTable* tabella) {
	
	if (tabella == NULL) {
        return;
    }
    
    // Libera tutti i bucket
    for (int i = 0; i < SIZE; i++) {
        if (tabella->bucket[i] != NULL) {
            liberaCatenaBucket(tabella->bucket[i]);
            tabella->bucket[i] = NULL;  // Opzionale: azzera il puntatore
        }
    }
    
    // Libera la struttura HashTable stessa
    free(tabella);

}



struct HashTable* creaHashTable() {
    struct HashTable* tabella = (struct HashTable*)malloc(sizeof(struct HashTable));
    for (int i = 0; i < SIZE; i++) {
        tabella->bucket[i] = NULL;
    }
    return tabella;
}

//inserisce un valore nella tabella
void inserisci(struct HashTable* tabella, int chiave, t_uscita valore) {
    int indice = createHash(chiave) % SIZE;

    struct Nodo* nuovo_nodo = (struct Nodo*)malloc(sizeof(struct Nodo));
    //valore.used = false;
    //valore.dist = LARGE;
	//stampaRotte(valore.rotte);
    nuovo_nodo->key = chiave;
    nuovo_nodo->u = valore;
    nuovo_nodo->next = tabella->bucket[indice];
    tabella->bucket[indice] = nuovo_nodo;
}


//cerca un valore nella tabella
t_uscita cerca(struct HashTable* tabella, int chiave, t_axial_coord c) {
	t_uscita test;
	test.costo = -1;
	test.rotte = NULL;
    int indice = createHash(chiave) % SIZE;

    struct Nodo* nodo = tabella->bucket[indice];
    while (nodo) {
        if (nodo->key == chiave) {
			if(cubeEqual(c, nodo->u.coord)) {
				return nodo->u;
			}
        }
        nodo = nodo->next;
    }
    return test;  // Chiave non trovata
}

//cerca un valore nella tabella speciale per assegnare valori PER SOSTITUIRE I VALORI RITORNO IL NODO E POI AGGIORNO CON IL PUNTATORE
struct Nodo* sostituisci(struct HashTable* tabella, int chiave, t_axial_coord c) {
    int indice = createHash(chiave) % SIZE;
    struct Nodo* nodo = tabella->bucket[indice];
    while (nodo) {
        if (nodo->key == chiave) {
			if(cubeEqual(c, nodo->u.coord)) {
				return nodo;
			}
        }
        nodo = nodo->next;
    }
    return NULL;  // Chiave non trovata
}

void cancella(struct HashTable* tabella, int chiave) {


    unsigned int indice = createHash(chiave) % SIZE;
    struct Nodo* nodo = tabella->bucket[indice];
    struct Nodo* precedente = NULL;

	while(nodo != NULL) {
		
		if(nodo->key == chiave) {
			
			if(nodo == tabella->bucket[indice]) {
				tabella->bucket[indice] = nodo->next;
			}
			else {
				precedente->next = nodo->next;
			}
			free(nodo->u.rotte);
			free(nodo);
		
		}
		
		precedente = nodo;
		nodo = nodo->next;

	}
}



//FUNZIONI PRINCIPALI: change_cost, toggle_air_route, travel_cost

typedef struct { //valore della tabella hash
	int costo;
	t_axial_coord coord;
} t_nodoDij;

struct NodoDI { //nodo della tabella hash
	int key;
	t_nodoDij u;
	struct NodoDI* next;
};

struct HashTable2 {
	struct NodoDI* secchio[SIZE];
};


// Funzione per liberare una singola catena di nodi in un secchio
void liberaCatenaSecchio(struct NodoDI* nodo) {
	while (nodo != NULL) {
		struct NodoDI* temp = nodo;
		nodo = nodo->next;

		// Libera il nodo stesso
		free(temp);
	}
}

void liberaTabella2(struct HashTable2* tabella) {

	if (tabella == NULL) {
		return;
	}

	// Libera tutti i secchio
	for (int i = 0; i < SIZE; i++) {
		if (tabella->secchio[i] != NULL) {
			liberaCatenaSecchio(tabella->secchio[i]);
			tabella->secchio[i] = NULL;  // Opzionale: azzera il puntatore
		}
	}

	// Libera la struttura HashTable stessa
	free(tabella);

}


struct HashTable2* creaHashTable2() {
	struct HashTable2* tabella = (struct HashTable2*)malloc(sizeof(struct HashTable2));
	for (int i = 0; i < SIZE; i++) {
		tabella->secchio[i] = NULL;
	}
	return tabella;
}

//inserisce un valore nella tabella
void inserisci2(struct HashTable2* tabella, int chiave, t_nodoDij valore) {
	int indice = createHash(chiave) % SIZE;

	struct NodoDI* nuovo_nodo = (struct NodoDI*)malloc(sizeof(struct NodoDI));
	nuovo_nodo->key = chiave;
	nuovo_nodo->u = valore;
	nuovo_nodo->next = tabella->secchio[indice];
	tabella->secchio[indice] = nuovo_nodo;
}

//cerca un valore nella tabella
t_nodoDij cerca2(struct HashTable2* tabella, int chiave, t_axial_coord c) {

	int indice = createHash(chiave) % SIZE;

	struct NodoDI* nodo = tabella->secchio[indice];
	while (nodo) {
		if (nodo->key == chiave) {
			if(cubeEqual(nodo->u.coord,c)) {
				return nodo->u;
			}
		}
		nodo = nodo->next;
	}
	t_nodoDij test;
	test.costo = INF;
	test.coord = cube(-1, -1, -1);
	return test;  // Chiave non trovata
}

//cerca un valore nella tabella speciale per assegnare valori
struct NodoDI* sostituisci2(struct HashTable2* tabella, int chiave, t_axial_coord c) {
	int indice = createHash(chiave) % SIZE;
	struct NodoDI* nodo = tabella->secchio[indice];
	while (nodo) {
		if (nodo->key == chiave) {
			if(cubeEqual(nodo->u.coord,c)) {
				return nodo;
			}
		}
		nodo = nodo->next;
	}
	return NULL;  // Chiave non trovata
}

void cancella2(struct HashTable2* tabella, int chiave) {
	unsigned int indice = createHash(chiave) % SIZE;
	struct NodoDI* nodo = tabella->secchio[indice];
	struct NodoDI* precedente = NULL;

	while(nodo != NULL) {

		if(nodo->key == chiave) {

			if(nodo == tabella->secchio[indice]) {
				tabella->secchio[indice] = nodo->next;
			}
			else {
				precedente->next = nodo->next;
			}
			free(nodo);

		}

		precedente = nodo;
		nodo = nodo->next;

	}
}
int max(int a, int b) {
	return (a > b) ? a : b;
}

int min(int a, int b) {
	return (a < b) ? a : b;
}


void visitCell2(t_axial_coord start, int r, t_axial_coord limit, struct HashTable* tabella, int v) {

	for (int q = -r; q <= r; q++) {
		int r1 = max(-r, -q-r);
		int r2 = min(r, -q+r);

		for(int rc = r1; rc <= r2; rc++) {
			t_axial_coord coord = cube(start.q + q, start.r + rc, -(start.q + q) - (start.r + rc));

			if (!checkValid(coord, limit)) continue;

			if (calcDist(start,coord) > r) continue;

			//aggiornamento costo
			int old_c = cerca(tabella, createKey(coord), coord).costo;

			int old = (old_c == -1) ? 1 : old_c;   // -1 valore se non esiste in tabella
			float b = (float)(r - calcDist(start,coord))/r;
			float max = (0 > b) ? 0 : b;
			int new_c = floor(old + (v*max));
			new_c = (new_c < 0) ? 0 : new_c;
			new_c = (new_c > 100) ? 100 : new_c;
			//printf("\n %d \n", old_c);
			t_uscita new;
			//printf("\n %d %d %d \n", castToOffset(coord).x, castToOffset(coord).y, new_c);
			//FUNZIONA
			if(old_c == -1) {
				//printf("\n %d \n", new_c);

				if(new_c != 1) {
					new.rotte = NULL;
					new.costo = new_c;
					new.coord = coord;
					inserisci(tabella, createKey(coord), new);
				}

			}

			else {

				//new.dest = sostituisci(tabella, createKey(current->cell))->u.rotte;
				sostituisci(tabella, createKey(coord), coord)->u.costo = new_c;
			}

		}
	}

}
//disjkstra con heap e hashmap per le distanze

//DIJKSTRA CON HEAP E HASH TABLE PER DISTANZE

//FUNZIONA MA UN BOTTO LENTO


// Struttura dell'elemento dell'heap
typedef struct {
    t_axial_coord coord;           // coordinate dell'esagono
    int dist;      // Distanza utilizzata per l'ordinamento
} heap_node;

// Struttura dell'heap
typedef struct {
    heap_node *array;    // Array dinamico degli elementi
    int size;             // Numero corrente di elementi
    int capacity;         // Capacità massima corrente
} min_heap;

// Inizializza l'heap con capacità iniziale
min_heap* createHeap(int initial_capacity) {
    min_heap *heap = (min_heap*)malloc(sizeof(min_heap));
    if (!heap) return NULL;
    
    heap->array = (heap_node*)malloc(initial_capacity * sizeof(heap_node));
    if (!heap->array) {
        free(heap);
        return NULL;
    }
   
    
    heap->size = 0;
    heap->capacity = initial_capacity;
    return heap;
}

// Distrugge l'heap liberando la memoria
void destroyHeap(min_heap *heap) {
    if (heap) {
        free(heap->array);
        free(heap);
    }
}

// Ridimensiona l'array se necessario
int resizeHeap(min_heap *heap, int new_capacity) {
    heap_node *new_array = (heap_node*)realloc(heap->array, new_capacity * sizeof(heap_node));
    if (!new_array) return 0; // Errore
    
    heap->array = new_array;
    heap->capacity = new_capacity;
    return 1; // Successo
}

// Scambia due elementi nell'heap
void swap(heap_node *a, heap_node *b) {
    heap_node temp = *a;
    *a = *b;
    *b = temp;
}

// Restituisce l'indice del genitore
int parent(int i) {
    return (i - 1) / 2;
}

// Restituisce l'indice del figlio sinistro
int leftChild(int i) {
    return 2 * i + 1;
}

// Restituisce l'indice del figlio destro
int rightChild(int i) {
    return 2 * i + 2;
}

// Heapify verso l'alto (utilizzato nell'inserimento)
void heapifyUp(min_heap *h, int index) {
    if (index == 0) return;

	heap_node target = h->array[index]; // Salviamo il nodo da inserire
	int current = index;

	while(current > 0) {
		int parent = (current - 1) >> 1;

		// Se il genitore è già più piccolo, abbiamo trovato il posto
		if (h->array[parent].dist <= target.dist) {
			break;
		}

		h->array[current] = h->array[parent];
		current = parent;
	}
	h->array[current] = target;
}

// Heapify verso il basso (utilizzato nell'estrazione)
void heapifyDown(min_heap *h, int index) {
	heap_node target = h->array[index]; // Salviamo il nodo da riposizionare
	int current = index;
	int half_size = h->size >> 1; // Solo i nodi prima di half_size hanno figli

	while (current < half_size) {
		int left = (current << 1) + 1;
		int right = left + 1;
		int smallest = left;

		// Se esiste un figlio destro ed è più piccolo del sinistro, scegliamo lui
		if (right < h->size && h->array[right].dist < h->array[left].dist) {
			smallest = right;
		}

		// Se il figlio più piccolo è già maggiore o uguale al target, abbiamo finito
		if (h->array[smallest].dist >= target.dist) {
			break;
		}

		// Spostiamo il figlio verso l'alto
		h->array[current] = h->array[smallest];
		current = smallest;
	}

	// Posizioniamo il nodo target nella sua collocazione finale
	h->array[current] = target;
}


// Inserisce un elemento nell'heap
int insert(min_heap *heap, heap_node element) {

    // Ridimensiona se necessario (raddoppia la capacità)
    if (heap->size == heap->capacity) {
        if (!resizeHeap(heap, heap->capacity * 2)) {
            return 0; // Errore di allocazione
        }
    }
    
    // Inserisce il nuovo elemento alla fine
    heap->array[heap->size] = element;
    heap->size++;
    
    // Ripristina la proprietà dell'heap
    heapifyUp(heap, heap->size - 1);
    
    return 1; // Successo
}

// Estrae l'elemento con distanza minima (radice)
heap_node extractMin(min_heap *heap) {
    heap_node min_element = heap->array[0];
    

    // Sposta l'ultimo elemento alla radice
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    
    // Ripristina la proprietà dell'heap
    if (heap->size > 0) {
        heapifyDown(heap, 0);
    }
    // POCO VANTAGGIOSO TANTO TEMPO NECESSARIO
    // // Ridimensiona se l'heap è troppo vuoto (meno del 25% della capacità)
    // if (heap->size > 0 && heap->size < heap->capacity / 4) {
    //     resizeHeap(heap, heap->capacity / 2);
    // }

    return min_element;
}


// Stampa l'heap (per debug)

void printHeap(min_heap *heap) {
    printf("Heap (size=%d, capacity=%d): ", heap->size, heap->capacity);
    for (int i = 0; i < heap->size; i++) {
        printf("[x:%d, y:%d, dist:%d] ", castToOffset(heap->array[i].coord).x, castToOffset(heap->array[i].coord).y, heap->array[i].dist);
    }
    printf("\n");
}

// Verifica se l'heap è vuoto
int isEmptyHeap(min_heap *heap) {
    return heap->size == 0;
}

// Restituisce la dimensione dell'heap
int getSize(min_heap *heap) {
    return heap->size;
}


//nodo per DIJKSTRA
typedef struct {
	int dist;
	int timestamp_global;  //estratto dall'heap
	int timestamp_local;	//dist modificata
	int costo_uscita;
} DijkstraNode;

int idxDij(t_axial_coord coord, t_axial_coord limit) {
	t_std_coord off = castToOffset(coord);
	t_std_coord max = castToOffset(limit);

	int width_reale = max.x;
	return off.y * width_reale + off.x;
}

//spachettamento dijkstra



void checkRotte(int alt, min_heap *heap, t_axial_coord u, t_axial_coord limit, lista_rotte lista, DijkstraNode *nodes, int global_timestamp) {
	//implementare anche per le rotte aeree

		while(lista != NULL) {
			t_axial_coord fly = castToAxial(lista->dest);

			if(checkValid(fly, limit)) {

				int fly_idx = idxDij(fly, limit);

				if(nodes[fly_idx].timestamp_global == global_timestamp) {
					lista = lista->next;
					continue;
				}

				int current_dist = (nodes[fly_idx].timestamp_local == global_timestamp) ? nodes[fly_idx].dist : INF;


				if(alt < current_dist) {


					nodes[fly_idx].dist = alt;
					nodes[fly_idx].timestamp_local = global_timestamp;


					heap_node nodo2;
					nodo2.coord = fly;
					nodo2.dist = alt;
					insert(heap, nodo2);
				}


			}
			lista = lista->next;
		}

}


// ALGORITMO DI DIJKSTRA
int dijkstra(struct HashTable* tabella, t_axial_coord start, t_axial_coord end, t_axial_coord limit, DijkstraNode *nodes, int *p_global_timestamp, min_heap *heap) {

	(*p_global_timestamp)++;
	int global_timestamp = *p_global_timestamp;

	heap_node nodo;
	if(cubeEqual(start,end)) {
		return 0;
	}
	heap->size = 0;
	//min_heap *heap = createHeap(2);

	nodo.coord = start;
	nodo.dist = 0;
	
	insert(heap, nodo);

	nodes[idxDij(start, limit)].dist = 0;

	/*
	struct HashTable2* tabella2 = creaHashTable2();

	t_nodoDij nodoD;
	nodoD.coord = start;
	nodoD.costo = 0;
	nodoD.visited = false;
	inserisci2(tabella2, createKey(nodoD.coord), nodoD);*/

	//printHeap(heap);
	while(!isEmptyHeap(heap)) {
		//printHeap(heap);

		heap_node current = extractMin(heap);
		t_axial_coord u = current.coord;
		int u_idx = idxDij(u, limit);

		//printf("\n%d %d \n", castToOffset(u).x,castToOffset(u).y);
		if(cubeEqual(u, end)) {

			//destroyHeap(heap);

			return current.dist;

		}
		//int keyU = createKey(u);
		int dist, alt;

		DijkstraNode *nodo_u = &nodes[u_idx];

		struct Nodo* t_nodo_u = sostituisci(tabella, createKey(u), u);


		if(nodes[u_idx].timestamp_global == global_timestamp) {
			continue;
		}

		nodes[u_idx].timestamp_global = global_timestamp;


		if(current.dist > nodo_u->dist) {
			continue;
		}




		//cerco il costo di uscita dal nodo della griglia
		if (t_nodo_u == NULL) { //uso nodo.coord perchè è il costo di uscita che ho, non quello di entrata nel nodo
			dist = 1;
		} else if (t_nodo_u->u.costo == 0) {  //se ha costo 0, non può essere attraversato
			continue;
		} else {
			dist = t_nodo_u->u.costo;
		}

		alt = current.dist + dist; 		  //costo salvato nella tabella hash di nodo + il costo per uscire da nodo e andare al vicino

		//checkVicini(alt, heap, u, limit, nodes, global_timestamp);
		for (int i = 0; i < 6; i++) {
			t_axial_coord v = cubeNeighbor(u,i);

			if(checkValid(v, limit)) {

				//int key = createKey(v);
				int v_idx = idxDij(v, limit);

				if(nodes[v_idx].timestamp_global == global_timestamp) {
					continue;
				}

				int current_dist = (nodes[v_idx].timestamp_local == global_timestamp) ? nodes[v_idx].dist : INF;

				if(alt < current_dist) {

					nodes[v_idx].dist = alt;
					nodes[v_idx].timestamp_local = global_timestamp;

					heap_node nodo2;
					nodo2.coord = v;
					nodo2.dist = alt;
					//printf("\nadd to heap: %d %d %d\n", castToOffset(v).x, castToOffset(v).y, alt);
					insert(heap, nodo2);
					//printHeap(heap);
				}
			}
		}

		if (t_nodo_u != NULL) {
			if(t_nodo_u->u.rotte != NULL) {
				checkRotte(alt, heap, u, limit, t_nodo_u->u.rotte, nodes, global_timestamp);
			}
		}

	}

    //destroyHeap(heap);
    return -1;
}



//main con if else per gestire quello che succede in base alla riga che legge
int main() {
	FILE* file = stdin; //per testare, da sostituire poi
	char string[SIZE];
	int x, y, r, v;
	
	t_axial_coord matC;
	t_std_coord mat; //serve per controllare l'esistenza delle coordinate inserite
	
	mat.x = 0;
	mat.y = 0;
	matC = castToAxial(mat);
	int global_timestamp = 0;


	struct HashTable* tabella = NULL;
	DijkstraNode *nodes = NULL;
	min_heap *heap = NULL;
	cache list_cache = NULL;


    while(fscanf(file, "%s", string) != EOF) {

     	if (strcmp(string, "init") == 0) { 
     	
     		//da specifica è giusto così: x è il numero di colonne e y il numero di righe
        	if(fscanf(file, "%d %d", &x, &y) != EOF) {

        		mat.x = x;
        		mat.y = y;
        		matC = castToAxial(mat);


				if(tabella != NULL) {
					liberaTabella(tabella);
				}
				tabella = creaHashTable();

				//METTO TUTTI I NODI A INF QUANDO CREO LA TABELLA
				int tot = mat.x * mat.y;

				nodes = malloc(tot * sizeof(DijkstraNode));

				// Inizializza NECESSARIO PER METTERE TUTTI I COSTI A INF
				for(int i = 0; i < tot; i++) {
					nodes[i].dist = INF;
					nodes[i].timestamp_global = 0;
					nodes[i].timestamp_local = 0;
				}

				if (heap == NULL) {
					heap = createHeap(10000); // Inizia con 100k elementi
				} else {
					heap->size = 0; // Se esiste già (nuova init), lo svuotiamo semplicemente
				}

				printf("OK\n");


        	}
        
        }
        
        
//FUNZIONA
        if (strcmp(string, "toggle_air_route") == 0) { 
        	t_std_coord coordA, coordB;
        	t_axial_coord hexA;
        	if(fscanf(file, "%d %d %d %d", &coordA.x, &coordA.y, &coordB.x, &coordB.y) != EOF) {
        		
        		//controllo la validità delle coordinate OK
				if(coordA.x < mat.x && coordA.x >= 0 && coordA.y < mat.y && coordA.y >= 0 &&
				coordB.x < mat.x && coordB.x >= 0 && coordB.y < mat.y && coordB.y >= 0 && tabella != NULL) {
				
					//tasformo in coordinate cubiche
					hexA = castToAxial(coordA);
					
					//se l'elemento non c'è nella tabella, lo creo
					if(cerca(tabella, createKey(hexA), hexA).costo == -1) {
						//creo una nuova entry nella tabella
						t_uscita new;
						new.coord = hexA;
						new.costo = 1;
						new.rotte = NULL;
						//printf("\n %d %d \n", coordB.x, coordB.y);
						addRotta(&new.rotte, coordB);
						//printf("new.rotte: ");
						//stampaRotte(new.rotte);
						inserisci(tabella, createKey(hexA), new);
						//printf("\nstampa rotte: ");
						//stampaRotte(cerca(tabella, createKey(hexA), hexA).rotte);
					}
					
					//aggiorno la tabella esistente
					else{
						lista_rotte test_rotta = sostituisci(tabella, createKey(hexA), hexA)->u.rotte;


						if(checkRotta(test_rotta, coordB)) {
							test_rotta = deleteRic(test_rotta, coordB);
							sostituisci(tabella, createKey(hexA), hexA)->u.rotte = test_rotta;
						}
						else if (contaRotte(test_rotta) < 5){
							addRotta(&test_rotta, coordB);
							sostituisci(tabella, createKey(hexA), hexA)->u.rotte = test_rotta;
							
						}
						else
							printf("KO\n");					
					}

				//tutto è stato creato correttamente	
				list_cache = libCache(list_cache);
				printf("OK\n");
				//stampaRotte(cerca(tabella, createKey(hexA), hexA).rotte);
				}
				
				else
					printf("KO\n");

        	}
        
        }

//FUNZIONA
        if (strcmp(string, "change_cost") == 0) { 
        	t_std_coord coord;
        	t_axial_coord exa;
        	if(fscanf(file, "%d %d %d %d", &coord.x, &coord.y, &v, &r) != EOF) {
        		exa = castToAxial(coord);
				

        		if(coord.x < mat.x && coord.y < mat.y && coord.x >= 0 && coord.y >= 0 && r > 0 && v >= -10 && v <= 10 && tabella != NULL) { 	//validità di v e r, validità delle coordinate
						visitCell2(exa, r, matC, tabella, v);
						list_cache = libCache(list_cache);
						printf("OK\n");
				}
				else
					printf("KO\n");
			}
		}

		if (strcmp(string, "travel_cost") == 0) { 
        	t_std_coord coordA, coordB;
        	t_axial_coord start, end;
			int dist;
        	if(fscanf(file, "%d %d %d %d", &coordA.x, &coordA.y, &coordB.x, &coordB.y) != EOF) {
        		start = castToAxial(coordA);
				end = castToAxial(coordB);

				
        		if(coordA.x < mat.x && coordA.y < mat.y && coordA.x >= 0 && coordA.y >= 0 && coordB.x < mat.x && coordB.y < mat.y && coordB.x >= 0 && coordB.y >= 0 && tabella != NULL) { //validità delle coordinate
					if(costoCache(list_cache, coordA, coordB) == -1) {
						dist = dijkstra(tabella, start, end, matC, nodes, &global_timestamp, heap);
					}
					else
						dist = costoCache(list_cache, coordA, coordB);
        			
        			if(dist >= 0) {
						addCache(&list_cache, coordA, coordB, dist);
						printf("%d\n", dist);
        				}
        				else
							printf("-1\n");
				}
				else
					printf("-1\n");
			}
		}

	}        
	list_cache = libCache(list_cache);
	destroyHeap(heap);
	free(nodes);
	liberaTabella(tabella);
	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TAM_FILA 5
#define TAM_PILHA 3

typedef struct {
    int id;
    char nome;
} Peca;

typedef struct {
    Peca pecas[TAM_FILA];
    int inicio;     
    int fim;        
    int quantidade; 
} Fila;

typedef struct {
    Peca pecas[TAM_PILHA];
    int topo;
} Pilha;

/* Para desfazer: guardamos um snapshot do estado anterior */
typedef struct {
    Fila fila;
    Pilha pilha;
    int valido;
} Snapshot;

/* tipos possíveis de peças */
char tipos[] = {'I', 'O', 'T', 'L', 'J', 'S', 'Z'};
int contadorID = 1;

/* Gera uma nova peça aleatória com id único */
Peca gerarPeca() {
    Peca nova;
    nova.id = contadorID++;
    nova.nome = tipos[rand() % (sizeof(tipos)/sizeof(tipos[0]))];
    return nova;
}

/* ---------- Funções da fila ---------- */
void inicializarFila(Fila *f) {
    f->inicio = 0;
    f->fim = 0;
    f->quantidade = 0;
    // preencher com TAM_FILA peças iniciais
    for (int i = 0; i < TAM_FILA; i++) {
        f->pecas[f->fim] = gerarPeca();
        f->fim = (f->fim + 1) % TAM_FILA;
        f->quantidade++;
    }
}

int filaVazia(Fila *f) {
    return f->quantidade == 0;
}

int filaCheia(Fila *f) {
    return f->quantidade == TAM_FILA;
}

/* enfileira p no fim (se houver espaço) */
void enfileirar(Fila *f, Peca p) {
    if (filaCheia(f)) {
        printf("[AVISO] Tentativa de enfileirar em fila cheia.\n");
        return;
    }
    f->pecas[f->fim] = p;
    f->fim = (f->fim + 1) % TAM_FILA;
    f->quantidade++;
}

/* desenfileira a peça da frente; retorna peça com id=0 e nome='?' se vazia */
Peca desenfileirar(Fila *f) {
    Peca vazio = {0, '?'};
    if (filaVazia(f)) return vazio;

    Peca removida = f->pecas[f->inicio];
    f->inicio = (f->inicio + 1) % TAM_FILA;
    f->quantidade--;
    return removida;
}

/* mostra conteúdo da fila, do elemento da frente até o fim lógico */
void mostrarFila(Fila *f) {
    printf("\n=== Fila (frente -> fim) ===\n");
    if (filaVazia(f)) {
        printf("[vazia]\n");
    } else {
        int idx = f->inicio;
        for (int c = 0; c < f->quantidade; c++) {
            Peca p = f->pecas[idx];
            printf("Pos %d: %c (ID %d)\n", c+1, p.nome, p.id);
            idx = (idx + 1) % TAM_FILA;
        }
    }
    printf("============================\n");
}

/* Reverte a ordem dos elementos da fila (frente vira fim e vice-versa) */
void inverterOrdemFila(Fila *f) {
    if (f->quantidade <= 1) return;
    Peca temp[TAM_FILA];
    int idx = f->inicio;
    for (int c = 0; c < f->quantidade; c++) {
        temp[c] = f->pecas[idx];
        idx = (idx + 1) % TAM_FILA;
    }
    
    for (int c = 0; c < f->quantidade; c++) {
        f->pecas[c] = temp[f->quantidade - 1 - c];
    }
    f->inicio = 0;
    f->fim = f->quantidade % TAM_FILA;
}

/* ---------- Funções da pilha ---------- */
void inicializarPilha(Pilha *p) {
    p->topo = -1;
}

int pilhaVazia(Pilha *p) {
    return p->topo == -1;
}

int pilhaCheia(Pilha *p) {
    return p->topo == TAM_PILHA - 1;
}

/* push (se não cheia) */
void push(Pilha *p, Peca x) {
    if (pilhaCheia(p)) {
        printf("[AVISO] Pilha cheia — não foi possível reservar a peça.\n");
        return;
    }
    p->pecas[++p->topo] = x;
}

/* pop retorna peça com id=0,nome='?' se vazia */
Peca pop(Pilha *p) {
    Peca vazio = {0, '?'};
    if (pilhaVazia(p)) return vazio;
    return p->pecas[p->topo--];
}

/* mostra pilha do topo para base */
void mostrarPilha(Pilha *p) {
    printf("\n=== Pilha de Reserva (topo -> base) ===\n");
    if (pilhaVazia(p)) {
        printf("[vazia]\n");
    } else {
        for (int i = p->topo; i >= 0; i--) {
            Peca q = p->pecas[i];
            printf("Nivel %d: %c (ID %d)\n", i+1, q.nome, q.id);
        }
    }
    printf("=======================================\n");
}

/* Reverte a ordem da pilha (topo e base trocam) */
void inverterOrdemPilha(Pilha *p) {
    if (p->topo <= 0) return;
    int i = 0, j = p->topo;
    while (i < j) {
        Peca tmp = p->pecas[i];
        p->pecas[i] = p->pecas[j];
        p->pecas[j] = tmp;
        i++; j--;
    }
    // topo continua apontando para o mesmo índice (mesma quantidade), apenas os elementos estão invertidos
}

/* ---------- Snapshot (undo) ---------- */
void salvarSnapshot(Snapshot *s, Fila *f, Pilha *p) {
    memcpy(&s->fila, f, sizeof(Fila));
    memcpy(&s->pilha, p, sizeof(Pilha));
    s->valido = 1;
}

void restaurarSnapshot(Snapshot *s, Fila *f, Pilha *p) {
    if (!s->valido) return;
    memcpy(f, &s->fila, sizeof(Fila));
    memcpy(p, &s->pilha, sizeof(Pilha));
    s->valido = 0; // só undo uma vez
}


/* Jogar peça: desenfileira a frente (mostra) e enfileira uma nova gerada */
void operacaoJogar(Fila *f, Pilha *p) {
    // f será modificado; pilha não
    Peca jogada = desenfileirar(f);
    if (jogada.id == 0) {
        printf("Fila vazia — nada para jogar.\n");
        return;
    }
    printf(">> Jogando peça: %c (ID %d)\n", jogada.nome, jogada.id);
    // sempre repor com uma nova peça para manter fila cheia
    enfileirar(f, gerarPeca());
}

/* Reservar: desenfileira frente e push na pilha (se houver espaço) e enfileira nova peça */
void operacaoReservar(Fila *f, Pilha *p) {
    if (pilhaCheia(p)) {
        printf("Pilha cheia — não é possível reservar mais peças.\n");
        return;
    }
    Peca frente = desenfileirar(f);
    if (frente.id == 0) {
        printf("Fila vazia — nada para reservar.\n");
        return;
    }
    push(p, frente);
    printf(">> Reservada peça: %c (ID %d)\n", frente.nome, frente.id);
    // repor fila
    enfileirar(f, gerarPeca());
}

/* Usar peça reservada: pop e mostrar (não altera fila) */
void operacaoUsarReservada(Pilha *p) {
    if (pilhaVazia(p)) {
        printf("Pilha vazia — sem peça reservada para usar.\n");
        return;
    }
    Peca usada = pop(p);
    printf(">> Usando peça reservada: %c (ID %d)\n", usada.nome, usada.id);
}

/* Trocar top da pilha com a frente da fila */
void operacaoTrocarTopoFila(Fila *f, Pilha *p) {
    if (pilhaVazia(p)) {
        printf("Pilha vazia — nada para trocar.\n");
        return;
    }
    if (filaVazia(f)) {
        printf("Fila vazia — nada para trocar.\n");
        return;
    }
    // acessar front index
    int idxFrente = f->inicio;
    // top index
    int idxTopo = p->topo;
    Peca tempFila = f->pecas[idxFrente];
    Peca tempPilha = p->pecas[idxTopo];

    // troca
    f->pecas[idxFrente] = tempPilha;
    p->pecas[idxTopo] = tempFila;

    printf(">> Trocado: pilha.top (%c ID %d) <-> fila.frente (%c ID %d)\n",
           f->pecas[idxFrente].nome, f->pecas[idxFrente].id,
           p->pecas[idxTopo].nome, p->pecas[idxTopo].id);
}

/* Inverter fila e pilha (reverter ordem interna de cada estrutura) */
void operacaoInverter(Fila *f, Pilha *p) {
    inverterOrdemFila(f);
    inverterOrdemPilha(p);
    printf(">> Inversão aplicada: fila e pilha tiveram suas ordens invertidas.\n");
}

/* ---------- Utilitários ---------- */
void mostrarEstado(Fila *f, Pilha *p) {
    mostrarFila(f);
    mostrarPilha(p);
}

/* ---------- Programa principal (menu) ---------- */
int main() {
    srand((unsigned) time(NULL));
    Fila fila;
    Pilha pilha;
    Snapshot snap;
    snap.valido = 0;

    inicializarFila(&fila);
    inicializarPilha(&pilha);

    int opcao;
    printf("=== Tetris — Nível Mestre ===\n");

    do {
        printf("\nMenu:\n");
        printf("1 - Jogar peça\n");
        printf("2 - Reservar peça\n");
        printf("3 - Usar peça reservada\n");
        printf("4 - Trocar topo da pilha com frente da fila\n");
        printf("5 - Desfazer (undo) ultima ação\n");
        printf("6 - Inverter fila e pilha\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        if (scanf("%d", &opcao) != 1) {
            // entrada inválida — limpar stdin
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        switch (opcao) {
            case 1:
                // salvar snapshot antes da ação
                salvarSnapshot(&snap, &fila, &pilha);
                operacaoJogar(&fila, &pilha);
                break;

            case 2:
                salvarSnapshot(&snap, &fila, &pilha);
                operacaoReservar(&fila, &pilha);
                break;

            case 3:
                salvarSnapshot(&snap, &fila, &pilha);
                operacaoUsarReservada(&pilha);
                break;

            case 4:
                salvarSnapshot(&snap, &fila, &pilha);
                operacaoTrocarTopoFila(&fila, &pilha);
                break;

            case 5:
                if (!snap.valido) {
                    printf("Nada para desfazer.\n");
                } else {
                    restaurarSnapshot(&snap, &fila, &pilha);
                    printf(">> Última ação desfeita (estado restaurado).\n");
                }
                break;

            case 6:
                salvarSnapshot(&snap, &fila, &pilha);
                operacaoInverter(&fila, &pilha);
                break;

            case 0:
                printf("Saindo...\n");
                break;

            default:
                printf("Opção inválida.\n");
                break;
        }

        // mostrar estado após cada tentativa de ação (exceto se o usuário digitou 0)
        if (opcao != 0) mostrarEstado(&fila, &pilha);

    } while (opcao != 0);

    return 0;
}

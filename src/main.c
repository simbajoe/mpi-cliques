#include <omp.h>
#include <pcontrol.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

struct graph {
    int *edges;
    int *ec;
    int *off;
    int size;
    double p;
    int me;
    int **neigs;
    int *nc;
};

double rand01() {
    return (double) rand() / RAND_MAX;
}

void print_edge(int *edge, int ecc) {
    int i;
    //printf("%d: ", ecc);
    for (i = 0; i < ecc; i++) {
        printf("%d ", edge[i]);
    }
    printf("\n");
}

void print_m(struct graph *g) {
    int i;
    for (i = 0; i < g->size; i++) {
        printf("%d: ", i);
        print_edge(g->edges + g->off[i], g->ec[i]);
    }
}

void print_n(struct graph *g) {
    int i;
    for (i = 0; i < g->size; i++) {
        printf("%d: ", i);
        print_edge(g->neigs[i], g->nc[i]);
    }
}

struct graph generate(int size, double p) {
    int me = 0;
    int ecc, a;
    int i, j;
    struct graph g;
    g.size = size;
    g.p = p;

    g.ec = (int *) malloc(size * sizeof(int));
    g.off = (int *) malloc(size * sizeof(int));
    g.nc = (int *) malloc(size * sizeof(int));
    g.neigs = (int **) malloc(size * sizeof(int *));

    for (i = 0; i < size; i++) {
        g.off[i] = me;
        ecc = (int) (rand01() * (g.size - i - 1) * p);
        me += ecc;
        g.ec[i] = ecc;
    }

    for (i = 0; i < size; i++) {
        g.neigs[i] = (int *) malloc(size * sizeof(int));
        g.nc[i] = 0;
    }

    g.edges = (int *) malloc(me * sizeof(int));
    for (i = 0; i < g.size; i++) {
        ecc = g.ec[i];
        a = 0;
        for (j = i + 1; j < g.size; j++) {
            if (ecc - a <= 0) break;
            if (rand01() < (double) (ecc - a) / (g.size - j)) {
                g.edges[g.off[i] + a] = j;
                a++;

                g.neigs[i][g.nc[i]++] = j;
                g.neigs[j][g.nc[j]++] = i;
            }
        }
    }

    for (i = 0; i < size; i++) {
        g.neigs[i] = realloc(g.neigs[i], g.nc[i] * sizeof(int));
    }

    g.me = me;

    return g;
}

void free_graph(struct graph *g) {
    int i;
    free(g->edges);
    free(g->ec);
    free(g->off);
    for (i = 0; i < g->size; i++) free(g->neigs[i]);
    free(g->neigs);
    free(g->nc);
}

int init_max(int n1, int n2, int **res) {
    int n;
    if (n1 > n2) {
        n = n1;
    } else {
        n = n2;
    }
    *res = (int *) malloc(n * sizeof(int));
    return n;
}

int intersec(int *a1, int n1, int *a2, int n2, int **res) {
    int c1 = 0, c2 = 0;
    int n = 0;
    int tmp_n;
    int *r;

    tmp_n = init_max(n1, n2, &r);

    while (c1 < n1 && c2 < n2) {
        if (a1[c1] == a2[c2]) {
            r[n] = a1[c1];
            c1++;
            c2++;
            n++;
        } else if (a1[c1] < a2[c2]) {
            c1++;
        } else {
            c2++;
        }
    }
    if (tmp_n > n) {
        r = realloc(r, n * sizeof(int));
    }
    *res = r;
    return n;
}

int get_new_c(int *c, int n, int val, int **res) {
    int i;
    int *r;
    r = (int *) malloc((n + 1) * sizeof(int));
    for (i = 0; i < n; i++) {
        r[i] = c[i];
    }
    r[n] = val;
    *res = r;
    return n + 1;
}

int array_union(int *a1, int n1, int *a2, int n2, int **res) {
    int c1 = 0, c2 = 0;
    int n = 0, i;
    int tmp_n;
    int *r;

    tmp_n = n1 + n2;
    r = (int *) malloc(tmp_n * sizeof(int));

    while (c1 < n1 && c2 < n2) {
        if (a1[c1] == a2[c2]) {
            r[n] = a1[c1];
            c1++;
            c2++;
        } else if (a1[c1] < a2[c2]) {
            r[n] = a1[c1];
            c1++;
        } else {
            r[n] = a2[c2];
            c2++;
        }
        n++;
    }
    if (c1 < n1) {
        for (i = c1; i < n1; i++) {
            r[n] = a1[i];
            n++;
        }
    } else if (c2 < n2) {
        for (i = c2; i < n2; i++) {
            r[n] = a2[i];
            n++;
        }
    }

    if (tmp_n > n) {
        r = realloc(r, n * sizeof(int));
    }

    *res = r;
    return n;
}

int array_sub(int *a1, int n1, int *a2, int n2, int **res) {
    int c1 = 0, c2 = 0;
    int n = 0, i;
    int tmp_n;
    int *r;

    tmp_n = n1;
    r = (int *) malloc(tmp_n * sizeof(int));

    while (c1 < n1 && c2 < n2) {
        if (a1[c1] == a2[c2]) {
            c1++;
            c2++;
        } else if (a1[c1] < a2[c2]) {
            r[n] = a1[c1];
            c1++;
            n++;
        } else {
            c2++;
        }
    }

    if (c1 < n1) {
        for (i = c1; i < n1; i++) {
            r[n] = a1[i];
            n++;
        }
    }

    if (tmp_n > n) {
        r = realloc(r, n * sizeof(int));
    }
    *res = r;
    return n;
}

int build_neigs(int *c, int n, struct graph *g, int **res) {
    int i;
    int *neigs, *neigs2;
    int nn = 0, nn2 = 0;
    for (i = 0; i < n; i++) {
        nn2 = array_union(neigs, nn, g->neigs[c[i]], g->nc[c[i]], &neigs2);
        if (nn2 <= nn) {
            if (nn2) free(neigs2);
        }
        else {
            if (nn) free(neigs);
            neigs = neigs2;
            nn = nn2;
        }
    }
    nn2 = array_sub(neigs, nn, c, n, &neigs2);
    if (nn2 <= nn) {
        if (nn2) free(neigs2);
    }
    else {
        if (nn) free(neigs);
        neigs = neigs2;
        nn = nn2;
    }
    *res = neigs;
    return nn;
}

int array_in(int *a1, int n1, int *a2, int n2) {
    int c1 = 0, c2 = 0;
    while (c1 < n1 && c2 < n2 && a2[c2] <= a1[c1]) {
        if (a2[c2] == a1[c1]) {
            c1++;
            c2++;
        } else {
            c2++;
        }
    }
    return c1 == n1;
}

int check_candidate(int *c, int n, struct graph *g) {
    int *neigs;
    int i;
    int res = 1;
    int nn = build_neigs(c, n, g, &neigs);
    if (nn) {
        for (i = 0; i < nn; i++) {
            if (array_in(c, n, g->neigs[neigs[i]], g->nc[neigs[i]])) {
                res = 0;
                break;
            }
        }
        free(neigs);
    }
    return res;
}

int cpc(int ***a1, int *n1, int **ns1, int **a2, int n2, int *ns2) {
    int i, j;
    if (!*n1) {
        *a1 = (int **) malloc(n2 * sizeof(int *));
        *ns1 = (int *) malloc(n2 * sizeof(int));
    } else {
        *a1 = realloc(*a1, (*n1 + n2) * sizeof(int *));
        *ns1 = realloc(*ns1, (*n1 + n2) * sizeof(int));
    }

    for (i = 0; i < n2; i++) {
        (*ns1)[*n1 + i] = ns2[i];
        (*a1)[*n1 + i] = (int *) malloc(ns2[i] * sizeof(int));
        for (j = 0; j < ns2[i]; j++) {
            (*a1)[*n1 + i][j] = a2[i][j];
        }
    }
    *n1 += n2;
}

void free_cl(int **r, int *ns, int n) {
    int i;
    if (!n) return;
    for (i = 0; i < n; i++) {
        free(r[i]);
    }
    free(r);
    free(ns);
}

void clique(int *c, int cn, int *t, int tn, struct graph *g, int ***res, int **rns, int *nn) {
    int i, u, j;
    int new_tn, new_cn;
    int *new_t, *new_c;
    for (i = 0; i < tn; i++) {
        u = t[i];
        new_cn = get_new_c(c, cn, u, &new_c);
        new_tn = intersec(t + i, tn - i, g->edges + g->off[u], g->ec[u], &new_t);
        if (new_tn) {
            clique(new_c, new_cn, new_t, new_tn, g, res, rns, nn);
        } else {
            if (check_candidate(new_c, new_cn, g)) {
                cpc(res, nn, rns, &new_c, 1, &new_cn);
            }
        }
        free(new_c);
        free(new_t);
    }
}

void clique_i(int i, struct graph *g, int ***res, int **rns, int *nn) {
    clique(&i, 1, g->edges + g->off[i], g->ec[i], g, res, rns, nn);
}

int main (argc, argv)
    int argc;
    char *argv[];
{
    MPI_Status status;
    int rank, m;

    MPI_Init (&argc, &argv);  /* starts MPI */
    MPI_Pcontrol(TRACEFILES, NULL, "trace", 0);
    MPI_Pcontrol(TRACELEVEL, 1, 1, 1);
    MPI_Pcontrol(TRACENODE, 1000000, 1, 1);

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);    /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &m);    /* get number of processes */

    struct graph g;
    int i, j, t;
    int **res;
    int *rns;
    int rn = 0;

    if (rank == 0) {
	int size = 8000;
	double p = 0.01;
	srand( time(NULL) );
	g = generate(size, p);

/*
	printf("M\n");
	print_m(&g);
	printf("Neighbors\n");
	print_n(&g);
*/
	printf("%d vertices\n", g.size);
	printf("%d edges\n", g.me);
    }

    MPI_Bcast(&g.size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&g.me, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) {
	g.ec = (int *) malloc(g.size * sizeof(int));
	g.off = (int *) malloc(g.size * sizeof(int));
	g.nc = (int *) malloc(g.size * sizeof(int));
	g.neigs = (int **) malloc(g.size * sizeof(int *));
	g.edges = (int *) malloc(g.me * sizeof(int));
    }
    MPI_Bcast(g.ec, g.size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(g.off, g.size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(g.nc, g.size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(g.edges, g.me, MPI_INT, 0, MPI_COMM_WORLD);

    for (i = 0; i < g.size; i++) {
	if (rank != 0) {
	    g.neigs[i] = (int *) malloc(g.nc[i] * sizeof(int));
	}
	MPI_Bcast(g.neigs[i], g.nc[i], MPI_INT, 0, MPI_COMM_WORLD);
    }
	double start_time = MPI_Wtime();
	MPI_Pcontrol(TRACEEVENT, "entry", 1, 0, NULL);
    for (i = 0; i < g.size / m + 1; i++) {
	if (i % 2) t = (i + 1) * m - rank - 1;
	else t = i * m + rank;
	if (t < g.size) {
            clique_i(t, &g, &res, &rns, &rn);
	}
	//printf("rank %d. ready: %d\n", rank, i + 1);
    }
	MPI_Pcontrol(TRACEEVENT, "exit", 1, 0, NULL);
    if (rank != 0) {
	MPI_Ssend(&rn, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
/*	if (rn) {
	    MPI_Ssend(rns, rn, MPI_INT, 0, 2, MPI_COMM_WORLD);
	    for (i = 0; i < rn; i++) {
		MPI_Ssend(res[i], rns[i], MPI_INT, 0, 3, MPI_COMM_WORLD);
	    }
	}*/
    } else {
	int an = rn;
	printf("run time: %lf\n", MPI_Wtime() - start_time);
	int trn;
	int *trns;
	int **tres;
	for (i = 1; i < m; i++) {
	    MPI_Recv(&trn, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
	    an += trn;
	    /*if (trn) {
		trns = (int *) malloc(trn * sizeof(int));
		MPI_Recv(trns, trn, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
		tres = (int **) malloc(trn * sizeof(int *));
		for (j = 0; j < trn; j++) {
		   tres[j] = (int *) malloc(trns[j] * sizeof(int));
		   MPI_Recv(tres[j], trns[j], MPI_INT, i, 3, MPI_COMM_WORLD, &status);
		}
		cpc(&res, &rn, &rns, tres, trn, trns);
	        free_cl(tres, trns, trn);
	    }*/
	}
	printf("total cliques: %d\n", an);
/*
	for (i = 0; i < rn; i++) {
	    print_edge(res[i], rns[i]);
	}
*/
    }

    free_cl(res, rns, rn);
    free_graph(&g);

    MPI_Finalize();
    return 0;
}

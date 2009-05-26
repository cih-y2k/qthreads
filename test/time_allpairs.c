#ifdef HAVE_CONFIG_H
# include "config.h" /* for _GNU_SOURCE */
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <qthread/qthread.h>
#include <qthread/allpairs.h>
#include <qtimer.h>

size_t ASIZE = 1026;

aligned_t hamming = (aligned_t) - 1;

static void assigni(qthread_t * me, const size_t startat, const size_t stopat,
		    qarray * q, void *arg)
{
    int *ptr = qarray_elem_nomigrate(q, startat);

    for (size_t i = 0; i < (stopat - startat); i++) {
	ptr[i] = (i + startat);
    }
}

static void assignrand(qthread_t * me, const size_t startat,
		       const size_t stopat, qarray * q, void *arg)
{
    int *ptr = qarray_elem_nomigrate(q, startat);

    for (size_t i = 0; i < (stopat - startat); i++) {
	ptr[i] = random();
    }
}

static void printout(int *restrict * restrict out)
{
    size_t i;

    for (i = 0; i < ASIZE; i++) {
	size_t j;

	for (j = 0; j < ASIZE; j++) {
	    if (out[i][j] == -1) {
		printf("       _ ");
	    } else {
		printf("%8i ", out[i][j]);
	    }
	    assert(out[i][j] == out[j][i]);
	}
	printf("\n");
    }
}

static void mult(const int *inta, const int *intb, int *restrict out)
{
    assert(*out == -1);
    *out = (*inta) * (*intb);
}

static void hammingdist(const int *inta, const int *intb)
{
    unsigned int ham = *inta ^ *intb;
    aligned_t hamdist = 0;
    qthread_t *me = qthread_self();

    while (ham != 0) {
	hamdist += ham & 1;
	ham >>= 1;
    }
    if (hamming > hamdist) {
	qthread_lock(me, &hamming);
	if (hamming > hamdist) {
	    hamming = hamdist;
	}
	qthread_unlock(me, &hamming);
    }
}

int main(int argc, char *argv[])
{
    qarray *a1, *a2;
    int **out;
    size_t i;
    int threads = 0;
    int interactive = 0;
    qthread_t *me;
    qtimer_t timer = qtimer_new();

    if (argc >= 2) {
	threads = strtol(argv[1], NULL, 0);
	if (threads < 0) {
	    threads = 0;
	    interactive = 0;
	} else {
	    interactive = 1;
	}
    }
    if (argc >= 3) {
	ASIZE = strtol(argv[2], NULL, 0);
    }

    qthread_init(threads);
    me = qthread_self();

    a1 = qarray_create_tight(ASIZE, sizeof(int));
    a2 = qarray_create_tight(ASIZE, sizeof(int));
    qarray_iter_loop(me, a1, 0, ASIZE, assigni, NULL);
    qarray_iter_loop(me, a2, 0, ASIZE, assigni, NULL);

    out = calloc(ASIZE, sizeof(int *));
    assert(out);
    for (i = 0; i < ASIZE; i++) {
	size_t j;
	out[i] = calloc(sizeof(int), ASIZE);
	assert(out[i]);
	for (j = 0; j < ASIZE; j++) {
	    out[i][j] = -1;
	}
    }

    qtimer_start(timer);
    qt_allpairs_output(a1, a2, (dist_out_f) mult, (void **)out, sizeof(int));
    qtimer_stop(timer);
    printf("mult time: %f\n", qtimer_secs(timer));
    for (i = 0; i < ASIZE; i++) {
	free(out[i]);
    }
    free(out);

    /* trial #2 */
    qarray_iter_loop(me, a1, 0, ASIZE, assignrand, NULL);
    qarray_iter_loop(me, a2, 0, ASIZE, assignrand, NULL);

    qtimer_start(timer);
    qt_allpairs(a1, a2, (dist_f) hammingdist);
    qtimer_stop(timer);

    printf("hamming time: %f\n", qtimer_secs(timer));
    assert(hamming > 0);

    qarray_destroy(a1);
    qarray_destroy(a2);

    qthread_finalize();
    return 0;
}

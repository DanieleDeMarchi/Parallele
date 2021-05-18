#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <stdbool.h>
#include <omp.h> //openMp library

#include "verbose.h"

/** =====================================================
*		Pars args con argp.h
*	=====================================================
*/
static char doc[] = "Approssima pigreco con openmp.\nOPTIONS:";
static char args_doc[] = "";
static struct argp_option options[] = {
	{"verbose", 'v', 0, 0, "verbose"},
	{"points", 'p', "[int_value]", 0, "numero di punti, default=10^8"},
	{"seed", 's', "[int_value]", 0, "seed per random number generator. default=123456"},
	{"threads", 't', "[int_value]", 0, "numero di thread paralleli. default=8"},
	{0}};

struct arguments
{
	int npoints;
	int seed;
	int threads;
	bool verbose;
};

static error_t parser(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	switch (key)
	{
	case 'v':
		arguments->verbose = true;
		break;
	case 'p':
		if (!(atoi(arg)))
		{
			printf("n not a num\n");
			argp_usage(state);
			return EINVAL;
		}
		arguments->npoints = atoi(arg);
		break;
	case 't':
		if (!(atoi(arg)))
		{
			printf("n not a num\n");
			argp_usage(state);
			return EINVAL;
		}
		arguments->threads = atoi(arg);
		break;
	case 's':
		if (!(atoi(arg)))
		{
			printf("s not a num\n");
			argp_usage(state);
			return EINVAL;
		}
		arguments->seed = atoi(arg);
		break;
	case ARGP_KEY_ARG:
		return 0;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parser, args_doc, doc, 0, 0, 0};

/** ####################################################			
 * 						MAIN
 *  ####################################################
 */
int main(int argc, char **argv)
{

	/** =====================================================
	*		PARSE ARGS
	*	=====================================================
	*/
	struct arguments arguments;

	arguments.verbose = false;
	arguments.npoints = 100000000;
	arguments.threads = 8;
	arguments.seed = 123456;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	/** =====================================================
	*		INIT VALS
	*	=====================================================
	*/
	arguments.verbose ? setVerbose(true) : setVerbose(false);

	int sample_points_per_thread, threads, num_threads, sum;
	double rand_no_x, rand_no_y;

	int npoints = arguments.npoints;
	int seed = arguments.seed;
	threads = arguments.threads;

	printf("num points: %d\n", npoints);
	printf("\n================\n\n");

	/** =====================================================
	*		CODICE SERIALE
	*	=====================================================
	*/

	{
		num_threads = omp_get_num_threads();
		sample_points_per_thread = npoints / num_threads;
		sum = 0;

		for (int i = 0; i < sample_points_per_thread; i++)
		{
			rand_no_x = ((double)rand_r(&seed)) / (double)RAND_MAX;
			rand_no_y = ((double)rand_r(&seed)) / (double)RAND_MAX;

			if (((rand_no_x) * (rand_no_x) +
				 (rand_no_y) * (rand_no_y)) < 1)
				sum++;
		}
	}

	printf("sum sequenziale: %d \n", sum);
	printf("pi sequenziale: %f\n", (4 * (double)sum) / (double)npoints);

	/** =====================================================
	*		CODICE PARALLELO
	*	=====================================================
	*/

	printf("\n================\n\n");

	sum = 0;

	printf("Num threads: %d\n", threads);
#pragma omp parallel private(rand_no_x, rand_no_y, num_threads, sample_points_per_thread, seed) \
	shared(npoints) reduction(+                                                                 \
							  : sum) num_threads(threads)
	{
		num_threads = omp_get_num_threads();
		sample_points_per_thread = npoints / num_threads;
		seed = arguments.seed;
		seed += omp_get_thread_num();

		verbose("seed thread %d before loop: %d\n", omp_get_thread_num(), seed);

		//printf("num_threads: %d \nsample_points_per_thread: %d\n", num_threads, sample_points_per_thread);
		for (int i = 0; i < sample_points_per_thread; i++)
		{
			rand_no_x = ((double)rand_r(&seed)) / (double)RAND_MAX;
			rand_no_y = ((double)rand_r(&seed)) / (double)RAND_MAX;

			if (((rand_no_x - 0.5) * (rand_no_x - 0.5) +
				 (rand_no_y - 0.5) * (rand_no_y - 0.5)) < 0.25)
				sum++;
		}

		verbose("seed thread  %d after loop: %d\n", omp_get_thread_num(), seed);
		verbose("sum thread %d: %d\n", omp_get_thread_num(), sum);
	}
	verbose("\n");
	printf("sum parallelo: %d \n", sum);
	printf("pi parallelo: %f\n", (4 * (double)sum) / (double)npoints);

#if false

	/** =====================================================
	*		CODICE PARALLELO senza reduction
	*	=====================================================
	*/

	printf("\n================\n\n");

	sum = 0;
	num_threads = 8;
	int *sumArray = (int *)calloc(num_threads, sizeof(int));

#pragma omp parallel private(rand_no_x, rand_no_y, num_threads, sample_points_per_thread, seed) \
	shared(npoints, sumArray) num_threads(8)
	{
		num_threads = omp_get_num_threads();
		sample_points_per_thread = npoints / num_threads;
		int thread = omp_get_thread_num();
		seed = arguments.seed;
		seed += omp_get_thread_num();

		//verbose("seed thread %d: %d\n", omp_get_thread_num(), seed);

		//printf("num_threads: %d \nsample_points_per_thread: %d\n", num_threads, sample_points_per_thread);
		for (int i = 0; i < sample_points_per_thread; i++)
		{
			rand_no_x = ((double)rand_r(&seed)) / (double)RAND_MAX;
			rand_no_y = ((double)rand_r(&seed)) / (double)RAND_MAX;

			if (((rand_no_x - 0.5) * (rand_no_x - 0.5) +
				 (rand_no_y - 0.5) * (rand_no_y - 0.5)) < 0.25)
				sumArray[thread]++;
		}

		//verbose("seed thread  %d after: %d\n", omp_get_thread_num(), seed);
		//printf("seed thread  %d after: \n", omp_get_thread_num());
		verbose("sum thread %d: %d\n", omp_get_thread_num(), sumArray[thread]);
	}

	for (int i = 0; i < num_threads; i++)
	{
		sum += sumArray[i];
	}

	printf("sum parallelo senza redux: %d \n", sum);
	printf("pi parallelo senza redux: %f\n", (4 * (double)sum) / (double)npoints);

#endif

	return EXIT_SUCCESS;
}

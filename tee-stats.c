/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <tee_client_api.h>

static int verbosity;
static int clear;

#define _verbose(lvl, ...)			\
	do {					\
		if (verbosity >= lvl) {		\
			printf(__VA_ARGS__);	\
			fflush(stdout);		\
		}				\
	} while (0)

#define verbose(...)  _verbose(1, __VA_ARGS__)
#define vverbose(...) _verbose(2, __VA_ARGS__)

/*
 * TA interface. Keep in sync with OP-TEE OS!
 *  - core/include/mm/tee_mm.h
 *  - core/arch/arm/sta/stats.c
 */

#define STATS_UUID { 0xd96a5b40, 0xe2c7, 0xb1af, \
			{ 0x87, 0x94, 0x10, 0x02, 0xa5, 0xd5, 0xc6, 0x1b } }

#define TEE_MM_POOL_DESC_LENGTH 32
struct tee_mm_pool_stats {
        char desc[TEE_MM_POOL_DESC_LENGTH];
        uint32_t allocated;
        uint32_t max_allocated;
        uint32_t size;
};

#define STATS_CMD_ALLOC_STATS 1

/* End of TA interface */

static void errx(const char *msg, TEEC_Result res)
{
	fprintf(stderr, "%s: 0x%08x\n", msg, res);
	exit (1);
}

static void check_res(TEEC_Result res, const char *errmsg)
{
	if (res != TEEC_SUCCESS)
		errx(errmsg, res);
}

#define _TO_STR(x) #x
#define TO_STR(x) _TO_STR(x)

static void usage(const char *progname)
{
	fprintf(stderr, "Statistics gathering tool for OP-TEE (%s)\n\n",
		TO_STR(VERSION));
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s -h\n", progname);
	fprintf(stderr, "  %s [-v] [-c] [<what>]\n\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -c    Clear counters after getting them\n");
	fprintf(stderr, "  -h    Print this help and exit\n");
	fprintf(stderr, "  -v    Be verbose (use twice for greater effect)\n\n");
	fprintf(stderr, "Values for <what>:\n");
	fprintf(stderr, "  heap  Print statistics about the heap (default)\n");
}

static void get_stats()
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	uint32_t ret_origin;
	TEEC_UUID uuid = STATS_UUID;
	struct tee_mm_pool_stats st;

	res = TEEC_InitializeContext(NULL, &ctx);
	check_res(res,"TEEC_InitializeContext");

	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL,
			       NULL, &ret_origin);
	if (res == TEEC_ERROR_ITEM_NOT_FOUND) {
		fprintf(stderr, "TEEC_OpenSession: TEEC_ERROR_ITEM_NOT_FOUND "
			"(did you forget to build OP-TEE with "
			"CFG_WITH_STATS=y?)\n");
		exit(1);
		
	}
	check_res(res,"TEEC_OpenSession");


	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = 1; /* Pool ID */
	op.params[0].value.b = clear;
	op.params[1].tmpref.buffer = (void *)&st;
	op.params[1].tmpref.size = sizeof(st);

	res = TEEC_InvokeCommand(&sess, STATS_CMD_ALLOC_STATS, &op,
				 &ret_origin);
	check_res(res, "TEEC_InvokeCommand");

	printf("HEAP: cur_alloc %u max_alloc %u heap_size %u\n",
	        st.allocated, st.max_allocated, st.size);

	TEEC_CloseSession(&sess);
}

#define NEXT_ARG(i) \
	do { \
		if (++i == argc) { \
			fprintf(stderr, "%s: %s: missing argument\n", \
				argv[0], argv[i-1]); \
			return 1; \
		} \
	} while (0);

int main(int argc, char *argv[])
{
	int i;
	struct timespec ts;

	/* Parse command line */
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h")) {
			usage(argv[0]);
			return 0;
		}
	}
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-c")) {
			clear = 1;
		} else if (!strcmp(argv[i], "-v")) {
			verbosity++;
		} else if (!strcmp(argv[i], "heap")) {
			/* This is the default command */
		} else {
			fprintf(stderr, "%s: invalid argument: %s\n",
				argv[0], argv[i]);
			usage(argv[0]);
			return 1;
		}
	}

	vverbose("tee-stats version %s\n", TO_STR(VERSION));

	get_stats();

	return 0;
}

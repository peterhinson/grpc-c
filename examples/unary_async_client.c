/*
 * Copyright (c) 2017, Juniper Networks, Inc.
 * All rights reserved.
 */

#include <stdio.h>
#include "foo.grpc-c.h"

static grpc_c_client_t *client;
static int done = 0;

static void *
test_check (void *arg)
{
    while (done == 0) {};

    grpc_c_client_free(client);
    grpc_c_shutdown();

    return NULL;
}

static void 
cb (grpc_c_context_t *context, void *tag, int success)
{
    foo__HelloReply *r;
    if (context->gcc_stream->read(context, (void **)&r, -1)) {
	printf("Failed to read\n");
	exit(1);
    }

    if (r) {
	printf("Got back: %s\n", r->message);
	int status = context->gcc_stream->finish(context, NULL);
	printf("Finished with %d\n", status);
	done = 1;
    }
}

/*
 * Takes as argument the socket name
 */
int 
main (int argc, char **argv) 
{
    if (argc < 2) {
	fprintf(stderr, "Too few arguments\n");
	exit(1);
    }

    /*
     * Initialize grpc-c library to be used with vanilla grpc
     */
    grpc_c_init(GRPC_THREADS, NULL);

    /*
     * Create a client object with client name as foo client to be talking to
     * a insecure server
     */
    client = grpc_c_client_init(argv[1], "unary async client", NULL, NULL);

    /*
     * Create a hello request message and call RPC
     */
    foo__HelloRequest h;
    foo__hello_request__init(&h);
    foo__HelloReply *r;

    char str[BUFSIZ];
    snprintf(str, BUFSIZ, "world");
    h.name = str;

    /*
     * This will invoke a async RPC
     */
    foo__greeter__say_hello__async(client, NULL, &h, &cb, (void *)1);

    pthread_t thr;
    pthread_create(&thr, NULL, test_check, NULL);

    grpc_c_client_wait(client);
}

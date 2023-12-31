#include "client_lib.h"
#include "client_server_shared.h"
#include <errno.h>
#include <time.h>

client_ctx_t *setup_client(struct sockaddr_in *client_addr,
                           struct sockaddr_in *server_addr) {

    int rc = -1;

    // Allocate a connection object
    client_ctx_t *ctx = calloc(1, sizeof(client_ctx_t));
    API_NULL(
        ctx, { return (NULL); }, "Unable to allocate client ctx obj\n");

    ctx->fd = socket(AF_INET, SOCK_STREAM, 0);
    API_STATUS(
        ctx->fd,
        {
            free(ctx);
            return (NULL);
        },
        "Unable to open SOCK_STREAM connection\n");

    // Bind a connection to an client IP address
    rc = bind(ctx->fd, (struct sockaddr *)(client_addr),
              sizeof(struct sockaddr_in));
    API_STATUS(
        rc,
        {
            close(ctx->fd);
            free(ctx);
            return (NULL);
        },
        "Unable to bind connection to IP address: %s\n",
        inet_ntoa((client_addr->sin_addr)));

    // Connect to server IP address
    rc = connect(ctx->fd, (struct sockaddr *)(server_addr),
                 sizeof(struct sockaddr_in));
    API_STATUS(
        rc,
        {
            close(ctx->fd);
            free(ctx);
            return (NULL);
        },
        "Unable to connect to IP address: %s. Reason: %s\n",
        inet_ntoa((server_addr->sin_addr)), strerror(errno));

    ctx->msgbuf = (void *)calloc(1, sizeof(msgbuf_t));
    API_NULL(
        ctx->msgbuf,
        {
            close(ctx->fd);
            free(ctx);
            return (NULL);
        },
        "Unable to allocate client-side tx/rx msg buf\n");

    return (ctx);
}

ssize_t tx_client(void **buf) {
    ssize_t i = 0;
    ssize_t nbytes = 64;
    // Randomize Tx on client before send
    // Print Tx on client for debugging
    srand(time(NULL));
    printf("\n==========[TX START]===========\n");
    for (i = 0; i < nbytes; i++) {
        if (i % 8 == 0) {
            printf("\n");
        }

        *(uint8_t *)(*buf + i) = (rand() % 256);
        printf("%02x \t", *(uint8_t *)(*buf + i));
    }
    printf("\n==========[TX END]============\n");

    return (nbytes);
}

int rx_client(void **buf, ssize_t nbytes) {
    ssize_t i = 0;
    // Print Rx on client after recv
    // Randomize Tx on client before send
    srand(time(NULL));
    printf("\n==========[RX START]===========\n");
    for (i = 0; i < nbytes; i++) {
        if (i % 8 == 0) {
            printf("\n");
        }

        printf("%02x \t", *(uint8_t *)(*buf + i));
        *(uint8_t *)(*buf + i) = (rand() % 256);
    }
    printf("\n==========[RX END]============\n");

    return (0);
}

void print_client(client_ctx_t *ctx, uint16_t rank, int iterations,
                  long unsigned int xfer_sz) {

    uint64_t sum = 0;
    printf("\n================================\n");
    printf("[RANK]: %u\n", rank);
    printf("[ITERATIONS]: %d\n", iterations);
    printf("[XFER SIZE]: %lu\n", xfer_sz);
    for (int i = 0; i < ctx->cur_elapsed; i++) {
        sum += ctx->elapsed[i];
    }

    printf("[RTT AVG LATENCY ELAPSED (nsec)]: %.2f\n",
           ((double)(sum) / (double)(ctx->cur_elapsed)));
    printf("\n================================\n");
}

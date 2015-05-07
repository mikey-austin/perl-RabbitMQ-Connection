#ifndef XS_UTILS_H
#define XS_UTILS_H

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "utils.h"

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#define RMQC_OK  0
#define RMQC_ERR 1

struct rmqc {
    amqp_connection_state_t con;
    HV *options;
    int *channels;
    int max_channels;
    int num_channels;
};

typedef struct rmqc rmqc_t;
typedef rmqc_t * RabbitMQ__Connection;

extern int rmqc_new(rmqc_t **self, HV *args);

extern int rmqc_destroy(rmqc_t *self);

extern char *rmqc_declare_queue(rmqc_t *self, HV *args);

extern int rmqc_bind(rmqc_t *self, HV *args);

extern int rmqc_consume(rmqc_t *self, HV *args);

extern SV *rmqc_receive(rmqc_t *self);

extern int rmqc_connect(rmqc_t *self);

extern int rmqc_close(rmqc_t *self);

#endif

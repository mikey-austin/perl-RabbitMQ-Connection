#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define DEFAULT_PORT      5672
#define DEFAULT_TLS       0
#define DEFAULT_USER      "guest"
#define DEFAULT_PASS      "guest"
#define DEFAULT_VHOST     "/"
#define DEFAULT_CHANNEL   1
#define DEFAULT_EXCHANGE  "amq.direct"
#define DEFAULT_KEY       "#"
#define DEFAULT_NO_LOCAL  0
#define DEFAULT_EXCLUSIVE 0
#define DEFAULT_NO_ACK    1

#define FRAME_MAX    131072
#define HEARTBEAT    0

static int close_channel(rmqc_t *self, int channel);
static int channel_exists(rmqc_t *self, int channel);
static void store_channel(rmqc_t *self, int channel);
static int fetch_int(HV *h, char *key, int *val); 
static int fetch_str(HV *h, char *key, char **val);

extern int
rmqc_new(rmqc_t **self, HV *args)
{
    /* Validate the connection parameters. */
    if(!hv_exists(args, "host", strlen("host")))
        croak("the host is required");

    if(!hv_exists(args, "port", strlen("port")))
        hv_store(args, "port", strlen("port"), newSVuv(DEFAULT_PORT), 0);

    if(!hv_exists(args, "tls", strlen("tls")))
        hv_store(args, "tls", strlen("tls"), newSViv(0), 0);

    if(!hv_exists(args, "user", strlen("user")))
        hv_store(args, "user", strlen("user"), newSVpv(DEFAULT_USER, strlen(DEFAULT_USER)), 0);

    if(!hv_exists(args, "pass", strlen("pass")))
        hv_store(args, "pass", strlen("pass"), newSVpv(DEFAULT_PASS, strlen(DEFAULT_PASS)), 0);

    if(!hv_exists(args, "vhost", strlen("vhost")))
        hv_store(args, "vhost", strlen("vhost"), newSVpv(DEFAULT_VHOST, strlen(DEFAULT_VHOST)), 0);

    if(!hv_exists(args, "max_channels", strlen("max_channels")))
        hv_store(args, "max_channels", strlen("max_channels"), newSViv(1), 0);

    *self = calloc(1, sizeof(**self));
    if(*self == NULL)
        croak("could not initialize instance");

    (*self)->con = NULL;
    (*self)->options = args;
    (*self)->num_channels = 0;

    fetch_int(args, "max_channels", &(*self)->max_channels);
    if(((*self)->channels = calloc((*self)->max_channels, sizeof(int))) == NULL)
        croak("could not initialize list of connections");

    return RMQC_OK;
}

/*
 * Establish a connection and login.
 */
extern int
rmqc_connect(rmqc_t *self)
{
    amqp_socket_t *socket = NULL;
    amqp_rpc_reply_t reply;
    int status, port;
    char *host, *user, *pass, *vhost;

    self->con = amqp_new_connection();
    if(!(socket = amqp_tcp_socket_new(self->con)))
        croak("could not create socket");

    fetch_str(self->options, "host", &host);
    fetch_int(self->options, "port", &port);
    status = amqp_socket_open(socket, host, port);
    if(status != 0)
        croak("open socket to %s port %d", host, port);

    fetch_str(self->options, "user", &user);
    fetch_str(self->options, "pass", &pass);
    fetch_str(self->options, "vhost", &vhost);
    reply = amqp_login(self->con, vhost, self->max_channels,
                       FRAME_MAX, HEARTBEAT, AMQP_SASL_METHOD_PLAIN,
                       user, pass);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("login failed for user %s, vhost %s", user, vhost);

    return RMQC_OK;
}

/*
 * Open the specified channel (defaults to 1) and declare a queue on
 * a connected connection.
 */
extern char
*rmqc_declare_queue(rmqc_t *self, HV *args)
{
    amqp_rpc_reply_t reply;
    amqp_bytes_t queue;
    char *queue_name = NULL;
    int channel, passive, durable, exclusive, auto_delete;

    if(fetch_int(args, "channel", &channel) != RMQC_OK)
        channel = DEFAULT_CHANNEL;

    amqp_channel_open(self->con, channel);
    store_channel(self, channel);
    reply = amqp_get_rpc_reply(self->con);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("failed to open channel");
    
    if(fetch_int(args, "passive", &passive) != RMQC_OK)
        passive = 0;
    if(fetch_int(args, "durable", &durable) != RMQC_OK)
       durable = 0;
    if(fetch_int(args, "exclusive", &exclusive) != RMQC_OK)
        exclusive = 0;
    if(fetch_int(args, "auto_delete", &auto_delete) != RMQC_OK)
        auto_delete = 1;
    
    fetch_str(args, "queue", &queue_name);
    
    queue = (queue_name ? amqp_cstring_bytes(queue_name) : amqp_empty_bytes);
    amqp_queue_declare(self->con, channel, queue, passive, durable, exclusive,
                       auto_delete, amqp_empty_table);
    reply = amqp_get_rpc_reply(self->con);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("failed to declare queue");

    return queue.bytes;
}

extern int
rmqc_bind(rmqc_t *self, HV *args)
{
    amqp_rpc_reply_t reply;
    amqp_bytes_t queue;
    char *queue_name = NULL, *exchange = NULL, *key = NULL;
    int channel;

    if(fetch_int(args, "channel", &channel) != RMQC_OK)
        channel = DEFAULT_CHANNEL;
    if(!channel_exists(self, channel))
        croak("channel %d has not been opened", channel);

    if(fetch_str(args, "exchange", &exchange) != RMQC_OK)
        exchange = DEFAULT_EXCHANGE;
    if(fetch_str(args, "key", &key) != RMQC_OK)
        key = DEFAULT_KEY;

    fetch_str(args, "queue", &queue_name);
    queue = (queue_name ? amqp_cstring_bytes(queue_name) : amqp_empty_bytes);

    amqp_queue_bind(self->con, channel, queue, amqp_cstring_bytes(exchange),
                    amqp_cstring_bytes(key), amqp_empty_table);
    reply = amqp_get_rpc_reply(self->con);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("failed to bind on channel %d", channel);

    return RMQC_OK;
}

extern int
rmqc_consume(rmqc_t *self, HV *args)
{
    amqp_rpc_reply_t reply;
    amqp_bytes_t queue, consumer_tag;
    char *queue_name = NULL, *tag_name = NULL;
    int channel, no_local, no_ack, exclusive;

    if(fetch_int(args, "channel", &channel) != RMQC_OK)
        channel = DEFAULT_CHANNEL;
    if(!channel_exists(self, channel))
        croak("channel %d has not been opened", channel);

    if(fetch_int(args, "no_local", &no_local) != RMQC_OK)
        no_local = DEFAULT_NO_LOCAL;

    if(fetch_int(args, "no_ack", &no_ack) != RMQC_OK)
        no_ack = DEFAULT_NO_ACK;

    if(fetch_int(args, "exclusive", &exclusive) != RMQC_OK)
        exclusive = DEFAULT_EXCLUSIVE;

    fetch_str(args, "consumer_tag", &tag_name);
    consumer_tag = (tag_name ? amqp_cstring_bytes(tag_name) : amqp_empty_bytes);

    fetch_str(args, "queue", &queue_name);
    queue = (queue_name ? amqp_cstring_bytes(queue_name) : amqp_empty_bytes);

    amqp_basic_consume(self->con, channel, queue, consumer_tag, no_local,
                       no_ack, exclusive, amqp_empty_table);
    reply = amqp_get_rpc_reply(self->con);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("failed to consume on channel %d", channel);

    return RMQC_OK;
}

extern SV
*rmqc_receive(rmqc_t *self)
{
    amqp_rpc_reply_t res;
    amqp_envelope_t envelope;
    HV *out = newHV();

    amqp_maybe_release_buffers(self->con);

    /* We set no timeout, so this will block. */
    res = amqp_consume_message(self->con, &envelope, NULL, 0);
    if(res.reply_type != AMQP_RESPONSE_NORMAL)
        croak("received unexpected response");

    hv_store(out, "channel", strlen("channel"), newSViv(envelope.channel), 0);
    hv_store(out, "exchange", strlen("exchange"),
             newSVpv(envelope.exchange.bytes, envelope.exchange.len), 0);
    hv_store(out, "consumer_tag", strlen("consumer_tag"),
             newSVpv(envelope.consumer_tag.bytes, envelope.consumer_tag.len), 0);
    hv_store(out, "routing_key", strlen("routing_key"),
             newSVpv(envelope.routing_key.bytes, envelope.routing_key.len), 0);
    hv_store(out, "body", strlen("body"),
             newSVpv(envelope.message.body.bytes, envelope.message.body.len), 0);
    
    amqp_destroy_envelope(&envelope);

    return newRV_noinc((SV *) out);
}

extern int
rmqc_close(rmqc_t *self)
{
    int i;
    amqp_rpc_reply_t reply;

    for(i = 0; i < self->num_channels; i++) {
        close_channel(self, self->channels[i]);
    }
    free(self->channels);
    self->channels = NULL;

    reply = amqp_connection_close(self->con, AMQP_REPLY_SUCCESS);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("failed to close connection");
    amqp_destroy_connection(self->con);
    self->con = NULL;

    return RMQC_OK;
}

extern int
rmqc_destroy(rmqc_t *self)
{
    if(self->con != NULL)
        rmqc_close(self);
    
    if(self != NULL)
        free(self);
    
    return RMQC_OK;
}

static int
close_channel(rmqc_t *self, int channel)
{
    amqp_rpc_reply_t reply;

    reply = amqp_channel_close(self->con, channel, AMQP_REPLY_SUCCESS);
    if(reply.reply_type != AMQP_RESPONSE_NORMAL)
        croak("failed to close channel %d", channel);

    return RMQC_OK;
}

static int
channel_exists(rmqc_t *self, int channel)
{
    int i;

    for(i = 0; i < self->num_channels; i++)
        if(channel == self->channels[i])
            return 1;

    return 0;
}

static void
store_channel(rmqc_t *self, int channel)
{
    if(!channel_exists(self, channel))
        return;

    if(self->num_channels == self->max_channels)
        croak("max configured channels of %d exceeded", self->max_channels);

    self->channels[++self->num_channels] = channel;
}

static int
fetch_int(HV *h, char *key, int *val)
{
    SV **v;

    if(!hv_exists(h, key, strlen(key))
       || !(v = hv_fetch(h, key, strlen(key), 0)))
    {
        return RMQC_ERR;
    }

    *val = SvIV(*v);
    return RMQC_OK;
}

static int
fetch_str(HV *h, char *key, char **val)
{
    SV **v;

    if(!hv_exists(h, key, strlen(key))
       || !(v = hv_fetch(h, key, strlen(key), 0)))
    {
        return RMQC_ERR;
    }

    *val = SvPV_nolen(*v);
    return RMQC_OK;
}

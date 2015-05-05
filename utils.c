#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define DEFAULT_PORT 5672
#define DEFAULT_TLS  0

extern int
rmqc_new(rmqc_t **self, HV *args)
{
    /* Validate the parameters. */
    if(!hv_exists(args, "host", strlen("host")))
        croak("the host is required");

    if(!hv_exists(args, "port", strlen("port")))
        hv_store(args, "port", strlen("port"), newSVuv(DEFAULT_PORT), 0);

    if(!hv_exists(args, "tls", strlen("tls")))
        hv_store(args, "tls", strlen("tls"), newSViv(0), 0);

    *self = calloc(1, sizeof(**self));
    if(*self == NULL)
        croak("could not initialize instance");

    (*self)->con = NULL;
    (*self)->options = args;

    return RMQC_OK;
}

extern int
rmqc_destroy(rmqc_t *self)
{
    if(self != NULL)
        free(self);
    
    return RMQC_OK;
}

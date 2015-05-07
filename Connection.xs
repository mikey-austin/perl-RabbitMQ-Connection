#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "utils.h"

MODULE = RabbitMQ::Connection		PACKAGE = RabbitMQ::Connection		
    
PROTOTYPES: ENABLE

SV *
_new(package, args_ref)
    char *package
    SV *args_ref
    INIT:
        rmqc_t *s;
        HV *args = (HV *) SvRV(args_ref);
    CODE:
        rmqc_new(&s, args);
        SV *addr = newSViv(PTR2IV(s));
        SV * const self = newRV_noinc(addr);
        RETVAL = sv_bless(self, gv_stashpv(package, 0));
    OUTPUT:
        RETVAL

void
connect(self)
    RabbitMQ::Connection self
    CODE:
        rmqc_connect(self);

char *
_declare_queue(self, args_ref)
    RabbitMQ::Connection self
    SV *args_ref
    INIT:
        HV *args = (HV *) SvRV(args_ref);
    CODE:
        RETVAL = rmqc_declare_queue(self, args);
    OUTPUT:
        RETVAL

void
_bind(self, args_ref)
    RabbitMQ::Connection self
    SV *args_ref
    INIT:
        HV *args = (HV *) SvRV(args_ref);
    CODE:
        rmqc_bind(self, args);

void
_consume(self, args_ref)
    RabbitMQ::Connection self
    SV *args_ref
    INIT:
        HV *args = (HV *) SvRV(args_ref);
    CODE:
        rmqc_consume(self, args);

void
receive(self)
    RabbitMQ::Connection self
    INIT:
        amqp_envelope_t envelope;
    CODE:
        rmqc_receive(self, &envelope);

void
close(self)
    RabbitMQ::Connection self
    CODE:
        rmqc_close(self);

void
DESTROY(self)
    RabbitMQ::Connection self
    CODE:
        rmqc_destroy(self);

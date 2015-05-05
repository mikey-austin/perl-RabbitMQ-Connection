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

void
DESTROY(self)
    RabbitMQ::Connection self
    CODE:
        rmqc_destroy(self);

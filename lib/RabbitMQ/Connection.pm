package RabbitMQ::Connection;

use strict;
use warnings;

our $VERSION = '0.6';

require XSLoader;
XSLoader::load('RabbitMQ::Connection', $VERSION);

sub new {
    my ($class, %args) = @_;
    $class->_new(\%args);
}

sub declare_queue {
    my ($self, %args) = @_;
    $self->_declare_queue(\%args);
}

sub declare_exchange {
    my ($self, %args) = @_;
    $self->_declare_exchange(\%args);
}

sub send {
    my ($self, %args) = @_;
    $self->_send(\%args);
}

sub send_ack {
    my ($self, %args) = @_;
    $self->_send_ack(\%args);
}

sub bind {
    my ($self, %args) = @_;
    $self->_bind(\%args);
}

sub consume {
    my ($self, %args) = @_;
    $self->_consume(\%args);
}

1;

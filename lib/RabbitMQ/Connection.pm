package RabbitMQ::Connection;

use strict;
use warnings;

our $VERSION = '0.1';

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

1;

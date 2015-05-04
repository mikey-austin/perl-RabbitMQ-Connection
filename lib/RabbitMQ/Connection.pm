package RabbitMQ::Connection;

use strict;
use warnings;

our $VERSION = '0.1';

require XSLoader;
XSLoader::load('RabbitMQ::Connection', $VERSION);

1;

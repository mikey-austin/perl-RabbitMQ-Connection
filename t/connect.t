use strict;
use warnings;
use Env qw(TEST_HOST TEST_PORT);

use Test::More tests => 2;
BEGIN {
    use_ok('RabbitMQ::Connection');
};

my $c = RabbitMQ::Connection->new(
    host => $ENV{TEST_HOST} || 'localhost',
    port => $ENV{TEST_PORT} || 5672,
    tls  => 0
    );

isa_ok($c, 'RabbitMQ::Connection');

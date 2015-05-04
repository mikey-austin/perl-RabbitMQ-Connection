use strict;
use warnings;
use Env qw(TEST_BASE);

use Test::More tests => 1;
BEGIN {
    use_ok('RabbitMQ::Connection');
};

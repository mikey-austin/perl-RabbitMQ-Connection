#use 5.018004;
use 5.010001; # EL6 version
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME         => 'RabbitMQ::Connection',
    VERSION_FROM => 'lib/RabbitMQ/Connection.pm',
    LIBS         => ['-L../rabbitmq-c/build/librabbitmq -lrabbitmq'],
    INC          => '-I. -I../rabbitmq-c/librabbitmq',
    OBJECT       => '$(O_FILES)',
    );

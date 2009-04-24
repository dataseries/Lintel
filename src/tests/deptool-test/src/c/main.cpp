#include <a.h>
#include <b.h>

int main( int argc, char **argv) {
    Actor clown1( "Emmett Kelly");
    Actor clown2( "Grock");
    Actor clown3( "Bill Irwin");

    BigTop bt[3] = { BigTop( &clown1), BigTop( &clown2), BigTop( &clown3)};

    if (argc != 1) usage();
    int clown = atoi(argv[0])-1;
    if (clown < 0 || clown > 2) usage();

    bt[clown].show();
}

void usage() {
    cout << "Usage: clown [1|2|3]" << endl;
    exit(1);
}

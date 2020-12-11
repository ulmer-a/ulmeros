#include <stdlib.h>

extern int main(int argc, char** argv);

void _start()
{
    int ret = main(0, 0);
    exit(ret);
}
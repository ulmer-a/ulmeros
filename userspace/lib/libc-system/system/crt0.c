#include <stdlib.h>

extern int main(int argc, char** argv);

char** _envp;

void _start(int argc, char** argv, char** envp)
{
  envp = envp;
  int status = main(argc, argv);
  exit(status);
}

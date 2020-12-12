#include <fcntl.h>
#include <stdlib.h>

char **environ;

extern void exit(int status);
extern int main (int argc, char** argv);

void _start(int argc, char** argv, char** envp)
{
  environ = envp;
  int status = main(argc, argv);
  exit(status);
}

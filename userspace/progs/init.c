#include <stdio.h>
#include <fcntl.h>

const char* ULMEROS_BANNER =
  "\n"
  " _    _ _                      ____   _____ \n"
  "| |  | | |                    / __ \\ / ____|\n"
  "| |  | | |_ __ ___   ___ _ __| |  | | (___  \n"
  "| |  | | | '_ ` _ \\ / _ \\ '__| |  | |\\___ \\ \n"
  "| |__| | | | | | | |  __/ |  | |__| |____) |\n"
  " \\____/|_|_| |_| |_|\\___|_|   \\____/|_____/ \n\n";

int main()
{
  /* open standard streams */
  open("/dev/stdin", O_RDONLY);
  open("/dev/stdout", O_WRONLY);
  open("/dev/stderr", O_WRONLY);

  /* print the Ulmer OS banner */
  puts(ULMEROS_BANNER);

  /* perform some more initialization (like
   * mounting file systems, etc. */
  return 0;
}

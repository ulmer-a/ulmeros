cmake_minimum_required(VERSION 3.15)

set(NL_CLFAGS -ffreestanding -nostdinc
    -fno-strict-aliasing -fno-builtin -fno-stack-protector)

add_subdirectory(arch)
add_subdirectory(newlib-core)



add_custom_target(libc)
add_dependencies(libc newlib-core newlib-arch)

target_compile_definitions(newlib-core PUBLIC
    -DHAVE_OPENDIR -D_NO_FNMATCH -D_NO_GLOB -D_NO_WORDEXP
    -DHAVE_MMAP=0 -D_NO_GETCWD -D_NO_GETLOGIN -D_NO_GETPASS
    -D_NO_GETPWENT -D_NO_GETUT -D_NO_TTYNAME -D_HAVE_LONG_DOUBLE
    -DMISSING_SYSCALL_NAMES -D_NO_POSIX_SPAWN -D_NO_REGEX
    -D__LARGE64_FILES -D_COMPILING_NEWLIB
)

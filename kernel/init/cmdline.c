#include <cmdline.h>
#include <mm/memory.h>
#include <util/list.h>
#include <util/string.h>
#include <debug.h>

typedef struct
{
  char name[32];
  char value[32];
} cmdline_param_t;

static list_t args;

const char *cmdline_get(const char *param)
{
  for (list_item_t* it = list_it_front(&args);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    cmdline_param_t* p = list_it_get(it);
    if (strcmp(param, p->name) == 0)
      return p->value;
  }
  return NULL;
}

void cmdline_parse(const char *cmdline)
{
  list_init(&args);

  // TODO: buffer overrun checks!

  int mode = 0;
  cmdline_param_t param;
  char *name_ptr = param.name,
      *val_ptr = param.value;

  while (true)
  {
    char c = *cmdline++;

    if (mode == 0)
    {
      if (c == '=')
      {
        *name_ptr++ = 0;
        mode = 1;
      }
      else
        *name_ptr++ = c;
    }
    else
    {
      if (c == ' ' || c == 0)
      {
        mode = 0;
        *val_ptr++ = 0;
        name_ptr = param.name;
        val_ptr = param.value;

        cmdline_param_t* p = kmalloc(sizeof(cmdline_param_t));
        strcpy(p->name, param.name);
        strcpy(p->value, param.value);
        list_add(&args, p);

        //debug(INIT, "parsed cmdline arg: %s, %s\n", p->name, p->value);

        if (c == 0)
          break;
      }
      else
        *val_ptr++ = c;
    }
  }
}

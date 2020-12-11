#include <amd64/boot.h>
#include <amd64/multiboot.h>
#include <vspace.h>

extern size_t total_pages_;
extern uint8_t* refcounter_;

static void set_entries(size_t start_page, size_t end_page, uint8_t value)
{
  debug(PAGEMGR, "  %zd -> %zd: %s\n", start_page, end_page,
        (value == 0) ? "free" : "reserved");
  for (size_t page = start_page; page <= end_page; page++)
    refcounter_[page] = value;
}

void page_init(boot_info_t* boot_info)
{
  struct mmape_struct* mmap = (void*)boot_info->mmap;
  struct mmape_struct *end = (struct mmape_struct*)
      ((char*)boot_info->mmap + boot_info->mmap_length);

  debug(PAGEMGR, "starting multiboot memory scan\n");
  refcounter_ = vspace_get_page_ptr(boot_info->heapStartPage +
                 boot_info->heapPageCount);

  size_t last_end_page = 0;
  for (struct mmape_struct *entry = mmap;
       entry < end;
       entry = (struct mmape_struct*)((char*)entry + entry->entry_size + 4))
  {
    size_t start_page = entry->base_addr / PAGE_SIZE;
    if (entry->type == 1 && entry->base_addr % PAGE_SIZE != 0)
      start_page++;

    size_t end_page   = start_page + (entry->size / PAGE_SIZE);
    if (entry->type != 1 && entry->size % PAGE_SIZE != 0)
      end_page ++;

    size_t diff = start_page - last_end_page;
    if (diff > 2048)
    {
      break;
    }
    else if (diff > 0)
    {
      set_entries(last_end_page + 1, start_page - 1, 1);
    }

    set_entries(start_page, end_page, (entry->type == 1) ? 0 : 1);
    last_end_page = end_page;
  }

  total_pages_ = last_end_page;
  size_t refcounter_pages = last_end_page / PAGE_SIZE;
  if (last_end_page % PAGE_SIZE != 0)
    refcounter_pages += 1;

  set_entries(0, 0, 1);
  set_entries(boot_info->kernelStartPage, boot_info->kernelStartPage
              + boot_info->kernelPageCount, 1);
  set_entries(boot_info->heapStartPage, boot_info->heapStartPage
              + boot_info->heapPageCount + refcounter_pages, 1);
  debug(PAGEMGR, "memscan completed\n");
}

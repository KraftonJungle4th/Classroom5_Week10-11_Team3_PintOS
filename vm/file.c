/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
/* file-backed 페이지에 대한 함수 포인터 테이블 */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Do the mmap */
void *do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset) {
	struct file *mfile = file_reopen(file);

    void * ori_addr = addr;
    size_t read_bytes = length > file_length(file) ? file_length(file) : length;
    size_t zero_bytes = PGSIZE - read_bytes % PGSIZE;

	while (read_bytes > 0 || zero_bytes > 0) {
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

        struct supplemental_page_table *spt = (struct supplemental_page_table*)malloc
										(sizeof(struct supplemental_page_table));
        spt->file = mfile;
        spt->offset = offset;
        spt->read_bytes = page_read_bytes;

		if (!vm_alloc_page_with_initializer (VM_FILE, addr, writable, lazy_load_segment, spt)) {
			return NULL;
        }
		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		addr       += PGSIZE;
		offset     += page_read_bytes;
	}
	return ori_addr;
}

/* Do the munmap */
void do_munmap (void *addr) {
	while (true) {
        struct page* page = spt_find_page(&thread_current()->spt, addr);
        
        if (page == NULL)
            break;

        struct supplemental_page_table * aux = (struct supplemental_page_table *) page->uninit.aux;
        
        // dirty(사용되었던) bit 체크
        if(pml4_is_dirty(thread_current()->pml4, page->va)) {
            file_write_at(aux->file, addr, aux->read_bytes, aux->offset);
            pml4_set_dirty (thread_current()->pml4, page->va, 0);
        }

        pml4_clear_page(thread_current()->pml4, page->va);
        addr += PGSIZE;
    }
}
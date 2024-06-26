#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;

struct anon_page {
    // int slot_no; // swap out될 때 이 페이지가 저장된 slot의 번호
    int sec_no;
    struct thread *thread;
};

void vm_anon_init (void);
bool anon_initializer (struct page *page, enum vm_type type, void *kva);

#endif

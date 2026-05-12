#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#define META_SIZE sizeof(struct program_block)

struct program_block {
    size_t size;
    struct program_block *next;
    int free; // 1 if availabe 0 if not
};

void *global_memory_pointer = NULL;

void* isaac_malloc(int size){
    //first lets see if theres a valid space
    struct program_block *current = global_memory_pointer;
    while(current){ //pointer is not null
        if(current->free == 1 && current->size <= size){
            current->free = 0; //now its occupied
            return current + 1;
        }
        current = current->next;
    }
    //create our block of memory
    struct program_block *block;

    //ask OS/Kernel for more memory
    //normally look through linked list but not for now
    block = sbrk(size + META_SIZE);
    printf("Asked for more memory");

    block->size = size;
    block->next = NULL;
    block->free = 0;


    if(!global_memory_pointer){
        global_memory_pointer = block;
    }else{
        //find the latest block that has no pointer to next
        struct program_block *current = global_memory_pointer;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = block;
    }

    //now return the memory address of the other bit of the block
    return(block+1);
}

void isaac_free(void *ptr){
    //creates block a program block defined as ptr - 1 so we can access its headers.
    struct program_block *block = (struct program_block*)ptr-1;

    block->free = 1;
}

void visualize_heap() {
    struct program_block *curr = global_memory_pointer;
    printf("\n--- HEAP SNAPSHOT ---\n");
    while (curr) {
        printf("[Header at %p] | Size: %zu | Status: %s\n", 
                (void*)curr, curr->size, curr->free ? "FREE" : "IN-USE");
        printf("  -> User Data starts at %p\n", (void*)(curr + 1));
        curr = curr->next; // This requires you to link blocks in malloc
        if(!curr) break; 
    }
    printf("---------------------\n");
}

int main() {

    visualize_heap();

    int *a = (int*)isaac_malloc(sizeof(int));
    *a = 100;

    int *b = (int*)isaac_malloc(sizeof(int));
    *b = 200;

    int *c = (int*)isaac_malloc(sizeof(int));
    *c = 300;

    visualize_heap();

    isaac_free(b); 

    visualize_heap();

    int *d = (int*)isaac_malloc(sizeof(int));
    *d = 200;
    
    visualize_heap();

    isaac_free(d); 

    visualize_heap();

    return 0;
}

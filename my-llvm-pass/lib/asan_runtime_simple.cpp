#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <map>

extern "C"{

    static std::map<void*, size_t> alloc_map;
    static std::map<void*, void*> ptr_relocations;

    int isAddressValid(void* addr, size_t size){
        fprintf(stderr, "[Runtime] checking isAddressValid(%p, %zu)\n", addr, size);
        
        if(addr == nullptr){
            return 0;
        }
        
        uintptr_t a = (uintptr_t)addr;
        size_t s = size == 0 ? 1 : size;

        for (auto &kv : alloc_map) {
            uintptr_t start = (uintptr_t)kv.first;
            size_t   len   = kv.second;

            if (a >= start && (a + s) <= (start + len)) {
                fprintf(stderr, "=> VALID\n");
                return 1;
            }
        }

        fprintf(stderr, "=> INVALID\n");
        return 0;
    }

    void* my_handle_illegal_access(void* base_ptr, void* curr_ptr, size_t size){
        fprintf(stderr, "\n========================================\n");
        fprintf(stderr, "[ASan] OUT-OF-BOUNDS ACCESS!\n");
        fprintf(stderr, "[ASan] Base: %p, Current: %p, Size: %zu\n", base_ptr, curr_ptr, size);
        
        if(base_ptr == nullptr || curr_ptr == nullptr) {
            fprintf(stderr, "[ASan] ERROR: NULL pointer\n");
            return nullptr;
        }

        uintptr_t base = (uintptr_t)base_ptr;
        uintptr_t illegal = (uintptr_t)curr_ptr;

        auto it = alloc_map.find(base_ptr);
        if(it == alloc_map.end()){
            fprintf(stderr, "[ASan] ERROR: Base %p not in map\n", base_ptr);
            fprintf(stderr, "[ASan] Known allocations:\n");
            for (auto &kv : alloc_map) {
                fprintf(stderr, "  %p -> %zu bytes\n", kv.first, kv.second);
            }
            return nullptr;
        }

        size_t oldSize = it->second;
        size_t offset = illegal - base;
        size_t needed = offset + size;

        fprintf(stderr, "[ASan] Old size: %zu, Offset: %zu, Needed: %zu\n", oldSize, offset, needed);

        alloc_map.erase(it);
        
        void* newBase = realloc(base_ptr, needed);

        if(!newBase){
            fprintf(stderr, "[ASan] FATAL: realloc failed!\n");
            alloc_map[base_ptr] = oldSize;
            return nullptr;
        }

        alloc_map[newBase] = needed;
        ptr_relocations[base_ptr] = newBase;
        
        fprintf(stderr, "[ASan] SUCCESS! %p -> %p, size %zu -> %zu\n", base_ptr, newBase, oldSize, needed);
        
        uintptr_t newIllegal = (uintptr_t)newBase + offset;
        fprintf(stderr, "[ASan] New access: %p\n", (void*)newIllegal);
        fprintf(stderr, "========================================\n\n");

        return (void*)newIllegal;
    }


    void update_pointer_after_realloc(void** ptr_var, void* new_access_ptr) {
        fprintf(stderr, "[Runtime] update_pointer_after_realloc(%p, %p)\n", ptr_var, new_access_ptr);
    }
    
    // Wrapper for malloc to track allocations
    void* tracked_malloc(size_t size) {
        void* ptr = malloc(size);
        if (ptr) {
            alloc_map[ptr] = size;
            fprintf(stderr, "[Runtime] tracked_malloc(%zu) = %p\n", size, ptr);
        }
        return ptr;
    }

    void* get_new_base(void* base_ptr){
        auto it = ptr_relocations.find(base_ptr);
        if (it != ptr_relocations.end()) {
            void* new_base = it->second;
            fprintf(stderr, "[Runtime] get_new_base(%p) = %p\n", base_ptr, new_base);
            return new_base;
        } else {
            fprintf(stderr, "[Runtime] get_new_base(%p) = not relocated\n", base_ptr);
            
        }
        return base_ptr;
    }

}
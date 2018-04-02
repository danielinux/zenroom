#include <errno.h>
#include <stdlib.h>
#include <jutils.h>

#include <zenroom.h>
#include <umm_malloc.h>

void *zen_memalign(size_t size, size_t align) {
	void *mem = NULL;
# if defined(_WIN32)
	mem = __mingw_aligned_malloc(size, align);
	if(!mem) {
		error("error in memory allocation.");
		return NULL; }
# else
	int res;
	res = posix_memalign(&mem, align, size);
	if(res == ENOMEM) {
		error("insufficient memory to allocate %u bytes.", size);
		return NULL; }
	if(res == EINVAL) {
		error("invalid memory alignment at 16 bytes.");
		return NULL; }
# endif
	return(mem);
}

typedef struct {
	void* (*malloc)(size_t size);
	void* (*realloc)(void *ptr, size_t size);
	void  (*free)(void *ptr);
} zen_mem_t;

static zen_mem_t zen_mem_f;

// Global HEAP pointer in the STACK
char *zen_heap;
size_t zen_heap_size;
void umm_memory_init(size_t size) {
	zen_heap = zen_memalign(size, 8);
	zen_heap_size = size;
	zen_mem_f.malloc = umm_malloc;
	zen_mem_f.realloc = umm_realloc;
	zen_mem_f.free = umm_free;
	umm_init(zen_heap, size);
	// pointers saved in umm_malloc.c (stack)
}

void libc_memory_init() {
	zen_mem_f.malloc = malloc;
	zen_mem_f.realloc = realloc;
	zen_mem_f.free = free;
	zen_heap = NULL;
}
void *zen_memory_alloc(size_t size) { return (*zen_mem_f.malloc)(size); }
void *zen_memory_realloc(void *ptr, size_t size) { return (*zen_mem_f.realloc)(ptr, size); }
void  zen_memory_free(void *ptr) { (*zen_mem_f.free)(ptr); }



/**
 * Implementation of the memory allocator for the Lua state.
 *
 * See: http://www.lua.org/manual/5.3/manual.html#lua_Alloc
 *
 * @param ud User Data Pointer
 * @param ptr Pointer to the memory block being allocated/reallocated/freed.
 * @param osize The original size of the memory block.
 * @param nsize The new size of the memory block.
 *
 * @return void* A pointer to the memory block.
 */
void *umm_memory_manager(void *ud, void *ptr, size_t osize, size_t nsize) {
	if(ptr == NULL) {
		// When ptr is NULL, osize encodes the kind of object that Lua
		// is allocating. osize is any of LUA_TSTRING, LUA_TTABLE,
		// LUA_TFUNCTION, LUA_TUSERDATA, or LUA_TTHREAD when (and only
		// when) Lua is creating a new object of that type. When osize
		// is some other value, Lua is allocating memory for something
		// else.
		if(nsize!=0)
			return umm_malloc(nsize);
		return NULL;


	} else {
		// When ptr is not NULL, osize is the size of the block
		// pointed by ptr, that is, the size given when it was
		// allocated or reallocated.
		if(nsize==0) {
			// When nsize is zero, the allocator must behave like free
			// and return NULL.
			umm_free(ptr);
			return NULL; }

		// When nsize is not zero, the allocator must behave like
		// realloc. The allocator returns NULL if and only if it
		// cannot fulfill the request. Lua assumes that the allocator
		// never fails when osize >= nsize.
		if(osize >= nsize) { // shrink
			return umm_realloc(ptr, nsize);
		} else { // extend
			return umm_realloc(ptr, nsize);
		}
	}
}

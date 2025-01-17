#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
		initialize_dyn_block_system();
		cprintf("DYNAMIC BLOCK SYSTEM IS INITIALIZED\n");
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//=================================
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//panic("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	//[2] Dynamically allocate the array of MemBlockNodes at VA USER_DYN_BLKS_ARRAY
	//	  (remember to set MAX_MEM_BLOCK_CNT with the chosen size of the array)
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	//[4] Insert a new MemBlock with the heap size into the FreeMemBlocksList
	LIST_INIT(&FreeMemBlocksList);
		LIST_INIT(&AllocMemBlocksList);
		MAX_MEM_BLOCK_CNT = (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;
		uint32 r = ROUNDUP(MAX_MEM_BLOCK_CNT * sizeof(struct MemBlock), PAGE_SIZE);
		MemBlockNodes = (struct MemBlock *)USER_DYN_BLKS_ARRAY;
		sys_allocate_chunk(USER_DYN_BLKS_ARRAY,r,PERM_WRITEABLE|PERM_USER);
		initialize_MemBlocksList(MAX_MEM_BLOCK_CNT); // initially make the  memblocknode list is available
		struct MemBlock *tmp = LIST_FIRST(&AvailableMemBlocksList); //here we take from the avilable
		LIST_REMOVE(&AvailableMemBlocksList, tmp);
		tmp->size = (USER_HEAP_MAX - USER_HEAP_START);
		tmp->sva = USER_HEAP_START;
		LIST_INSERT_HEAD(&FreeMemBlocksList, tmp);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//==============================================================

	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] malloc
	// your code is here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FF strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	// 	3) Return pointer containing the virtual address of allocated space,
	//
	//Use sys_isUHeapPlacementStrategyFIRSTFIT()... to check the current strategy
	size=ROUNDUP(size, PAGE_SIZE);
		if(sys_isUHeapPlacementStrategyFIRSTFIT())
				{
				     struct MemBlock*blo=alloc_block_FF(size);
					if(blo!=NULL)
					{
					insert_sorted_allocList(blo); //insert in the allocated from free
					return (void*)blo->sva;
				}
					else{
					return NULL;
				     }
			}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	FROM main memory AND free pages from page file then switch back to the user again.
//
//	We can use sys_free_user_mem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls free_user_mem() in
//		"kern/mem/chunk_operations.c", then switch back to the user mode here
//	the free_user_mem function is empty, make sure to implement it.
void free(void* virtual_address)
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] free
	// your code is here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you should get the size of the given allocation using its address
	//you need to call sys_free_user_mem()
	//refer to the project presentation and documentation for details
	uint32 va = (uint32) virtual_address;
		struct MemBlock *blockExist = find_block(&(AllocMemBlocksList), va);
		if (blockExist != NULL)
		{
			uint32 size=blockExist->size;
			sys_free_user_mem(va,size);
			LIST_REMOVE(&AllocMemBlocksList, blockExist);
			insert_sorted_with_merge_freeList(blockExist);
		}
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	uint32 Nsize = ROUNDUP(size, PAGE_SIZE);
		if (sys_isUHeapPlacementStrategyFIRSTFIT())
		{
			struct MemBlock*blockF = alloc_block_FF(Nsize);
			if (blockF != NULL)
			{
				//uint32 vablock = ROUNDDOWN(blockF->sva, PAGE_SIZE);
				int SharedFound = sys_createSharedObject(sharedVarName, Nsize,isWritable, (void*) blockF->sva);
				if(SharedFound==E_SHARED_MEM_EXISTS||SharedFound==E_NO_SHARE)
				{
					return NULL;
				}
				else
				{
					return (void*) blockF->sva;
				}
			}
			else
			{
				return NULL;
			}
		}
		return NULL;
		// Steps:
		//	1) Implement FIRST FIT strategy to search the heap for suitable space
		//		to the required allocation size (space should be on 4 KB BOUNDARY)
		//	2) if no suitable space found, return NULL
		//	 Else,
		//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
		//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
		//	4) If the Kernel successfully creates the shared variable, return its virtual address
		//	   Else, return NULL

		//This function should find the space of the required range
		// ******** ON 4KB BOUNDARY ******************* //

		//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");

	uint32 sizeofthesharedvariable = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
		if (sizeofthesharedvariable == E_SHARED_MEM_NOT_EXISTS){
			return NULL;
		}
		if (sizeofthesharedvariable != E_SHARED_MEM_NOT_EXISTS){
			if (sys_isUHeapPlacementStrategyFIRSTFIT()){
				sizeofthesharedvariable = ROUNDUP(sizeofthesharedvariable,PAGE_SIZE);
				struct MemBlock*blo = alloc_block_FF(sizeofthesharedvariable);
				if (blo == NULL){
					return NULL;
				}
				else{
					int SID = sys_getSharedObject(ownerEnvID, sharedVarName,(void*) blo->sva);
					if (SID !=E_SHARED_MEM_NOT_EXISTS){
						return (void*) blo->sva;
					}else{
						return NULL;
					}
				}
			}
		}
		return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// [USER HEAP - USER SIDE] realloc
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT MS3 - BONUS] [SHARING - USER SIDE] sfree()

	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}




//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");
}

#pragma once

#include <iostream>
#include <cstddef>
#include <climits>
#include <cstdlib>
#include <new>
#include <vector>
#include <string>
#include <sys/stat.h>

#include "libpmem.h"
#include "libpmemobj.h"
#include <garbage_list.h>

#include "utils.h"
 
// In this class, I will write a custom template allocator
// Specifically, it allocates persistent memory using PMDK interface
// Moreover, need to use static member to make all allocatoion in a single memory pool

static const char* layout_name = "template_pool";
static const uint64_t pool_addr = 0x5f0000000000;
static const char* pool_name = "/mnt/pmem0/zzy/template.data";
static const uint64_t pool_size = 40UL * 1024*1024*1024;

namespace my_alloc{

typedef void (*DestroyCallback)(void* callback_context, void* object);

	template <class T1, class T2>
	inline void _construct(T1* p, const T2& value){new (p) T1(value);}

	template <class T>
	inline void _destroy(T* ptr){ ptr->~T();}

    //Implement a base class that has the memory pool
class BasePMPool{
public:
    static PMEMobjpool *pm_pool_[nali::numa_node_num];
    static int allocator_num;   
    static BasePMPool* instance_;

    EpochManager epoch_manager_[nali::numa_node_num];
    GarbageList garbage_list_[nali::numa_node_num];

    static bool Initialize(const char* pool_name, size_t pool_size){
        //if(pm_pool_[nali::get_numa_id(nali::thread_id)] == nullptr){
        bool recover = false;
        const std::string pool_path_ = "/mnt/pmem";
        for (int i = 0; i < nali::numa_node_num; i++) {
            std::string path_ = pool_path_ + std::to_string(i) + "/zzy/nali_data";
            if ((pm_pool_[i] = pmemobj_create(path_.c_str(), POBJ_LAYOUT_NAME(btree), pool_size, 0666)) == NULL) {
                perror("failed to create pool.\n");
                exit(-1);
            }
        }

        instance_ = new BasePMPool();
        for (int i = 0; i < nali::numa_node_num; i++) {
            instance_->epoch_manager_[i].Initialize();
            instance_->garbage_list_[i].Initialize(&instance_->epoch_manager_[i], instance_->pm_pool_[nali::get_numa_id(nali::thread_id)], 1024 * 8);
        }

        IncreaseAllocatorNum();
        return recover;
    }

    static void IncreaseAllocatorNum(){
        allocator_num++;
    }

    static void DecreaseAllocatorNum(){
        allocator_num--;
    }

    static void ClosePool(){
        if(pm_pool_[nali::get_numa_id(nali::thread_id)] != nullptr){
            pmemobj_close(pm_pool_[nali::get_numa_id(nali::thread_id)]);
        }
    }

    static void* GetRoot(size_t size) {
        return pmemobj_direct(pmemobj_root(pm_pool_[nali::get_numa_id(nali::thread_id)], size));
    }

    static void AlignAllocate(void** ptr, size_t size){
        PMEMoid tmp_ptr;
        auto ret = pmemobj_alloc(pm_pool_[nali::get_numa_id(nali::thread_id)], &tmp_ptr, size + 64, TOID_TYPE_NUM(char), NULL, NULL);
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << size << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 1");
        }
        uint64_t ptr_value = (uint64_t)(pmemobj_direct(tmp_ptr)) + 48;
        *ptr = (void*)(ptr_value);
    }

    static void AlignZAllocate(void** ptr, size_t size){
        PMEMoid tmp_ptr;
        auto ret = pmemobj_zalloc(pm_pool_[nali::get_numa_id(nali::thread_id)], &tmp_ptr, size + 64, TOID_TYPE_NUM(char));
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << size << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 1");
        }
        uint64_t ptr_value = (uint64_t)(pmemobj_direct(tmp_ptr)) + 48;
        *ptr = (void*)(ptr_value);
    }

    static void Free(void* p){ 
        auto ptr = pmemobj_oid(p);
        pmemobj_free(&ptr);
    }

    //Need to address this
    static void Allocate(void** ptr, size_t size){
        PMEMoid tmp_ptr;
        auto ret = pmemobj_alloc(pm_pool_[nali::get_numa_id(nali::thread_id)], &tmp_ptr, size, TOID_TYPE_NUM(char), NULL, NULL);
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << size << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 1");
        }
        *ptr = pmemobj_direct(tmp_ptr);
    }

    static void ZAllocate(void** ptr, size_t size){
        PMEMoid tmp_ptr;
        auto ret = pmemobj_zalloc(pm_pool_[nali::get_numa_id(nali::thread_id)], &tmp_ptr, size, TOID_TYPE_NUM(char));
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << size << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 1");
        }
        *ptr = pmemobj_direct(tmp_ptr);
    }

    static void Allocate(PMEMoid *ptr, size_t size){
        auto ret = pmemobj_alloc(pm_pool_[nali::get_numa_id(nali::thread_id)], ptr, size, TOID_TYPE_NUM(char), NULL, NULL);
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << size << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 1");
        }
    }

    static void Allocate(PMEMoid* pm_ptr, uint32_t alignment, size_t size,
                       int (*alloc_constr)(PMEMobjpool* pool, void* ptr,
                                           void* arg),
                       void* arg) {
    auto ret = pmemobj_alloc(pm_pool_[nali::get_numa_id(nali::thread_id)], pm_ptr, size,
                             TOID_TYPE_NUM(char), alloc_constr, arg);
        if (ret) {
        LOG_FATAL("Allocate Initialize: Allocation Error in PMEMoid");
        }
    }

    static void ZAllocate(PMEMoid *ptr, size_t size){
        auto ret = pmemobj_zalloc(pm_pool_[nali::get_numa_id(nali::thread_id)], ptr, size, TOID_TYPE_NUM(char));
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << size << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 1");
        }
    }

    static void AlignFree(void* p){ 
        uint64_t ptr_value = (uint64_t)(p) - 48;
        void *new_p = reinterpret_cast<void*>(ptr_value);
        auto ptr = pmemobj_oid(new_p);
        pmemobj_free(&ptr);
    }

    static void Persist(void* p, size_t size){
        pmemobj_persist(pm_pool_[nali::get_numa_id(nali::thread_id)], p, size);
    }

    static void DefaultPMCallback(void* callback_context, void* ptr) {
        auto oid_ptr = pmemobj_oid(ptr);
        TOID(char) ptr_cpy;
        TOID_ASSIGN(ptr_cpy, oid_ptr);
        POBJ_FREE(&ptr_cpy);
    }

    static void DefaultDRAMCallback(void* callback_context, void* ptr) {
        free(ptr);
    }

    static void SafeFree(void* ptr, DestroyCallback callback = DefaultPMCallback,
                    void* context = nullptr) {
        instance_->garbage_list_[nali::get_numa_id(nali::thread_id)].Push(ptr, callback, context);
    }

    static void SafeFree(GarbageList::Item* item, void* ptr,
                    DestroyCallback callback = DefaultPMCallback,
                    void* context = nullptr) {
        item->SetValue(ptr, instance_->epoch_manager_[nali::get_numa_id(nali::thread_id)].GetCurrentEpoch(), callback,
                    context);
    }

    static EpochGuard AquireEpochGuard() {
        return EpochGuard{&instance_->epoch_manager_[nali::get_numa_id(nali::thread_id)]};
    }

    static void Protect() { instance_->epoch_manager_[nali::get_numa_id(nali::thread_id)].Protect(); }

    static void Unprotect() { instance_->epoch_manager_[nali::get_numa_id(nali::thread_id)].Unprotect(); }

    static GarbageList::Item* ReserveItem() {
        return instance_->garbage_list_[nali::get_numa_id(nali::thread_id)].ReserveItem();
    }

    static void ResetItem(GarbageList::Item* mem) {
        instance_->garbage_list_[nali::get_numa_id(nali::thread_id)].ResetItem(mem);
    }

    static void EpochRecovery() {
        instance_->garbage_list_[nali::get_numa_id(nali::thread_id)].Recovery(&instance_->epoch_manager_[nali::get_numa_id(nali::thread_id)],
                                        instance_->pm_pool_[nali::get_numa_id(nali::thread_id)]);
    }
};

	
template <class T>
class allocator : BasePMPool{
public: 
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	//constructor fucn
    allocator(){
        std::cout << "Intial allocator: " << allocator_num << std::endl;
        ADD(&allocator_num, 1);
        if(allocator_num == 1){
            BasePMPool::Initialize("", pool_size);
        }
    }

    allocator(const allocator<T>& c){
        ADD(&allocator_num, 1);
        if(allocator_num == 1){
            BasePMPool::Initialize("", pool_size);
        }
    }

    ~allocator(){
        if(allocator_num == 0){
            //delete PM pool
            ClosePool();
        }
    }

	template <class U>
	allocator(const allocator<U>& c){
        if(allocator_num == 1){
            BasePMPool::Initialize(pool_name, pool_size);
        }
    }

	//rebind allocator of type U
	template <class U>
	struct rebind {typedef allocator<U> other;};

	//pointer allocate(size_type n, const void* hint=0){
    pointer allocate(size_type n){
        //FIXME: non-safe memory allocation
        PMEMoid tmp_ptr;
        auto ret = pmemobj_alloc(pm_pool_[nali::get_numa_id(nali::thread_id)], &tmp_ptr, (size_t)(n * sizeof(T)), TOID_TYPE_NUM(char), NULL, NULL);
        if (ret) {
          std::cout << "Fail logging: " << ret << "; Size = " << n *sizeof(T) << std::endl;
          LOG_FATAL("Allocate: Allocation Error in PMEMoid 2");
        }
        pointer tmp = (pointer)pmemobj_direct(tmp_ptr);
        return tmp;
	}

	void deallocate(pointer p, size_type n){ 
        auto ptr = pmemobj_oid(p);
        pmemobj_free(&ptr);
    }
	void construct(pointer p, const T& value){ _construct(p, value);}
	void destroy(pointer p){_destroy(p);}

	pointer address(reference x){return (pointer)&x;}
	const_pointer const_address(const_reference x){ return (const_pointer)&x;}

	size_type max_size() const {return size_type(UINT_MAX / sizeof(T));}
};

PMEMobjpool* BasePMPool::pm_pool_[nali::numa_node_num];
int BasePMPool::allocator_num = 0;
BasePMPool* BasePMPool::instance_ = nullptr;
}

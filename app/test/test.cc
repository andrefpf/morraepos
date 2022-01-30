#include <utility/ostream.h>
#include <process.h>
#include <memory.h>
#include <architecture/armv8/armv8_mmu.h>
#include <architecture/armv8/armv8_cpu.h>

using namespace EPOS;
using namespace EPOS::S;

OStream cout;

extern "C" char _end;

typedef S::U::Heap Heap;

void * APP_HEAP = reinterpret_cast<void *> (Traits<Machine>::APP_HEAP);
static const unsigned int HEAP_SIZE = Traits<Machine>::HEAP_SIZE;
static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;

// Empty class used to allocate something
class TestClass {
};

/*
 * @Brief Test 1 can be used without modifications on any system class, but relies on some
 * assumptions that can be changed futurelly.
 *
 * @Param char * ptr: A poiter to the first object allocated in the heap.
 */
void test_1heap(char * ptr) {
    cout << "Starting Test 1" << endl;

    int offset = 2 * sizeof(void *) + sizeof(int);

    void * data_end = reinterpret_cast<void *> (&_end);     // Get the pointer to the end of the data segment
    void * heap_pointer = reinterpret_cast<void *> (ptr - HEAP_SIZE + offset);   // Subtract HEAP_SIZE because of top-down allocator aproach, and compensate 8 bytes subtracted during allocation
    void * heap_with_stack = reinterpret_cast<void *> (ptr - HEAP_SIZE - STACK_SIZE + offset);   // Same as before but subtract stack allocated by SETUP

    assert(data_end != heap_pointer);
    assert(data_end != heap_with_stack);
    assert(heap_pointer == APP_HEAP);

    cout << "heap pointer address" << heap_pointer << endl;
    cout << "heap pointer with stack address " << heap_with_stack << endl;
    cout << "end of data segment " << data_end << endl;
    cout << "End of Test 1" << endl;
    cout << endl;
}

/*
 * @Brief Test 2 may be a little more robust, but relies on some very specific implementation details
 * and needs to add the start_address variable on the Heap class.
 *
 * @Param char * ptr: A poiter to some object allocated in the heap.
 */
void test_2heap(char * ptr) {
    cout << "Starting Test 2" << endl;

    size_t * addr = reinterpret_cast<size_t *>(ptr - sizeof(int) - sizeof(void*));
    Heap * heap = reinterpret_cast<Heap *> (*addr); // Figure out heap like in Heap::typed_free

    void * data_end = reinterpret_cast<void *> (&_end);
    void * heap_pointer = reinterpret_cast<void *> (heap->start_address);  // Using a new variable created at the class Heap
    void * heap_with_stack = reinterpret_cast<void *> (reinterpret_cast<char *>(heap->start_address) - STACK_SIZE);

    assert(data_end != heap_pointer);
    assert(data_end != heap_with_stack);
    assert(heap_pointer == APP_HEAP);

    cout << "heap pointer address " << heap_pointer << endl;
    cout << "heap pointer with stack address " << heap_with_stack << endl;
    cout << "end of data segment " << data_end << endl;
    cout << "End of Test 2" << endl;
    cout << endl;
}

void compare(CPU::Log_Addr a, CPU::Log_Addr b) {
    cout << "\t Expected: \t {addr=" << hex << a << ", f=" << b << endl;
    cout << "\t Actual Values: \t " << MMU::Translation(a) << endl; 
    cout << endl;
}

void test_1mmu() 
{
    cout << "Init MMU test 1" << endl;

    System_Info * si = System::info();

    cout << "App Code: " << "\n" << endl;
    compare(si->lm.app_code, si->pmm.app_code);

    cout << "App Data: " <<  "\n" << endl;
    compare(si->lm.app_data, si->pmm.app_data);

    cout << "Sys Code: " <<  "\n" << endl;
    compare(si->lm.sys_code, si->pmm.sys_code);

    cout << "Sys Data: " << "\n" << endl;
    compare(si->lm.sys_data, si->pmm.sys_data);

    cout << "Sys Stack: " << "\n" << endl;
    compare(si->lm.sys_stack, si->pmm.sys_stack);

    cout << "Sys Info: " << endl;
    compare(Memory_Map::SYS_INFO, si->pmm.sys_info);

    cout << "Sys Page Table: " << "\n" << endl;
    compare(Memory_Map::SYS_PT, si->pmm.sys_pt);
    
    cout << "Sys Page Directory: \n" << endl;
    compare(Memory_Map::SYS_PD, si->pmm.sys_pd);
}

void test_2mmu()
{	
	cout << "Init MMU test 2\n" << endl;
	
	cout << "MMU::current=(" << MMU::current() << ")\n"<< endl;
	cout << "CPU::ttbr0_el1()=(" << hex << CPU::ttbr0_el1() << ")\n" << endl;
	compare(MMU::current(), CPU::ttbr0_el1());
}
void test_3mmu()
{
	cout << "Init MMU test 3\n" << endl;
	
	CPU::Phy_Addr * var_addr = 0x00;
	
	cout << "Allocating sizeof(int) * 1000\n" << endl;
	var_addr = ARMv8_MMU::alloc(sizeof(int) * 1000);
	cout << "ARMv8_MMU::alloc sucefully!\n" << endl;
	ARMv8_MMU::free(var_addr);
	cout << "Memory sucefully free!\n" << endl;
}
int main()
{

    // alloc some data on application heap
    TestClass * ptr = new TestClass();
    
    // Test using two different methods
    test_1heap((char *) ptr);
    test_2heap((char *) ptr);
    
    test_1mmu();
    test_2mmu();
    test_3mmu();
    cout << "That's it! 毛泽东万岁 !\n" << endl;
    return 0;
}

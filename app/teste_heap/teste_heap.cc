#include <utility/ostream.h>

extern "C" char _end;

using namespace EPOS;

OStream cout;

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
void test_1(char * ptr) {
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
void test_2(char * ptr) {
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


int main()
{
    // alloc some data on application heap
    TestClass * ptr = new TestClass();

    // Test using two different methods
    test_1((char *) ptr);
    test_2((char *) ptr);

    return 0;
}

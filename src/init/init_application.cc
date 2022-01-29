// EPOS Application Initializer

#include <architecture.h>
#include <utility/heap.h>
#include <machine.h>
#include <system.h>

extern "C" char _end; // defined by GCC

__BEGIN_SYS

class Init_Application
{
private:
    static const unsigned int HEAP_SIZE = Traits<Application>::HEAP_SIZE;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

public:
    Init_Application() {
        db<Init>(TRC) << "Init_Application()" << endl;

        // Initialize Application's heap
        db<Init>(INF) << "Initializing application's heap: " << endl;
        if(Traits<System>::multiheap) { // heap in data segment arranged by SETUP
            // Dinamically alocated pointers
            Segment * tmp_segm = reinterpret_cast<Segment *>(&System::_preheap[0]);
            Heap * tmp_heap = reinterpret_cast<Heap *>(&System::_preheap[sizeof(Segment)]);

            // Creates a new application segment with the appropriate flags
            Application::_heap_segment = new (tmp_segm) Segment(HEAP_SIZE, Segment::Flags::APP);

            // Attaches the new segment with a address space
            Address_Space::Log_Addr address = Address_Space(MMU::current()).attach(Application::_heap_segment, Memory_Map::APP_HEAP);

            // Creates a heap on the new segment
            Application::_heap = new(tmp_heap) Heap(address, Application::_heap_segment->size());
        } else
            for(unsigned int frames = MMU::allocable(); frames; frames = MMU::allocable())
                System::_heap->free(MMU::alloc(frames), frames * sizeof(MMU::Page));
        db<Init>(INF) << "done!" << endl;
    }
};

// Global object "init_application"  must be linked to the application (not to the system) and there constructed at first.
Init_Application init_application;

__END_SYS

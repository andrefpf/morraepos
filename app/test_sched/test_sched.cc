#include <utility/ostream.h>
#include <real-time.h>

using namespace EPOS;

OStream cout;

int func1() {
	do {
        cout << "Running func1" << endl;
  } while (Periodic_Thread::wait_next());
  return 0;
}

int func2() {
	do {
        cout << "Running func2" << endl;
  } while (Periodic_Thread::wait_next());
  return 0;
}

int func3() {
	do {
        cout << "Running func3" << endl;
  } while (Periodic_Thread::wait_next());
  return 0;
}


int main() {
  // "The expected order is: func2 -> func1 -> func3"

  Periodic_Thread * a = new Periodic_Thread(100000,  50000,  &func1);
  Periodic_Thread * b = new Periodic_Thread(500000,  5000,   &func2);
  Periodic_Thread * c = new Periodic_Thread(1000000, 500000, &func3);

  a -> join();
  b -> join();
  c -> join();


  delete a;
  delete b;
  delete c;

  return 0;
}

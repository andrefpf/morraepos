#include <utility/ostream.h>

using namespace EPOS;
using namespace EPOS::S;

OStream cout;

void compare(CPU::Log_Addr a, CPU::Log_Addr b) {
    cout << "\t Expected: \t {addr=" << hex << a << ", f=" << b << endl;
    cout << "\t Actual Values: \t " << MMU::Translation(a) << endl; 
    cout << endl;
}

void test_1() 
{
	cout << "Init MMU test 2" << endl;

    System_Info * si = System::info();

    cout << "App Code: " << endl;
    compare(si->lm.app_code, si->pmm.app_code);

    cout << "App Data: " << endl;
    compare(si->lm.app_data, si->pmm.app_data);

    cout << "Sys Code: " << endl;
    compare(si->lm.sys_code, si->pmm.sys_code);

    cout << "Sys Data: " << endl;
    compare(si->lm.sys_data, si->pmm.sys_data);

    cout << "Sys Stack: " << endl;
    compare(si->lm.sys_stack, si->pmm.sys_stack);

    cout << "Sys Info: " << endl;
    compare(Memory_Map::SYS_INFO, si->pmm.sys_info);

    cout << "Sys Page Table: " << endl;
    compare(Memory_Map::SYS_PT, si->pmm.sys_pt);
    
    cout << "Sys Page Directory: " << endl;
    compare(Memory_Map::SYS_PD, si->pmm.sys_pd);
}

void test_2()
{	
	static Phy_Addr * var_addr 
	cout << "Init MMU test 2" << endl;
	Address_Space self(MMU::current());
	var_addr = self.alloc(sizeof(int) * 2);
	cout << "MMU::current=(" << MMU::current() << ")"<< endl;
	compare(var_addr, MMU::current());
	cout << "MMU::ttbr0_el1()=(" << MMU::ttbr0_el1() << ")" << endl;
}
int main()
{
    test_1();
    test_2();
    return 0;
}

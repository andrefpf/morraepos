#include <utility/ostream.h>
#include <process.h>
#include <memory.h>
#include <architecture/armv8/armv8_mmu.h>
#include <architecture/armv8/armv8_cpu.h>

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

void test_2()
{	
	cout << "Init MMU test 2\n" << endl;
	
	cout << "MMU::current=(" << MMU::current() << ")\n"<< endl;
	cout << "CPU::ttbr0_el1()=(" << hex << CPU::ttbr0_el1() << ")\n" << endl;
	compare(MMU::current(), CPU::ttbr0_el1());
	cout << "Memory_Map::SYS_PD=(" << Memory_Map::SYS_PD << "\n" << endl;
}

void test_3()
{
	cout << "Init MMU test 2\n" << endl;
	
	CPU::Phy_Addr * var_addr = 0x00;
	
	cout << "Allocating sizeof(int) * 1000\n" << endl;
	var_addr = ARMv8_MMU::alloc(sizeof(int) * 1000);
	cout << "ARMv8_MMU::alloc sucefully!\n" << endl;
	ARMv8_MMU::free(var_addr);
	cout << "Memory sucefully free!\n" << endl;
}
int main()
{
    test_1();
    test_2();
    test_3();
    cout << "That's it! 毛泽东万岁 !\n" << endl;
    return 0;
}

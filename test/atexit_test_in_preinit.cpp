#include <iostream>
#include <cstdlib>
#include <string>

void dump() {
	std::cout<<"Test for calling order of c++ global object destructor and atexit call in __libc_start_main."<<std::endl;
}

class TestAtExit {
	int k;
	public:
	TestAtExit(int i) {
		k = i;
	}
	TestAtExit() {

	}
	~TestAtExit() {
		std::cout<< "This is class TestAtExit destructor !"<<std::endl;
	}
	void print(std::string s) {
		std::cout<<"String "<<s<<std::endl;
	}
};

static void debug() {
	//std::cout<<"This is function in preinit section and call atexit."<<std::endl;
	atexit(dump);
}
__attribute__((section(".preinit_array"), used))
static void (*debug_ptr)() = debug;

TestAtExit test;
int main(int argc, char** argv) {
	test.print("dog");
	//atexit(dump);
	return 0;
}

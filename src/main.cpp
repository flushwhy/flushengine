#include <vk_engine.h>

int main(int argc, char* argv[])
{
	FlushEngine engine;

	engine.init();	
	
	engine.run();	

	engine.cleanup();	

	return 0;
}

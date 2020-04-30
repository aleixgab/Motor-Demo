#include <iostream>
#include "GameManager.h"
#include <Brofiler/Brofiler.h>

#pragma comment(lib, "3rdPart\\lib\\glfw3.lib")
#pragma comment(lib, "3rdPart\\lib\\ProfilerCore64.lib")


GameManager* Mng = nullptr;
int main(int argc, char** argv)
{
	bool canContinue = true;
	Mng = new GameManager();
	canContinue = Mng->Start();
	if (!canContinue)
		std::cout << "ERROR STARTING THE ENGINE" << std::endl;


	while (canContinue)
	{
		BROFILER_FRAME("Update");
		canContinue = Mng->Update();
		if (!canContinue)
			std::cout << "STOP UPDATE" << std::endl;
	}


	Mng->CleanUp();
	return 0;
}
#include "main.hpp"

void main(){



	Connection connection(800);
	Params params(&connection);

#ifdef _PC_CONNECT
	PCControle pc(&params);
	FlowControle controle(&pc);
#else
	SeesawFlow seesaw(&params);
	SwingFlow swing(&params);
	PoleFlow pole(&params);
	JungleFlow jungle(&params);
	FlowControle controle(&seesaw,&swing,&pole,&jungle);
#endif

	controle.Start(&params);
}

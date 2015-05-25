#include "main.hpp"


void main(){
	SlaveFlow slave_flow;
	PinMode(PC9,IO_OUTPUT);
	PinMode(PC8,IO_OUTPUT);
	for(;;){
		slave_flow.Input();
		slave_flow.Output();
		IO_Write(PC9,slave_flow.connected ? HIGH : LOW);
		IO_Write(PC8,slave_flow.connected ? LOW :HIGH); 
	}

}

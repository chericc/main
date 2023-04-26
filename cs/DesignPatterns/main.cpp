
#include <Public/DPVirtualFunction.h>
#include <Public/DPAbstractFactory.h>
#include <Public/DPBuilder.h>
#include <Public/DPFactoryMethod.h>
#include <Public/DPPrototype.h>
#include <Public/DPSingleton.h>
#include <Public/DPAdapter.h>
#include <Public/DPBridge.h>
#include <Public/DPProducerConsumer.h>

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <iostream>
#include <memory>
 
using std::endl;
using std::cout;

int main()
{
	//DESIGNPATTERNS::DPVirtualFunction iDPVirtualFunction;
	//iDPVirtualFunction.Play();

	//DESIGNPATTERNS::DPAbstractFactory iDPAbstractFactory;
	//iDPAbstractFactory.Play();

	//DESIGNPATTERNS::DPBuilder iDPBuilder;
	//iDPBuilder.Play();

	//DESIGNPATTERNS::DPFactoryMethod iDPFactoryMethod;
	//iDPFactoryMethod.Play();

	//DESIGNPATTERNS::DPPrototype iDPPrototype;
	//iDPPrototype.Play();

	//DESIGNPATTERNS::DPSingleton iDPSingleton;
	//iDPSingleton.Play();

	//DESIGNPATTERNS::DPAdapter iDPAdapter;
	//iDPAdapter.Play();

	//DESIGNPATTERNS::DPBridge iDPBridge;
	//iDPBridge.Play();

	DESIGNPATTERNS::DPProducerConsumer iDPProducerConsumer;
	iDPProducerConsumer.Play();

	return 0;
}
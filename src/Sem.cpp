#include "KerSem.h"

int Semaphore::val() const{
	return myImpl->value();
}

Semaphore::~Semaphore(){
	delete myImpl;
}

 Semaphore::Semaphore(int init){
	 myImpl = new KernelSem(init);
 }

 int Semaphore::wait(Time maxTimeToWait){
	 return myImpl->wait(maxTimeToWait);
 }

int Semaphore::signal(int n){
	return myImpl->signal(n);
}

//katic


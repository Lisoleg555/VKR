#include "Globals.h"

TTimer::TTimer() {}

int TTimer::Seed() {
	srand((time(NULL)));
	return rand();
}

void TTimer::Start() {
	this->start = std::chrono::high_resolution_clock::now();
	return;
}

void TTimer::Finish() {
	this->finish = std::chrono::high_resolution_clock::now();
	return;
}

double TTimer::GetTime() {
	std::chrono::duration<double> workTime = this->finish - this->start;
	std::chrono::milliseconds result = std::chrono::duration_cast<std::chrono::milliseconds>(workTime);
	return result.count();
}

TTimer::~TTimer() {}

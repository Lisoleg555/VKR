#include "Globals.h"

double lowestEffectiveOrbit = 0.0;
double highestEffectiveOrbit = 0.0;

int masterSatellite = 0;
int deltaTime = 0;
int satelCount = 0;
int satelInWork = 0;

bool ok = true;
bool badOk = false;

std::vector<TMyMutex> mainMutex;
std::vector<TSatellite> satellites;

TDanger* danger = nullptr;

std::mutex dangerMutex;

TTimer satelClock;

int main() {
	std::string input = "y";
	while (input == "y") {
		std::cout << "Enter time delta in milliseconds: ";
		deltaTime = ReadNumber<int>();
		if (deltaTime < 1) {
			ProgramErrorExit(WRONG_TIME);
		}
		SatellitesData();
		std::cout << "If you want to check all parameters input \'y\'\n";
		std::cin >> input;
		if (!std::cin) {
			ProgramErrorExit(WRONG_INPUT);
		}
		if (input == "y") {
			for (int i = 0; i < satelCount; ++i) {
				std::cout << "\nSatellite №" << i + 1 << ":\n" << satellites[i];
			}
			std::cout << "If you want to change all parameters input \'y\'\n";
			std::cin >> input;
			if (!std::cin) {
				ProgramErrorExit(WRONG_INPUT);
			}
		}
	}
	ok = true;
	badOk = false;
	const int threadsCount = satelCount;
	mainMutex.resize(threadsCount);
	if  (mainMutex.size() != threadsCount) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	std::vector<std::future<void>> moveThreads(threadsCount);
	try {
		TMyMutex tmp;
		for (int i = 0; i < satelCount; ++i) {
			tmp = std::make_unique<std::mutex>();
			mainMutex[i] = std::move(tmp);
			moveThreads[i] = std::async(std::launch::async, ThreadMove, i);
		}
	}
	catch (const std::future_error& e) {
		ProgramErrorExit(THREAD_ERROR);
	}
	std::cout << "Starting modeling.\n";
	int waitTime = 1;
	satelInWork = satelCount;
	while (ok && std::cin && satelInWork > 1) {
		std::cout << "Modeling options:\n\t\'s\' - spawn danger object\n\t\'p\' - print system status\n\t"
				  << "\'w\' - wait for some time and print system status\n\t\'q\' - quit\nYour option: ";
		std::cin >> input;
		switch (input[0]) {
			case 's':
				SpawnDanger();
				break;
			case 'w':
				std::cout << "Input time you want to wait in milliseconds before printing system status: ";
				waitTime = ReadNumber<int>();
				std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
			case 'p':
				PrintSatellitesInfo();
				break;
			case 'q':
				ok = false;
				break;
			default:
				std::cout << "No such option!\n";
				break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP));
	}
	badOk = false;
	ok = false;
	for (int i = 0; i < threadsCount; ++i) {
		moveThreads[i].wait();
	}
	mainMutex.clear();
	satellites.clear();
	return 0;
}

#include "Globals.h"

void ProgramErrorExit(int signal) {
	std::cout << "Error: ";
	switch (signal) {
		case WRONG_COUNT_SATELLITES:
			std::cout << "Not enough satellites for group!\n";
			break;
		case WRONG_INPUT:
			std::cout << "Wrong data type input!\n";
			break;
		case WRONG_ORBIT:
			std::cout << "Wrong orbit input!\n";
			break;
		case WRONG_FUEL:
			std::cout << "Wrong fuel input!\n";
			break;
		case WRONG_MASTER:
			std::cout << "Wrong master index input!\n";
			break;
		case WRONG_SAME_COORDINATES:
			std::cout << "Same coordinates were input for two satellites!\n";
			break;
		case WRONG_TIME:
			std::cout << "Wrong delta time input!\n";
			break;
		case WRONG_SIZE:
			std::cout << "Wrong size of danger object!\n";
			break;
		case MEMORY_ERROR:
			std::cout << "Can\'t allocate memory!\n";
			break;
		case THREAD_ERROR:
			std::cout << "Can\'t make new thread!\n";
			break;
		case MUTEX_ERROR:
			std::cout << "Can't use mutex!\n";
			break;
	}
	ok = false;
	badOk = false;
	std::this_thread::sleep_for(std::chrono::milliseconds(2 * deltaTime));
	if (danger != nullptr) {
		delete danger;
	}
	satellites.clear();
	mainMutex.clear();
	exit(0);
}

bool IsNumber(std::string a, double* answer) {
	char* end = nullptr;
	*answer = strtod(a.c_str(), &end);
	return end != a.c_str() && *end == '\0' && (*answer) != HUGE_VAL;
}

template<typename Type> Type ReadNumber() {
	std::string input = "";
	std::cin >> input;
	double answer = 0.0;
	if (!std::cin || !IsNumber(input, &answer)) {
		ProgramErrorExit(WRONG_INPUT);
	}
	return answer;
}

void CheckSame(int max) {
	for (int i = 0; i < max; ++i) {
		if (satellites[i].radius == satellites[max].radius && satellites[i].phi == satellites[max].phi
			&& satellites[i].teta == satellites[max].teta) {
			ProgramErrorExit(WRONG_SAME_COORDINATES);
		}
	}
	return;
}

void SameRadius(double amount) {
	if (masterSatellite == -1) {
		std::cout << "Enter height. if height < " << lowestEffectiveOrbit << " or height > " << highestEffectiveOrbit
				  << " - programm ends\n";
	}
	std::mt19937 random_gen(satelClock.Seed());
	std::uniform_real_distribution<double> fuelArea(0.0, MAX_RANDOM_FUEL);
	std::uniform_real_distribution<double> radiusArea(lowestEffectiveOrbit, highestEffectiveOrbit);
	std::uniform_real_distribution<double> phiArea(0.0, TWO_PI);
	std::uniform_real_distribution<double> tetaArea(0.0, M_PI);
	std::uniform_real_distribution<double> typeArea(0.0, 2.0);
	satellites[0].radius = (masterSatellite == 0 ? radiusArea(random_gen) : ReadNumber<double>());
	if (satellites[0].radius < lowestEffectiveOrbit || satellites[0].radius > highestEffectiveOrbit) {
		ProgramErrorExit(WRONG_ORBIT);
	}
	if (masterSatellite == -1) {
		std::cout << "Enter orbit type - meridian(m) or equator(else), phi, " << (badOk ? "and teta"
				   : "teta and start fuel") << " for satellites.\nAngles values are in radians\n" << (badOk ? ""
				   : "if start fuel < 0 - programm ends\n");
	}
	std::string input = "";
	for (int i = 0; i < satelCount; ++i) {
		if (masterSatellite == -1) {
			std::cout << "Enter data for satellite №" << i + 1 << ":\n\torbit type: ";
		}
		satellites[i].radius = satellites[0].radius;
		if (masterSatellite == -1) {
			std::cin >> input;
			if (!std::cin) {
				ProgramErrorExit(WRONG_INPUT);
			}
		}
		satellites[i].type = (masterSatellite == 0 ? (typeArea(random_gen) > 1.0) : (input != "m"));
		if (masterSatellite == -1) {
			std::cout << "\tphi: ";
		}
		satellites[i].phi = (masterSatellite == 0 ? phiArea(random_gen) : std::fmod(ReadNumber<double>(), TWO_PI));
		if (!satellites[i].type && masterSatellite == -1) {
			std::cout << "\tteta: ";
		}
		satellites[i].teta = (satellites[i].type ? HALF_PI : (masterSatellite == 0 ? tetaArea(random_gen)
						   : std::fmod(ReadNumber<double>(), M_PI)));
		CheckSame(i);
		if (!badOk && masterSatellite == -1) {
			std::cout << "\tfuel: ";
		}
		satellites[i].fuel = (badOk ? amount : (masterSatellite == 0 ? fuelArea(random_gen) : ReadNumber<double>()));
		if (satellites[i].fuel < 0.0) {
			ProgramErrorExit(WRONG_FUEL);
		}
		satellites[i].CalculateVelocity();
		satellites[i].BuildUsual();
	}
	return;
}

void DifferentRadius(double amount) {
	if (masterSatellite == -1) {
		std::cout << "Enter height, orbit type - meridian(m) or equator(else), phi, " << (badOk ? "and teta"
				   : "teta and start fuel")  << " for satellites.nAngles values are in radians\nif height < "
				  << lowestEffectiveOrbit << " or height > " << highestEffectiveOrbit << (badOk ? ""
				   : " or start fuel < 0") << " - programm ends\n";
	}
	std::mt19937 random_gen(satelClock.Seed());
	std::uniform_real_distribution<double> fuelArea(0.0, MAX_RANDOM_FUEL);
	std::uniform_real_distribution<double> radiusArea(lowestEffectiveOrbit, highestEffectiveOrbit);
	std::uniform_real_distribution<double> phiArea(0.0, TWO_PI);
	std::uniform_real_distribution<double> tetaArea(0.0, M_PI);
	std::uniform_real_distribution<double> typeArea(0.0, 2.0);
	std::string input = "";
	for (int i = 0; i < satelCount; ++i) {
		if (masterSatellite == -1) {
			std::cout << "Enter data for satellite №" << i + 1 << ":\n\theight: ";
		}
		satellites[i].radius = (masterSatellite == 0 ? radiusArea(random_gen) : ReadNumber<double>());
		if (satellites[i].radius < lowestEffectiveOrbit || satellites[i].radius > highestEffectiveOrbit) {
			ProgramErrorExit(WRONG_ORBIT);
		}
		if (masterSatellite == -1) {
			std::cout << "\torbit type: ";
			std::cin >> input;
			if (!std::cin) {
				ProgramErrorExit(WRONG_INPUT);
			}
		}
		satellites[i].type = (masterSatellite == 0 ? (typeArea(random_gen) > 1.0) : input != "m");
		if (masterSatellite == -1) {
			std::cout << "\tphi: ";
		}
		satellites[i].phi = (masterSatellite == 0 ? phiArea(random_gen) : std::fmod(ReadNumber<double>(), TWO_PI));
		if (!satellites[i].type && masterSatellite == -1) {
			std::cout << "\tteta: ";
		}
		satellites[i].teta = (satellites[i].type ? HALF_PI : (masterSatellite == 0 ? tetaArea(random_gen)
						   : std::fmod(ReadNumber<double>(), M_PI)));
		CheckSame(i);
		if (!badOk && masterSatellite == -1) {
			std::cout << "\tfuel: ";
		}
		satellites[i].fuel = (badOk ? amount :(masterSatellite == 0 ? fuelArea(random_gen) : ReadNumber<double>()));
		if (satellites[i].fuel < 0.0) {
			ProgramErrorExit(WRONG_FUEL);
		}
		satellites[i].CalculateVelocity();
		satellites[i].BuildUsual();
	}
	return;
}

void SatellitesData() {
	std::cout << "Enter count of satellites: ";
	satelCount = ReadNumber<int>();
	if (satelCount < 2) {
		ProgramErrorExit(WRONG_COUNT_SATELLITES);
	}
	std::cout << "Input lowest effective orbit for group.\nif it is less than " << LOW_ORBIT << " - programm ends\n";
	lowestEffectiveOrbit = ReadNumber<double>();
	if (lowestEffectiveOrbit < LOW_ORBIT) {
		ProgramErrorExit(WRONG_ORBIT);
	}
	std::cout << "Input highest effective orbit for group.\nif it is more than " << HIGH_ORBIT << " - programm ends\n";
	highestEffectiveOrbit = ReadNumber<double>();
	if (highestEffectiveOrbit > HIGH_ORBIT) {
		ProgramErrorExit(WRONG_ORBIT);
	}
	std::string input = "n";
	std::cout << "Are satellites on one height? input \'y\' for using same radius, else for using different radiuses: ";
	std::cin >> input;
	if (!std::cin) {
		ProgramErrorExit(WRONG_INPUT);
	}
	ok = (input == "y");
	satellites.resize(satelCount);
	if (satellites.size() != satelCount) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	std::cout << "Do satellites have same amount of fuel? input \'y\' for having same amount of fuel, "
			  << "else for different amount: ";
	std::cin >> input;
	if (!std::cin) {
		ProgramErrorExit(WRONG_INPUT);
	}
	badOk = (input == "y");
	std::cout << "If you want to use random number generator, input \'y\'. Input something else for hand input: ";
	std::cin >> input;
	if (!std::cin) {
		ProgramErrorExit(WRONG_INPUT);
	}
	masterSatellite = (input == "y" ? 0 : -1);
	if (badOk && masterSatellite == -1) {
		std::cout << "Input amount of fuel for satellites: ";
	}
	std::mt19937 random_gen(satelClock.Seed());
	std::uniform_real_distribution<double> fuelArea(0.0, MAX_RANDOM_FUEL);
	double amount = (badOk ? (masterSatellite == 0 ? fuelArea(random_gen) : ReadNumber<double>()) : -1.0);
	if (ok) {
		SameRadius(amount);
	}
	else {
		DifferentRadius(amount);
	}
	std::uniform_real_distribution<double> masterArea(0, satelCount);
	if (masterSatellite == -1) {
		std::cout << "Choose master-satellite: ";
	}
	masterSatellite = (masterSatellite == 0 ? masterArea(random_gen) : ReadNumber<int>() - 1) ;
	if (masterSatellite < 0 || masterSatellite >= satelCount) {
		ProgramErrorExit(WRONG_MASTER);
	}
	satellites[masterSatellite].BuildMaster();
	return;
}

std::string CountSuffix(int number) {
	switch (number % 10) {
		case 1:
			return "st";
		case 2:
			return "nd";
		case 3:
			return "rd";
	}
	return "th";
}

void ThreadMove(const int number) {
	while (ok && satellites[number].GetStatus() != DESTROYED) {
		try {
			mainMutex[number]->lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
		satellites[number].Move();
		mainMutex[number]->unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(deltaTime));
	}
	return;
}

bool Clash(int number) {
	double x1 = (EARTH_RADIUS + danger->radius) * sin(danger->teta) * cos(danger->phi);
	double y1 = (EARTH_RADIUS + danger->radius) * sin(danger->teta) * sin(danger->phi);
	double z1 = (EARTH_RADIUS + danger->radius) * cos(danger->teta);
	
	double x2 = (EARTH_RADIUS + satellites[number].radius) * sin(satellites[number].teta) * cos(satellites[number].phi);
	double y2 = (EARTH_RADIUS + satellites[number].radius) * sin(satellites[number].teta) * sin(satellites[number].phi);
	double z2 = (EARTH_RADIUS + satellites[number].radius) * cos(satellites[number].teta);
	
	x1 = (x1 - x2) * (x1 - x2);
	y1 = (y1 - y2) * (y1 - y2);
	z1 = (z1 - z2) * (z1 - z2);
	return sqrt(x1 + y1 + z1) <= danger->size + EPS_SIZE;
}

void StartBreak() {
	int i = 0;
	for (; i < mainMutex.size(); ++i) {
		try {
			mainMutex[i]->lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
	}
	for (i = 0; i < mainMutex.size(); ++i) {
		if (Clash(i)) {
			satellites[i].ChangeStatus(DESTROYED);
			--satelInWork;
			std::cout << "Satellite №" << i + 1 << " was destroyed in moment of detection.\n";
		}
	}
	for (i = 0; i < mainMutex.size(); ++i) {
		mainMutex[i]->unlock();
	}
	return;
}

int CountNotDamaged() {
	int result = 0;
	for (int i = 0; i < satelCount; result += (satellites[i].GetStatus() != DESTROYED ? 1 : 0), ++i);
	return result;
}

std::vector<int> FasterSearch() {
	std::vector<int> inDanger;
	const double dangerZone = danger->size + 2 * EPS_SIZE;
	for (int i = 0; i < satelCount; ++i) {
		if (satellites[i].GetStatus() != DESTROYED && abs(satellites[i].radius - danger->radius) < dangerZone)  {
			inDanger.push_back(i);
		}
	}
	return inDanger;
}

void MoveDanger(std::vector<int> inDanger) {
	int inDangerSize = inDanger.size();
	int i = 0;
	while (badOk && inDangerSize > 0) {
		try {
			dangerMutex.lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
		danger->Move();
		dangerMutex.unlock();
		for (; i < inDangerSize; ++i) {
			try {
				mainMutex[inDanger[i]]->lock();
			}
			catch (const std::system_error& e) {
				ProgramErrorExit(MUTEX_ERROR);
			}
			if (Clash(i)) {
				--satelInWork;
				satellites[i].ChangeStatus(DESTROYED);
				std::cout << "Satellite №" << i + 1 << " was destroyed.\n";
				mainMutex[inDanger[i]]->unlock();
				--inDangerSize;
				inDanger.erase(inDanger.begin() + i);
				--i;
				continue;
			}
			mainMutex[inDanger[i]]->unlock();
		}
		i = 0;
		std::this_thread::sleep_for(std::chrono::milliseconds(deltaTime));
	}
	return;
}

void CommonDanger(int number, int start, int finish) {
	satellites[number].CommonGetDanger(start, finish);
	return;
}

void SpawnDanger() {
	danger = new TDanger();
	if (danger == nullptr) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	if (danger->radius < LOW_ORBIT) {
		delete danger;
		std::cout << "Danger fell on Earth.\n";
		return;
	}
	else if (danger->radius > HIGH_ORBIT) {
		delete danger;
		std::cout << "Danger went to outer space.\n";
		return;
	}
	int satelCountAtEvadeStart = satelInWork;
	StartBreak();
	int notDamaged = CountNotDamaged();
	if (notDamaged <= 1) {
		std::cout << "Group was destroyed! One or zero satellites left!\n";
		ok = false;
		return;
	}
	badOk = true;
	std::future<void> dangerThread;
	try {
		dangerThread = std::async(std::launch::async, MoveDanger, FasterSearch());
	}
	catch (const std::future_error& e) {
		ProgramErrorExit(THREAD_ERROR);
	}
	satelClock.Start();
	satellites[masterSatellite].Evade(notDamaged, satelCountAtEvadeStart);
	dangerThread.wait();
	delete danger;
	return;
}

void PrintSatellitesInfo() {
	for (int i = 0; i < satelCount; ++i) {
		try {
			mainMutex[i]->lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
		std::cout << "\nSatellite №" << i + 1 << ": " << (satellites[i].GetStatus() == DESTROYED
				   ? "destroyed, last data:" : "") << "\n" << satellites[i];
		mainMutex[i]->unlock();
	}
	return;
}

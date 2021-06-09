#include "Globals.h"

TSatellite::TSatellite() {
	this->radius = 0.0;
	this->phi = 0.0;
	this->teta = 0.0;
	this->type = false;
	this->fuel = 0.0;
	this->usual = nullptr;
	this->master = nullptr;
	this->velocity = 0.0;
}

void TSatellite::CalculateVelocity() {
	double r = (this->radius + EARTH_RADIUS) * KM;
	this->velocity = sqrt(GM / r) / r * VELOCITY_CONSTANT;
	return;
}

void TSatellite::BuildUsual() {
	this->usual = new TPetry;
	if (this->usual == nullptr) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	this->usual->BuildUsual();
	return;
}

void TSatellite::BuildMaster() {
	this->master = new TPetry;
	if (this->master == nullptr) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	this->master->BuildMaster();
	return;
}

int TSatellite::GetStatus() {
	return this->usual->GetStatus();
}

void TSatellite::Move() {
	if (this->type) {
		this->phi = std::fmod(this->phi + this->velocity * deltaTime, TWO_PI);
	}
	else {
		this->teta += (this->phi > M_PI ? -1 : 1) * this->velocity * deltaTime;
		if (this->teta > M_PI) {
			this->teta = M_PI - std::fmod(this->teta, M_PI);
			this->phi += M_PI;
		}
		else if (this->teta < 0) {
			this->teta *= -1;
			this->phi -= M_PI;
		}
	}
	return;
}

void TSatellite::ChangeStatus(int newStatus) {
	this->usual->ChangeStatus(newStatus);
	return;
}

void TSatellite::RelocateMaster(TPetry* master, int newMaster) {
	this->master = master;
	masterSatellite = newMaster;
	return;
}

void TSatellite::DangerStatus(int status) {
	this->usual->DangerStatus(status);
	return;
}

void TSatellite::CommonGetDanger(int start, int finish) {
	this->usual->CommonGetDanger(start, finish);
	return;
}

void TSatellite::MasterOrbitCalculation(int notDamaged, int atEvadeStart) {
	std::vector<std::future<void>> satelThreads;
	if (notDamaged == atEvadeStart) {
		int divider = satelCount;
		if (satelCount > 100 && satelInWork > 4) {
			divider /= 5;
			satelThreads.resize(5);
		}
		else if (satelCount > 50 && satelInWork > 3) {
			divider /= 4;
			satelThreads.resize(4);
		}
		else if (satelCount > 25 && satelInWork > 2) {
			divider /= 3;
			satelThreads.resize(3);
		}
		else  {
			divider /= 2;
			satelThreads.resize(2);
		}
		int j = 0;
		int i = 0;
		for (; j < satelThreads.size() - 1 && i < satelCount; ++i) {
			if (satellites[i].GetStatus() != DESTROYED) {
				try {
					mainMutex[i]->lock();
				}
				catch (const std::system_error& e) {
					ProgramErrorExit(MUTEX_ERROR);
				}
				try {
					satelThreads[j] = std::async(std::launch::async, CommonDanger, i, divider * i, divider * (i + 1));
				}
				catch (const std::future_error& e) {
					ProgramErrorExit(THREAD_ERROR);
				}
				mainMutex[i]->unlock();
				++j;
			}
		}
		while (i < satelCount) {
			if (satellites[i].GetStatus() != DESTROYED) {
				try {
					mainMutex[i]->lock();
				}
				catch (const std::system_error& e) {
					ProgramErrorExit(MUTEX_ERROR);
				}
				try {
					satelThreads.back() = std::async(std::launch::async, CommonDanger, i, divider * i, satelCount);
				}
				catch (const std::future_error& e) {
					ProgramErrorExit(THREAD_ERROR);
				}
				mainMutex[i]->unlock();
				break;
			}
			++i;
		}
		for (i = 0; i < satelThreads.size() - 1; ++i) {
			satelThreads[i].wait();
			this->master->MasterOrbitCalculation(i * divider, (i + 1) * divider);
		}
		satelThreads.back().wait();
		this->master->MasterOrbitCalculation(i * divider, satelCount);
	}
	else {
		for (int i = 0; i < satelCount; ++i) {
			if (satellites[i].GetStatus() != DESTROYED) {
				satelThreads.push_back(std::async(std::launch::async, CommonDanger, i, i, i + 1));
			}
		}
		for (int i = 0; i < satelThreads.size(); ++i) {
			satelThreads[i].wait();
		}
		this->master->MasterOrbitCalculation(0, satelCount);
	}
	return;
}

void TSatellite::CommonSafeReadyChange(int number) {
	int status = this->GetStatus();
	if (status == NORMAL) {
		this->usual->CommonSafe(number);
	}
	else if (status == IN_DANGER) {
		this->usual->CommonReadyChange(number);
	}
	return;
}

void TSatellite::Common() {
	std::vector<std::future<void>> commons(satelCount);
	for (int i = 0; i < satelCount; ++i) {
		try {
			commons[i] = std::async(std::launch::async, &TSatellite::CommonSafeReadyChange, &satellites[i], i);
		}
		catch (const std::future_error& e) {
			ProgramErrorExit(THREAD_ERROR);
		}
	}
	for (int i = 0; i < satelCount; ++i) {
		commons[i].wait();
	}
	return;
}

void TSatellite::PetryParallel() {
	std::future<void> common, masterSafe, masterStartEscape;
	try {
		common = std::async(std::launch::async, &TSatellite::Common, this);
		masterSafe = std::async(std::launch::async, &TPetry::MasterSafe, this->master);
		masterStartEscape = std::async(std::launch::async, &TPetry::MasterStartEscape, this->master);
	}
	catch (const std::future_error& e) {
		ProgramErrorExit(THREAD_ERROR);
	}
	masterSafe.wait();
	masterStartEscape.wait();
	this->master->ClearD();
	common.wait();
	return;
}

double TSatellite::CountNewOrbit(bool inDanger, int number) {
	if (inDanger) {
		std::this_thread::sleep_for(std::chrono::milliseconds(DANGER_SLEEP));
	}
	double lowHeight = danger->radius - danger->size - EPS_SAFE;
	double highHeight = danger->radius + danger->size + EPS_SAFE;
	if (lowHeight < LOW_ORBIT) {
		return highHeight;
	}
	else if (highHeight > HIGH_ORBIT) {
		return lowHeight;
	}
	double a = (lowHeight < lowestEffectiveOrbit ? 2.0 : 1.0);
	double b = (highHeight > highestEffectiveOrbit ? 2.0 : 1.0) ;
	return (this->radius - a * lowHeight < b * highHeight - this->radius ? lowHeight : highHeight);
}

int TSatellite::TimeCount(int number, double dangerPhi, double dangerTeta, double newOrbit) {
	double dangerVelocity = (danger->radius + EARTH_RADIUS) * KM;
	dangerVelocity = sqrt(GM / dangerVelocity) / dangerVelocity * VELOCITY_CONSTANT;
	try {
		mainMutex[number]->lock();
	}
	catch (const std::system_error& e) {
		ProgramErrorExit(MUTEX_ERROR);
	}
	double satelPhi = this->phi;
	double satelTeta = this->teta;
	mainMutex[number]->unlock();
	satelPhi = (!this->type ? (satelPhi > M_PI ? TWO_PI - satelTeta : satelTeta) : satelPhi);
	dangerPhi = (!danger->type ? (dangerPhi > M_PI ? TWO_PI - dangerTeta : dangerTeta) : dangerPhi);
	satelPhi = dangerPhi - satelPhi + (dangerPhi < satelPhi ? TWO_PI : 0.0);
	satelPhi /= (dangerVelocity / 2.0);
	return satelPhi + 2.0;
}

void TSatellite::FastMove(int time) {
	if (this->type) {
		this->phi = std::fmod(this->phi + this->velocity * time, TWO_PI);
	}
	else {
		this->teta += (this->phi > M_PI ? -1 : 1) * this->velocity * time;
		this->teta = M_PI - std::fmod(this->teta, TWO_PI);
		if (this->teta > M_PI && this->phi > M_PI) {
			this->teta = M_PI - std::fmod(this->teta, M_PI);
		}
		else if (this->teta > M_PI) {
			this->teta = M_PI - std::fmod(this->teta, M_PI);
			this->phi += M_PI;
		}
		else if (this->teta < 0 && this->phi > M_PI) {
			this->teta *= -1;
			this->phi -= M_PI;
		}
		else if (this->teta < 0) {
			this->teta *= -1;
		}
	}
	return;
}

void TSatellite::CommonEscape(double newOrbit, int time, int number, double oldOrbit, bool fast, int status) {
	if (status == DESTROYED) {
		this->usual->CommonEscape(status);
		return;
	}
	int timesSleep = DELTA_COUNT / deltaTime;
	const int timeRest = DELTA_COUNT % deltaTime;
	double kmPerSleep = (newOrbit - oldOrbit) / timesSleep;
	for (int i = 0; i < timesSleep; ++i) {
		try {
			mainMutex[number]->lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
		this->radius += kmPerSleep;
		mainMutex[number]->unlock();
		this->fuel -= (kmPerSleep * FUEL_PER_KM * (kmPerSleep >= 0 ? 1 : -1));
		if (!fast) {
			std::this_thread::sleep_for(std::chrono::milliseconds(deltaTime));
		}
	}
	try {
		mainMutex[number]->lock();
	}
	catch (const std::system_error& e) {
		ProgramErrorExit(MUTEX_ERROR);
	}
	this->fuel -= ((this->radius - newOrbit) * FUEL_PER_KM * (this->radius - newOrbit >= 0 ? 1 : -1));
	this->radius = newOrbit;
	this->CalculateVelocity();
	mainMutex[number]->unlock();
	if (!fast) {
		std::this_thread::sleep_for(std::chrono::milliseconds(timeRest));
	}
	if (time - DELTA_COUNT > 0) {
		if (fast) {
			try {
				mainMutex[number]->lock();
			}
			catch (const std::system_error& e) {
				ProgramErrorExit(MUTEX_ERROR);
			}
			this->FastMove(time - DELTA_COUNT);
			mainMutex[number]->unlock();
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(time - DELTA_COUNT));
		}
	}
	if (time > 0) {
		for (int i = 0; i < timesSleep; ++i) {
			try {
				mainMutex[number]->lock();
			}
			catch (const std::system_error& e) {
				ProgramErrorExit(MUTEX_ERROR);
			}
			this->radius -= kmPerSleep;
			mainMutex[number]->unlock();
			this->fuel -= (kmPerSleep * FUEL_PER_KM * (kmPerSleep >= 0 ? 1 : -1));
			if (!fast) {
				std::this_thread::sleep_for(std::chrono::milliseconds(deltaTime));
			}
		}
		try {
			mainMutex[number]->lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
		this->fuel -= ((this->radius - newOrbit) * FUEL_PER_KM * (this->radius - newOrbit >= 0 ? 1 : -1));
		this->radius = oldOrbit;
		this->CalculateVelocity();
		mainMutex[number]->unlock();
		if (!fast) {
			std::this_thread::sleep_for(std::chrono::milliseconds(timeRest));
		}
	}
	this->fuel = (this->fuel < 0.0 ? 0.0 : this->fuel);
	this->usual->CommonEscape(status);
	return;
}

void TSatellite::EvadeResult(int notDamaged, int atEvadeStart, bool fast) {
	int newNotDamaged = CountNotDamaged();
	if (newNotDamaged == 0) {
		std::cout << "Group was destroyed! One or zero satellites left!\n";
		ok = false;
		return;
	}
	else if (notDamaged == newNotDamaged) {
		std::cout << "Completely successful evade! No " << (notDamaged != atEvadeStart ? "more " : "")
				  << "satellites destroyed!\n"; 
	}
	else {
		std::cout << "Evade made! " << notDamaged - newNotDamaged << " more satellites still were lost!\n";
	}
	if (!fast) {
		std::cout << "Needed time: " << satelClock.GetTime() << " milliseconds\n";
	}
	return;
}

void TSatellite::Evade(int notDamaged, int atEvadeStart) {
	if (this->usual->GetStatus() == DESTROYED) {
		for (int i = 0; i < satelCount; ++i) {
			if (satellites[i].GetStatus() != DESTROYED) {
				satellites[i].RelocateMaster(this->master, i);
				this->master = nullptr;
				satellites[i].Evade(notDamaged, atEvadeStart);
				return;
			}
		}
	}
	bool fast = true;
	this->MasterOrbitCalculation(notDamaged, atEvadeStart);
	this->PetryParallel();
	int sizeI = this->master->ISize();
	if (sizeI > 0) {
		std::vector<std::pair<int, double>> inDanger(sizeI);
		std::vector<std::pair<double, int>> newOrbits(sizeI);
		this->master->MasterGetNewOrbits(newOrbits, inDanger);
		fast = false;
		for (int i = 0; i < sizeI && !fast; ++i) {
			fast = (newOrbits[i].second > 0);
		}
		if (fast) {
			std::cout << "Some satellites need to spend time on temporary orbits:\n";
			for (int i = 0; i < sizeI; ++i) {
				if (newOrbits[i].second > 0) {
					std::cout << "\tSatellite №" << inDanger[i].first + 1 << ": " << newOrbits[i].second << "\n";
				}
			}
		}
		std::cout << "Input \'y\' for very fast changing orbits. Else for normal speed: ";
		std::string input = "";
		std::cin >> input;
		if (!std::cin) {
			ProgramErrorExit(WRONG_INPUT);
		}
		fast = (input == "y");
		this->master->MasterEscape(newOrbits, inDanger, fast);
	}
	this->master->Sort();
	satelClock.Finish();
	this->EvadeResult(notDamaged, atEvadeStart, fast);
	return;
}

std::ostream& operator << (std::ostream& os, const TSatellite& obj) {
	os << "\theight = " << obj.radius << "\n\tphi = " << obj.phi << "\n\tteta = " << obj.teta << "\n\tvelocity = "
	   << obj.velocity << " rad/millisecond\n\t" << (obj.type ? "equator" : "meridian")
	   << " orbit type\n\tlowest effective orbit = " << lowestEffectiveOrbit << "\n\thighest effective orbit = "
	   << highestEffectiveOrbit << "\n\tcount of fuel = " << obj.fuel << "\nusual Petry net:\n" << *obj.usual
	   << (obj.master == nullptr ? "not a master\n" : "master Petry net:\n");
	if (obj.master != nullptr) {
		os << *obj.master;
	}
	return os;
}

TSatellite::~TSatellite() {
	if (this->usual != nullptr) {
		delete this->usual;
	}
	if (this->master != nullptr) {
		delete this->master;
	}
}

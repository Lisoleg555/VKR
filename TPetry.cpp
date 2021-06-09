#include "Globals.h"

TPetry::TPetry() {}

void TPetry::BuildUsual() {
	this->P = {{{0, NORMAL}}, {}, {}};
	if (this->P.empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	this->I = {{{0, NORMAL}}, {{1, NORMAL}}, {{1, IN_DANGER}}, {{2, IN_DANGER}}};
	if (this->I.empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	this->O = {{1}, {0}, {2}, {0}};
	if (this->O.empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	return;
}

void TPetry::BuildMaster() {
	this->P = {{}, {}, {}, {}, {}};
	if (this->P.empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	this->P[0].resize(satelCount);
	if (this->P[0].empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	for (int i = 0; i < satelCount; ++i) {
		this->P[0][i] = {i, NORMAL};
	}
	this->I = {{{0, NORMAL}}, {{1, NORMAL}}, {{1, IN_DANGER}}, {{2, IN_DANGER}}, {{3, ANY}, {4, ANY}}};
	if (this->I.empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	this->O = {{1}, {0}, {2, 3}, {4}, {0}};
	if (this->O.empty()) {
		ProgramErrorExit(MEMORY_ERROR);
	}
	return;
}

int TPetry::GetStatus() {
	int result = 0;
	for (int i = 0; i < this->P.size(); ++i) {
		if (this->P[i].size() > 0) {
			result = this->P[i][0].second;
			break;
		}
	}
	return result;
}

void TPetry::ChangeStatus(int newStatus) {
	int i = 0;
	for (; !this->P[i].empty(); ++i);
	this->P[i - 1][0].second = newStatus;
	return;
}

void TPetry::DangerStatus(int status) {
	this->P[this->I[0][0].first] = {};
	this->P[this->O[0][0]].push_back({0, status});
	return;
}

void TPetry::CommonGetDanger(int start, int finish) {
	const double dangerZone = danger->size + 2 * EPS_SIZE;
	for (int i = start; i < finish; ++i) {
		try {
			mainMutex[i]->lock();
		}
		catch (const std::system_error& e) {
			ProgramErrorExit(MUTEX_ERROR);
		}
		if (satellites[i].GetStatus() == NORMAL) {
			satellites[i].DangerStatus(abs(satellites[i].radius - danger->radius) < dangerZone ? IN_DANGER : NORMAL);
		}
		mainMutex[i]->unlock();
	}
	return;
}

void TPetry::MasterOrbitCalculation(int start, int finish) {
	int status = 0;
	int previous = P[this->O[0][0]].size();
	for (int i = start; i < finish; ++i) {
		status = satellites[i].GetStatus();
		if (status != DESTROYED) {
			this->P[this->O[0][0]].push_back({this->P[this->I[0][0].first][i - previous].first, status});
			this->P[this->I[0][0].first].erase(this->P[this->I[0][0].first].begin() + i - previous);
			++previous;
		}
		else {
			this->P[this->I[0][0].first][i - previous].second = DESTROYED;
		}
	}
	return;
}

void TPetry::CommonSafe(int number) {
	try {
		mainMutex[number]->lock();
	}
	catch (const std::system_error& e) {
		ProgramErrorExit(MUTEX_ERROR);
	}
	this->P[this->O[1][0]].push_back(this->P[this->I[1][0].first][0]);
	this->P[this->I[1][0].first] = {};
	mainMutex[number]->unlock();
	return;
}

void TPetry::CommonReadyChange(int number) {
	try {
		mainMutex[number]->lock();
	}
	catch (const std::system_error& e) {
		ProgramErrorExit(MUTEX_ERROR);
	}
	this->P[this->O[2][0]].push_back(this->P[this->I[2][0].first][0]);
	this->P[this->I[2][0].first] = {};
	mainMutex[number]->unlock();
	return;
}

void TPetry::MasterSafe() {
	int end = P[this->I[1][0].first].size();
	for (int i = 0; i < end; ++i) {
		if (this->P[this->I[1][0].first][i].second == NORMAL) {
			this->P[this->O[1][0]].push_back(this->P[this->I[1][0].first][i]);
		}
	}
	return;
}

void TPetry::MasterStartEscape() {
	int end = P[this->I[2][0].first].size();
	int fuelStatus = 0;
	for (int i = 0; i < end; ++i) {
		if (this->P[this->I[2][0].first][i].second == IN_DANGER) {
			this->P[this->O[2][0]].push_back(this->P[this->I[2][0].first][i]);
			fuelStatus = (satellites[this->P[this->I[2][0].first][i].first].fuel > 0.0 ? NORMAL : IN_DANGER);
			this->P[this->O[2][1]].push_back({this->P[this->I[2][0].first][i].first, fuelStatus});
		}
	}
	return;
}

void TPetry::ClearD() {
	this->P[1].clear();
	return;
}

int TPetry::ISize() {
	return this->P[2].size();
}

void TPetry::MasterRecover(std::vector<std::pair<double, int>>& newOrbits) {
	for (int i = 0; i < this->P[2].size(); ++i) {
		this->P[this->O[3][0]].push_back({this->P[this->I[3][0].first][i].first, (newOrbits[i].second == 0
										  ? NORMAL : IN_DANGER)});
	}
	this->P[2].clear();
	return;
}

void TPetry::MasterGetNewOrbits(std::vector<std::pair<double, int>>& newOrbits,
								std::vector<std::pair<int, double>>& inDanger) {
	std::vector<int> normal;
	int i = 0;
	int status = 0;
	int position = 0;
	for (; i < satelCount; ++i) {
		status = satellites[i].GetStatus();
		if (status == NORMAL) {
			normal.push_back(i);
		}
		else if (status == IN_DANGER) {
			inDanger[position].first = i;
			inDanger[position].second = satellites[i].radius;
			++position;
		}
	}
	int inDangerSize = inDanger.size();
	std::vector<std::future<double>> forCount(inDangerSize);
	int border = (normal.size() >= inDangerSize ? inDangerSize : normal.size());
	try  {
		for (i = 0; i < border; ++i) {
			forCount[i] = std::async(std::launch::async, &TSatellite::CountNewOrbit, &satellites[normal[i]],
									 false, inDanger[i].first);
		}
		for (; i < inDangerSize; ++i) {
			forCount[i] = std::async(std::launch::async, &TSatellite::CountNewOrbit,
									 &satellites[inDanger[i - border].first], true, inDanger[i - border].first);
		}
	}
	catch (const std::future_error& e) {
		ProgramErrorExit(THREAD_ERROR);
	}
	for (i = 0; i < inDangerSize; ++i) {
		newOrbits[i].first = forCount[i].get();
		newOrbits[i].second = 0;
	}
	int metMax = 0;
	int metMin = 0;
	try {
		dangerMutex.lock();
	}
	catch (const std::system_error& e) {
		ProgramErrorExit(MUTEX_ERROR);
	}
	double dangerPhi = danger->phi;
	double dangerTeta = danger->teta;
	dangerMutex.unlock();
	std::vector<std::pair<std::future<int>, int>> timeThreads;
	for (i = 0; i < border; ++i) {
		if (newOrbits[i].first < lowestEffectiveOrbit && newOrbits[i].first - 0.5 * metMin > LOW_ORBIT) {
			newOrbits[i].first -= (0.5 * metMin);
			++metMin;
		}
		else if (newOrbits[i].first > highestEffectiveOrbit && newOrbits[i].first + 0.5 * metMax < HIGH_ORBIT) {
			newOrbits[i].first += (0.5 * metMax);
			++metMax;
		}
		else if (newOrbits[i].first < lowestEffectiveOrbit) {
			newOrbits[i].first = highestEffectiveOrbit + (0.5 * metMax);
			++metMax;
		}
		else if (newOrbits[i].first > highestEffectiveOrbit) {
			newOrbits[i].first = lowestEffectiveOrbit - (0.5 * metMin);
			++metMin;
		}
		if (newOrbits[i].first > highestEffectiveOrbit || newOrbits[i].first < lowestEffectiveOrbit) {
			try {
				timeThreads.push_back({std::async(std::launch::async, &TSatellite::TimeCount,
												  &satellites[normal[i]], normal[i], dangerPhi, dangerTeta,
												  newOrbits[i].first), i});
				}
			catch (const std::future_error& e) {
				ProgramErrorExit(THREAD_ERROR);
			}
		}
	}
	for (; i < inDangerSize; ++i) {
		if (newOrbits[i].first < lowestEffectiveOrbit && newOrbits[i].first - 0.5 * metMin > LOW_ORBIT) {
			newOrbits[i].first -= (0.5 * metMin);
			++metMin;
		}
		else if (newOrbits[i].first > highestEffectiveOrbit && newOrbits[i].first + 0.5 * metMax < HIGH_ORBIT) {
			newOrbits[i].first += (0.5 * metMax);
			++metMax;
		}
		else if (newOrbits[i].first < lowestEffectiveOrbit) {
			newOrbits[i].first = highestEffectiveOrbit + (0.5 * metMax);
			++metMax;
		}
		else if (newOrbits[i].first > highestEffectiveOrbit) {
			newOrbits[i].first = lowestEffectiveOrbit - (0.5 * metMin);
			++metMin;
		}
		if (newOrbits[i].first > highestEffectiveOrbit || newOrbits[i].first < lowestEffectiveOrbit) {
			try {
				timeThreads.push_back({std::async(std::launch::async, &TSatellite::TimeCount,
												  &satellites[inDanger[i - border].first], inDanger[i - border].first,
												  dangerPhi, dangerTeta, newOrbits[i].first), i});
			}
			catch (const std::future_error& e) {
				ProgramErrorExit(THREAD_ERROR);
			}
		}
	}
	for (i = 0; i < timeThreads.size(); ++i) {
		newOrbits[timeThreads[i].second].second = timeThreads[i].first.get();
	}
	this->MasterRecover(newOrbits);
	return;
}

void TPetry::CommonEscape(int status) {
	this->P[this->O[3][0]].push_back({this->P[this->I[3][0].first][0].first, status});
	this->P[this->I[3][0].first].pop_back();
	return;
}

void TPetry::MasterEscape(std::vector<std::pair<double, int>>& newOrbits, std::vector<std::pair<int, double>>& inDanger,
						  bool fast) {
	std::vector<std::future<void>> escapeThreads(newOrbits.size());
	int status = 0;
	int i = 0;
	try {
		for (; i < newOrbits.size(); ++i) {
			status = (this->P[this->I[4][0].first][i].second == IN_DANGER ? DESTROYED : NORMAL);
			escapeThreads[i] = std::async(std::launch::async, &TSatellite::CommonEscape, &satellites[inDanger[i].first],
										  newOrbits[i].first, newOrbits[i].second, inDanger[i].first, inDanger[i].second,
										  fast, status);
		}
	}
	catch (const std::future_error& e) {
		ProgramErrorExit(THREAD_ERROR);
	}
	badOk = false;
	if (!fast) {
		std::cout << "Wait for satellites to change orbits.\n";
	}
	for (i = 0; i < newOrbits.size(); ++i) {
		escapeThreads[i].wait();
	}
	for (i = 0; i < this->P[this->I[4][0].first].size(); ++i) {
		this->P[this->O[4][0]].push_back({this->P[this->I[4][1].first][i].first,
										 (this->P[this->I[4][0].first][i].second == IN_DANGER ? DESTROYED : NORMAL)});
	}
	this->P[this->I[4][1].first].clear();
	this->P[this->I[4][0].first].clear();
	return;
}

void TPetry::Sort() {
	std::sort(this->P[0].begin(), this->P[0].end());
	return;
}

std::ostream& operator << (std::ostream& os, const TPetry& obj) {
	os << "Places:\n";
	for (int i = 0; i < obj.P.size(); ++i) {
		os << "\tP" << i + 1 << ": ";
		if (obj.P[i].size() == 0) {
			os << 0 << " satellites\n";
		}
		else {
			std::vector<int> countNormal, countInDanger, countDesroyed, countWillBeDesroyed;
			for (int j = 0; j < obj.P[i].size(); ++j) {
				switch (obj.P[i][j].second) {
					case NORMAL:
						countNormal.push_back(obj.P[i][j].first);
						break;
					case IN_DANGER:
						countInDanger.push_back(obj.P[i][j].first);
						break;
					case DESTROYED:
						countDesroyed.push_back(obj.P[i][j].first);
						break;
				}
			}
			bool firstMet = true;
			if (!countNormal.empty()) {
				firstMet = false;
				os << "normal - ";
				for (int j = 0; j < countNormal.size(); ++j) {
					os << countNormal[j] + 1 << CountSuffix(countNormal[j] + 1) << "  ";
				}
				os << "\n";
			}
			if (!countInDanger.empty()) {
				os << (!firstMet ? "\t    " : "") << "in danger - ";
				firstMet = false;
				for (int j = 0; j < countInDanger.size(); ++j) {
					os << countInDanger[j] + 1 << CountSuffix(countInDanger[j] + 1) << "  ";
				}
				os << "\n";
			}
			if (!countDesroyed.empty()) {
				os << (!firstMet ? "\t    " : "") << "destroyed - ";
				for (int j = 0; j < countDesroyed.size(); ++j) {
					os << countDesroyed[j] + 1 << CountSuffix(countDesroyed[j] + 1) << "  ";
				}
				os << "\n";
			}
		}
	}
	os << "Transitions:\n";
	for (int i = 0; i < obj.I.size(); ++i) {
		os << "\tI(t" << i + 1 << "): ";
		for (int j = 0; j < obj.I[i].size(); ++j) {
			os << "P" << obj.I[i][j].first + 1 << " - " << (obj.I[i][j].second == ANY ? "Any" :
			   (obj.I[i][j].second == NORMAL ? "Normal" : (obj.I[i][j].second == IN_DANGER ? "In danger" :
			   (obj.I[i][j].second == DESTROYED ? "Destroyed" : "Will be destroyed"))))
			   << (j + 1 < obj.I[i].size() ? ", " : "");
		}
		os << "\n";
	}
	for (int i = 0; i < obj.O.size(); ++i) {
		os << "\tO(t" << i + 1 << "): ";
		for (int j = 0; j < obj.O[i].size(); ++j) {
			os << "P" << obj.O[i][j] + 1 << (j + 1 < obj.O[i].size() ? ", " : "");
		}
		os << "\n";
	}
	return os;
}

TPetry::~TPetry() {
	this->P.clear();
	this->I.clear();
	this->O.clear();
}

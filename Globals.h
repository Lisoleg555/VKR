#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <random>
#include <ctime>
#include <system_error>

#define _USE_MATH_DEFINES

class TTimer {
public:
	TTimer();
	int Seed();
	void Start();
	void Finish();
	double GetTime();
	~TTimer();
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> finish;
};

class TDanger {
public:
	TDanger();
	void CalculateVelocity();
	void Move();
	~TDanger();
	
	bool type;
	double radius;
	double phi;
	double teta;
	double size;
private:
	double velocity;
};

class TPetry {
public:
	TPetry();
	void BuildUsual();
	void BuildMaster();
	int GetStatus();
	void ChangeStatus(int);
	void DangerStatus(int);
	void CommonGetDanger(int, int);
	void MasterOrbitCalculation(int, int);
	void CommonSafe(int);
	void CommonReadyChange(int);
	void MasterSafe();
	void MasterStartEscape();
	void ClearD();
	int ISize();
	void MasterRecover(std::vector<std::pair<double, int>>&);
	void MasterGetNewOrbits(std::vector<std::pair<double, int>>&, std::vector<std::pair<int, double>>&);
	void CommonEscape(int);
	void MasterEscape(std::vector<std::pair<double, int>>&, std::vector<std::pair<int, double>>&, bool);
	void Sort();
	friend std::ostream& operator << (std::ostream&, const TPetry&);
	~TPetry();
private:
	std::vector<std::vector<std::pair<int, int>>> P;
	std::vector<std::vector<std::pair<int, int>>> I;
	std::vector<std::vector<int>> O;
};

class TSatellite {
public:
	TSatellite();
	void CalculateVelocity();
	void BuildUsual();
	void BuildMaster();
	int GetStatus();
	void Move();
	void ChangeStatus(int);
	void RelocateMaster(TPetry*, int);
	void DangerStatus(int);
	void CommonGetDanger(int, int);
	void MasterOrbitCalculation(int, int);
	void CommonSafeReadyChange(int);
	void Common();
	void PetryParallel();
	double CountNewOrbit(bool, int);
	int TimeCount(int, double, double, double);
	void FastMove(int);
	void CommonEscape(double, int, int, double, bool, int);
	void EvadeResult(int, int, bool);
	void Evade(int, int);
	friend std::ostream& operator << (std::ostream&, const TSatellite&);
	~TSatellite();

	bool type;
	double radius;
	double phi;
	double teta;
	double fuel;
private:
	double velocity;
	TPetry* usual;
	TPetry* master;
};

typedef std::unique_ptr<std::mutex> TMyMutex;

const double LOW_ORBIT = 180.0;
const double HIGH_ORBIT = 350000.0;
const double TWO_PI = 2 * M_PI;
const double HALF_PI = M_PI / 2.0;
const double EARTH_RADIUS = 6371.1;
const double MOON_RADIUS = 1738.14;
const double GM = 3.986025446;
const double EPS_SIZE = 0.01;
const double KM = 1000.0;
const double VELOCITY_CONSTANT = 10000.0;
const double FUEL_PER_KM = 0.1;
const double EPS_SAFE = 1000.0 * EPS_SIZE;
const double MAX_RANDOM_FUEL = 1000.0;

const int SLEEP = 500;
const int DANGER_SLEEP = 30;
const int DELTA_COUNT = 10000;

extern double lowestEffectiveOrbit;
extern double highestEffectiveOrbit;

extern int masterSatellite;
extern int deltaTime;
extern int satelCount;
extern int satelInWork;

extern bool ok;
extern bool badOk;

extern std::vector<TMyMutex> mainMutex;
extern std::vector<TSatellite> satellites;

extern TDanger* danger;

extern std::mutex dangerMutex;

extern TTimer satelClock;

void ProgramErrorExit(int);
bool IsNumber(std::string, double*);
template<typename Type> Type ReadNumber();
void CheckSame(int);
void SameRadius(double);
void DifferentRadius(double);
void SatellitesData();
std::string CountSuffix(int);
void ThreadMove(const int);
bool Clash(int);
void StartBreak();
int CountNotDamaged();
std::vector<int> FasterSearch();
void MoveDanger(std::vector<int>);
void CommonDanger(int, int, int);
void SpawnDanger();
void PrintSatellitesInfo();

enum PetryStatus {
	NORMAL,
	IN_DANGER,
	DESTROYED,
	ANY
};

enum Errors {
	WRONG_COUNT_SATELLITES,
	WRONG_INPUT,
	WRONG_ORBIT,
	WRONG_FUEL,
	WRONG_MASTER,
	WRONG_SAME_COORDINATES,
	WRONG_TIME,
	WRONG_SIZE,
	MEMORY_ERROR,
	THREAD_ERROR,
	MUTEX_ERROR
};

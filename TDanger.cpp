#include "Globals.h"

TDanger::TDanger() {
	std::string input = "";
	std::cout << "If you want to use random number generator, input \'y\'. Input something else for hand input: ";
	std::cin >> input;
	if (input == "y") {
		std::mt19937 random_gen(satelClock.Seed());
		std::uniform_real_distribution<double> heightArea(LOW_ORBIT, HIGH_ORBIT);
		std::uniform_real_distribution<double> phiArea(0.0, TWO_PI);
		std::uniform_real_distribution<double> tetaArea(0.0, M_PI);
		std::uniform_real_distribution<double> sizeArea(EPS_SIZE, MOON_RADIUS - EPS_SIZE);
		this->radius = heightArea(random_gen);
		int tmp = heightArea(random_gen);
		this->type = (tmp % 2 == 0);
		this->phi = phiArea(random_gen);
		this->teta = (this->type ? HALF_PI : tetaArea(random_gen));
		this->size = sizeArea(random_gen);
		this->CalculateVelocity();
		return;
	}
	std::cout << "Enter height for danger object: ";
	this->radius = ReadNumber<double>();
	if (this->radius < 0) {
		ProgramErrorExit(WRONG_ORBIT);
	}
	else if (this->radius < LOW_ORBIT || this->radius > HIGH_ORBIT) {
		return;
	}
	std::cout << "Enter orbit type for danger object: meridian(m) or equator(else): ";
	std::cin >> input;
	if (!std::cin) {
		ProgramErrorExit(WRONG_INPUT);
	}
	this->type = (input != "m");
	std::cout << "Enter phi of danger object: ";
	this->phi = std::fmod(ReadNumber<double>(), TWO_PI);
	if (!this->type) {
		std::cout << "Enter teta of danger object: ";
	}
	this->teta = this->type ? HALF_PI : std::fmod(ReadNumber<double>(), M_PI);
	std::cout << "Enter size of danger object. It can not be bigger than Moon: ";
	this->size = ReadNumber<double>();
	if (this->size <= 0.0 || this->size >= MOON_RADIUS) {
		ProgramErrorExit(WRONG_SIZE);
	}
	this->CalculateVelocity();
}

void TDanger::CalculateVelocity() {
	double r = (this->radius + EARTH_RADIUS) * KM;
	this->velocity = sqrt(GM / r) / r * VELOCITY_CONSTANT;
	return;
}

void TDanger::Move() {
	if (this->type) {
		this->phi = this->phi - this->velocity * deltaTime;
		this->phi += (this->phi < 0.0 ? TWO_PI : 0.0);
	}
	else {
		this->teta -= (this->phi > M_PI ? -1 : 1) * this->velocity * deltaTime;
		if (this->teta > M_PI) {
			this->teta = M_PI - std::fmod(this->teta, M_PI);
			this->phi -= M_PI;
		}
		else if (this->teta < 0) {
			this->teta *= -1;
			this->phi += M_PI;
		}
	}
	return;
}

TDanger::~TDanger() {}

// Ricardas Navickas 2020
#include "pid.h"

PIDController::PIDController(double Kp, double Ki, double Kd) {
	this->Kp = Kp;
	this->Ki = Ki;
	this->Kd = Kd;

	reset();
}

PIDController::~PIDController() {
}

void PIDController::reset() {
	prev_error = 0.0;
	error = 0.0;
	error_integral = 0.0;
	error_derivative = 0.0;
}

void PIDController::update(double dt, double err) {
	error = err;
	error_integral = error_integral + error * dt;
	error_derivative = (error - prev_error) / dt;

	prev_error = error;
}

double PIDController::output() {
	return Kp * error + Ki * error_integral + Kd * error_derivative;
}


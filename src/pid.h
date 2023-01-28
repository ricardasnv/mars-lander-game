// Ricardas Navickas 2020
#ifndef PID_H
#define PID_H

class PIDController {
public:
	PIDController(double Kp, double Ki, double Kd);
	~PIDController();

	void reset();

	void update(double dt, double err);
	double output();

	double Kp, Ki, Kd;

private:
	double prev_error;
	double error, error_integral, error_derivative;
};

#endif

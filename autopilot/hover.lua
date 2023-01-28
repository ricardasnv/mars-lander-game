-- Ricardas Navickas 2020
-- Hover script for scenario 1
target = 0.5 -- Your value here (hover altitude in km)

-- PID controller constants
Kp = 10.0
Ki = 0.0
Kd = 50.0

-- PID controller variables
prev_err = 0.0
err = 0.0
err_int = 0.0
err_deriv = 0.0

prev_time = TIME()

function STEP()
	if (TIME() < prev_time) then
		-- if scenario changed
		pid_reset()
	end

	-- Always keep parachute deployed
	DEPLOY_PARACHUTE()

	-- Update PID controller error and advance time
	dt = TIME() - prev_time
	err = target - LANDER_ALT()
	pid_update(dt)

	-- Always point radial and set main engine throttle to whatever the PID controller says
	HOLD_RADIAL()
	throttle = LANDER_WEIGHT() / LANDER_MAX_THRUST() + pid_output()
	SET_MAIN_ENGINE_THROTTLE(throttle)

	prev_time = TIME()
end

function pid_reset()
	prev_err = 0.0
	err = 0.0
	err_int = 0.0
	err_deriv = 0.0
end

function pid_update(delta_time)
	err_int = err_int + err * delta_time
	err_deriv = (err - prev_err) / delta_time
	prev_err = err
end

function pid_output()
	return Kp * err + Ki * err_int + Kd * err_deriv
end


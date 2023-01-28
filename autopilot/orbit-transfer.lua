-- Ricardas Navickas 2020
-- Set chosen periapsis/apoapsis
-- Works better with greater altitudes
target_periapsis = 600.0 -- Your value here
target_apoapsis = 2000.0 -- Your value here
tolerance = 5.0 -- Small tolerances can cause overshoot

state = 0
prev_state = state - 1
finished = false

function STEP()
	if (finished) then
		return
	end

	if (state == 0) then
		-- First coast to apoapsis
		first_run_message("Coasting to apoapsis")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(math.abs(LANDER_ALT() - APOAPSIS_ALT()) < 0.01)
	elseif (state == 1) then
		-- Point prograde/retrograde depending on periapsis alt.
		first_run_message("Correcting attitude for 10 seconds...")
		if (prev_state ~= state) then state_start_time = TIME() end

		if (PERIAPSIS_ALT() > target_periapsis + tolerance) then
			-- If periapsis too high, point retrograde
			HOLD_RETROGRADE()
		elseif (PERIAPSIS_ALT() < target_periapsis - tolerance) then
			-- If periapsis too low, point prograde
			HOLD_PROGRADE()
		end

		advance_state_if(TIME() - state_start_time > 10.0)
	elseif (state == 2) then
		-- Burn until periapsis altitude is within tolerance
		first_run_message("Correcting periapsis altitude...")
		MAINTAIN_ATTITUDE()
		SET_MAIN_ENGINE_THROTTLE(1.0)
		advance_state_if(math.abs(PERIAPSIS_ALT() - target_periapsis) < tolerance)
	elseif (state == 3) then
		-- Coast to periapsis
		first_run_message("Coasting to periapsis...")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(math.abs(LANDER_ALT() - PERIAPSIS_ALT()) < 0.01)
	elseif (state == 4) then
		-- Point prograde/retrograde depending on apoapsis alt.
		first_run_message("Correcting attitude for 10 seconds...")
		if (prev_state ~= state) then state_start_time = TIME() end

		if (APOAPSIS_ALT() > target_apoapsis + tolerance) then
			-- If apoapsis too high, point retrograde
			HOLD_RETROGRADE()
		elseif (APOAPSIS_ALT() < target_apoapsis - tolerance) then
			-- If apoapsis too low, point prograde
			HOLD_PROGRADE()
		end

		advance_state_if(TIME() - state_start_time > 10.0)
	elseif (state == 5) then
		-- Burn until apoapsis altitude is within tolerance
		first_run_message("Correcting apoapsis altitude...")
		MAINTAIN_ATTITUDE()
		SET_MAIN_ENGINE_THROTTLE(1.0)
		advance_state_if(math.abs(APOAPSIS_ALT() - target_apoapsis) < tolerance)
	else
		first_run_message("Program finished.")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		finished = true
	end
end

function first_run_message(msg)
	if (prev_state ~= state) then
		INFO(msg)
	end
end

function advance_state_if(condition)
	prev_state = state
	if (condition == true) then
		state = state + 1
	end
end


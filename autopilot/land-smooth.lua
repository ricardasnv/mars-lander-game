-- Ricardas Navickas 2020
-- General autopilot for soft landing in any scenario
-- Performs a smooth final burn (throttle is continuous over time) at the expense of a significant amount of fuel
state = 0
prev_state = state - 1
finished = false

function STEP()
	if (finished == true) then
		return
	end

	if (state == 0) then
		-- If already deorbited, jump to state 3
		if (PERIAPSIS_ALT() < 0.0) then
			state = 3
			return
		end

		first_run_message("Coasting to apoapsis...")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(APOAPSIS_ALT() - LANDER_ALT() < 0.01)
	elseif (state == 1) then
		first_run_message("Waiting 10 seconds to point retrograde...")
		if (prev_state ~= state) then state_start_time = TIME() end
		HOLD_RETROGRADE()
		advance_state_if(TIME() - state_start_time > 10.0)
	elseif (state == 2) then
		first_run_message("Performing deorbit burn...")
		HOLD_RETROGRADE()
		SET_MAIN_ENGINE_THROTTLE(1.0)
		advance_state_if(PERIAPSIS_ALT() < 0.0)
	elseif (state == 3) then
		first_run_message("Waiting for altitude to drop below 10km..")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(LANDER_ALT() < 10.0)
	elseif (state == 4) then
		first_run_message("Pointing retrograde in preparation for entry burn...")
		HOLD_SURFACE_RETROGRADE()
		advance_state_if(LANDER_ALT() < 9.0)
	elseif (state == 5) then
		first_run_message("Reducing surface velocity to 500 m/s...")
		HOLD_SURFACE_RETROGRADE()
		SET_MAIN_ENGINE_THROTTLE(1.0)
		advance_state_if(LANDER_SURFACE_VELOCITY() < 0.5)
	elseif (state == 6) then
		first_run_message("Deploying parachute.")
		DEPLOY_PARACHUTE()
		HOLD_SURFACE_RETROGRADE()
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(LANDER_ALT() < 3.0)
	elseif (state == 7) then
		first_run_message("Reducing groundspeed to 1.0 m/s...")
		MAINTAIN_ATTITUDE()
		SET_MAIN_ENGINE_THROTTLE(1.0)
		advance_state_if(LANDER_GROUND_SPEED() < 0.001 or LANDER_GROUND_SPEED() > prev_ground_speed + 0.001)
	elseif (state == 8) then
		first_run_message("Pointing retrograde in preparation for final burn...")
		HOLD_SURFACE_RETROGRADE()
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(LANDER_ALT() <= 1.0)
	elseif (state == 9) then
		first_run_message("Reducing descent rate...")
		HOLD_SURFACE_RETROGRADE()
		MAINTAIN_SURFACE_VELOCITY(0.0003 + 0.1 * LANDER_ALT() / 1.0)
		advance_state_if(LANDER_ALT() < 0.00005)
	else
		first_run_message("Program finished.")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		finished = true
	end

	prev_ground_speed = LANDER_GROUND_SPEED()
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


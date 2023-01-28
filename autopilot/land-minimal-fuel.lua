-- Ricardas Navickas 2020
-- Autopilot that lands with minimal fuel expenditure
--[[ Strategy:
	1. Continuously calculate "suicide burn" height (approx. minimal height at which soft landing is possible).
	2. If altitude > "suicide burn" altitude, set throttle to 1.0, else set throttle to 0.0.
	NOTE: Suicide burn height is found from the assumption that the lander thrust must do an amount of work equal to current potential+kinetic energy.
	      This isn't exactly correct because drag does a significant amount of work as well.
		  Because of this inaccuracy the throttle will "flicker" from 0.0 to 1.0 and more fuel will be used than the theoretical minimum. --]]
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
		first_run_message("Pointing retrograde in preparation for final burn...")
		HOLD_SURFACE_RETROGRADE()
		SET_MAIN_ENGINE_THROTTLE(0.0)
		advance_state_if(LANDER_ALT() <= suicide_burn_height())
	elseif (state == 8) then
		first_run_message("Reducing descent rate...")
		HOLD_SURFACE_RETROGRADE()
		
		if (LANDER_ALT() <= suicide_burn_height()) then
			SET_MAIN_ENGINE_THROTTLE(1.0)
		else
			SET_MAIN_ENGINE_THROTTLE(0.0)
		end

		advance_state_if(LANDER_ALT() < 0.00005)
	else
		first_run_message("Program finished.")
		SET_MAIN_ENGINE_THROTTLE(0.0)
		finished = true
	end
end

function suicide_burn_height()
	if (LANDER_DESCENT_RATE() < 0.0) then return 10000.0 end
	return LANDER_MASS() * LANDER_SURFACE_VELOCITY() * LANDER_SURFACE_VELOCITY() / (2 * (LANDER_MAX_THRUST() - LANDER_WEIGHT()))
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


-- Ricardas Navickas 2020
first_run = true

function STEP()
	if (first_run == true) then
		INFO("Aligning lander's -Y axis with velocity relative to surface...")
		first_run = false
	end

	HOLD_SURFACE_RETROGRADE()
end

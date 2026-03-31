//-------------------------------------------------------------------------------------------------
// VirtualMachine - A LegacyMachine implementation using Tilengine as the frontend.
//
// Copyright(C) 2024-2025 Steven Leffew
// All rights reserved
//-------------------------------------------------------------------------------------------------
// This program is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <https://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "JoypadManager.h"

//-------------------------------------------------------------------------------------------------
// JoypadAutoConfig Function
//-------------------------------------------------------------------------------------------------

// Auto configuration callback function for LMC_SetAutoConfigureJoypadCallback().
void JoypadAutoConfig(LMC_Player player, char* name, uint32_t vendor, uint32_t product)
{
	JoypadConfiguration* joypadConfig = JoypadManager::GetJoypadManager()->GetJoypadConfiguration(vendor, product, string(name));
	if (joypadConfig)
	{
		if (!joypadConfig->input_up.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_up.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_UP, joypadConfig->input_up.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_up.index, LMC_JOYPAD_INPUT_UP, (LMC_HatDirection)joypadConfig->input_up.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_up.index, LMC_JOYPAD_INPUT_UP, (LMC_AxisDirection)joypadConfig->input_up.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_down.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_down.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_DOWN, joypadConfig->input_down.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_down.index, LMC_JOYPAD_INPUT_DOWN, (LMC_HatDirection)joypadConfig->input_down.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_down.index, LMC_JOYPAD_INPUT_DOWN, (LMC_AxisDirection)joypadConfig->input_down.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_left.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_left.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_LEFT, joypadConfig->input_left.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_left.index, LMC_JOYPAD_INPUT_LEFT, (LMC_HatDirection)joypadConfig->input_left.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_left.index, LMC_JOYPAD_INPUT_LEFT, (LMC_AxisDirection)joypadConfig->input_left.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_right.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_right.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_RIGHT, joypadConfig->input_right.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_right.index, LMC_JOYPAD_INPUT_RIGHT, (LMC_HatDirection)joypadConfig->input_right.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_right.index, LMC_JOYPAD_INPUT_RIGHT, (LMC_AxisDirection)joypadConfig->input_right.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_a.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_a.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_A, joypadConfig->input_a.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_a.index, LMC_JOYPAD_INPUT_A, (LMC_HatDirection)joypadConfig->input_a.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_a.index, LMC_JOYPAD_INPUT_A, (LMC_AxisDirection)joypadConfig->input_a.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_b.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_b.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_B, joypadConfig->input_b.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_b.index, LMC_JOYPAD_INPUT_B, (LMC_HatDirection)joypadConfig->input_b.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_b.index, LMC_JOYPAD_INPUT_B, (LMC_AxisDirection)joypadConfig->input_b.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_x.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_x.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_X, joypadConfig->input_x.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_x.index, LMC_JOYPAD_INPUT_X, (LMC_HatDirection)joypadConfig->input_x.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_x.index, LMC_JOYPAD_INPUT_X, (LMC_AxisDirection)joypadConfig->input_x.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_y.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_y.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_Y, joypadConfig->input_y.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_y.index, LMC_JOYPAD_INPUT_Y, (LMC_HatDirection)joypadConfig->input_y.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_y.index, LMC_JOYPAD_INPUT_Y, (LMC_AxisDirection)joypadConfig->input_y.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_start.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_start.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_START, joypadConfig->input_start.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_start.index, LMC_JOYPAD_INPUT_START, (LMC_HatDirection)joypadConfig->input_start.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_start.index, LMC_JOYPAD_INPUT_START, (LMC_AxisDirection)joypadConfig->input_start.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_select.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_select.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_SELECT, joypadConfig->input_select.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_select.index, LMC_JOYPAD_INPUT_SELECT, (LMC_HatDirection)joypadConfig->input_select.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_select.index, LMC_JOYPAD_INPUT_SELECT, (LMC_AxisDirection)joypadConfig->input_select.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L, joypadConfig->input_l.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l.index, LMC_JOYPAD_INPUT_L, (LMC_HatDirection)joypadConfig->input_l.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l.index, LMC_JOYPAD_INPUT_L, (LMC_AxisDirection)joypadConfig->input_l.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R, joypadConfig->input_r.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r.index, LMC_JOYPAD_INPUT_R, (LMC_HatDirection)joypadConfig->input_r.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r.index, LMC_JOYPAD_INPUT_R, (LMC_AxisDirection)joypadConfig->input_r.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l2.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l2.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L2, joypadConfig->input_l2.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l2.index, LMC_JOYPAD_INPUT_L2, (LMC_HatDirection)joypadConfig->input_l2.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l2.index, LMC_JOYPAD_INPUT_L2, (LMC_AxisDirection)joypadConfig->input_l2.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r2.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r2.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R2, joypadConfig->input_r2.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r2.index, LMC_JOYPAD_INPUT_R2, (LMC_HatDirection)joypadConfig->input_r2.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r2.index, LMC_JOYPAD_INPUT_R2, (LMC_AxisDirection)joypadConfig->input_r2.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l3.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l3.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L3, joypadConfig->input_l3.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l3.index, LMC_JOYPAD_INPUT_L3, (LMC_HatDirection)joypadConfig->input_l3.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l3.index, LMC_JOYPAD_INPUT_L3, (LMC_AxisDirection)joypadConfig->input_l3.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r3.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r3.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R3, joypadConfig->input_r3.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r3.index, LMC_JOYPAD_INPUT_R3, (LMC_HatDirection)joypadConfig->input_r3.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r3.index, LMC_JOYPAD_INPUT_R3, (LMC_AxisDirection)joypadConfig->input_r3.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_home.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_home.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_HOME, joypadConfig->input_home.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_home.index, LMC_JOYPAD_INPUT_HOME, (LMC_HatDirection)joypadConfig->input_home.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_home.index, LMC_JOYPAD_INPUT_HOME, (LMC_AxisDirection)joypadConfig->input_home.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l_analog_up.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l_analog_up.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L_ANALOG_UP, joypadConfig->input_l_analog_up.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l_analog_up.index, LMC_JOYPAD_INPUT_L_ANALOG_UP, (LMC_HatDirection)joypadConfig->input_l_analog_up.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l_analog_up.index, LMC_JOYPAD_INPUT_L_ANALOG_UP, (LMC_AxisDirection)joypadConfig->input_l_analog_up.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l_analog_down.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l_analog_down.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L_ANALOG_DOWN, joypadConfig->input_l_analog_down.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l_analog_down.index, LMC_JOYPAD_INPUT_L_ANALOG_DOWN, (LMC_HatDirection)joypadConfig->input_l_analog_down.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l_analog_down.index, LMC_JOYPAD_INPUT_L_ANALOG_DOWN, (LMC_AxisDirection)joypadConfig->input_l_analog_down.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l_analog_left.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l_analog_left.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L_ANALOG_LEFT, joypadConfig->input_l_analog_left.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l_analog_left.index, LMC_JOYPAD_INPUT_L_ANALOG_LEFT, (LMC_HatDirection)joypadConfig->input_l_analog_left.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l_analog_left.index, LMC_JOYPAD_INPUT_L_ANALOG_LEFT, (LMC_AxisDirection)joypadConfig->input_l_analog_left.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_l_analog_right.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_l_analog_right.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_L_ANALOG_RIGHT, joypadConfig->input_l_analog_right.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_l_analog_right.index, LMC_JOYPAD_INPUT_L_ANALOG_RIGHT, (LMC_HatDirection)joypadConfig->input_l_analog_right.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_l_analog_right.index, LMC_JOYPAD_INPUT_L_ANALOG_RIGHT, (LMC_AxisDirection)joypadConfig->input_l_analog_right.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r_analog_up.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r_analog_up.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R_ANALOG_UP, joypadConfig->input_r_analog_up.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r_analog_up.index, LMC_JOYPAD_INPUT_R_ANALOG_UP, (LMC_HatDirection)joypadConfig->input_r_analog_up.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r_analog_up.index, LMC_JOYPAD_INPUT_R_ANALOG_UP, (LMC_AxisDirection)joypadConfig->input_r_analog_up.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r_analog_down.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r_analog_down.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R_ANALOG_DOWN, joypadConfig->input_r_analog_down.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r_analog_down.index, LMC_JOYPAD_INPUT_R_ANALOG_DOWN, (LMC_HatDirection)joypadConfig->input_r_analog_down.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r_analog_down.index, LMC_JOYPAD_INPUT_R_ANALOG_DOWN, (LMC_AxisDirection)joypadConfig->input_r_analog_down.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r_analog_left.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r_analog_left.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R_ANALOG_LEFT, joypadConfig->input_r_analog_left.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r_analog_left.index, LMC_JOYPAD_INPUT_R_ANALOG_LEFT, (LMC_HatDirection)joypadConfig->input_r_analog_left.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r_analog_left.index, LMC_JOYPAD_INPUT_R_ANALOG_LEFT, (LMC_AxisDirection)joypadConfig->input_r_analog_left.input);
				break;
			default:
				break;
			}
		}
		if (!joypadConfig->input_r_analog_right.type == INPUT_TYPE_NONE)
		{
			switch (joypadConfig->input_r_analog_right.type)
			{
			case INPUT_TYPE_BUTTON:
				LMC_DefineJoypadButtonInput(player, LMC_JOYPAD_INPUT_R_ANALOG_RIGHT, joypadConfig->input_r_analog_right.input);
				break;
			case INPUT_TYPE_HAT:
				LMC_DefineJoypadHatInput(player, joypadConfig->input_r_analog_right.index, LMC_JOYPAD_INPUT_R_ANALOG_RIGHT, (LMC_HatDirection)joypadConfig->input_r_analog_right.input);
				break;
			case INPUT_TYPE_AXIS:
				LMC_DefineJoypadAxisInput(player, joypadConfig->input_r_analog_right.index, LMC_JOYPAD_INPUT_R_ANALOG_RIGHT, (LMC_AxisDirection)joypadConfig->input_r_analog_right.input);
				break;
			default:
				break;
			}
		}
	}
}
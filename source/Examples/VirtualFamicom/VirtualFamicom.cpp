//-------------------------------------------------------------------------------------------------
// VirtualFamicom.
//
// Copyright(C) 2024 Steven Leffew
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
#include <IMachine.h>
#include <VirtualMachine.h>
#include <iostream>

#include "FamicomMachine.h"

//-------------------------------------------------------------------------------------------------
// Main function
//-------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Get the virtual machine engine.
    VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();

    // Initialize the virtual machine interface.
    IMachine* pMachine = new FamicomMachine;

    // Assign the created system interface to the engine.
    pVirtualMachine->m_machine = pMachine;

    // Initialize or quit if an error occured.
    if (!pMachine->Initialize(CWF_S4))
    {
        return EXIT_FAILURE;
    }

    // Run the virtual famicom machine.
    pVirtualMachine->Run();

    return EXIT_SUCCESS;
}
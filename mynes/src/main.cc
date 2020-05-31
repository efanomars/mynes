/*
 * Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   main.cc
 */

#include "binconfig.h"

#include "setup.h"

#include <stmm-games-gtk/mainwindow.h>

#include <gtkmm.h>

#include <iostream>
#include <cassert>
#include <string>

#include <stdlib.h>


namespace stmg
{

void printVersion(const std::string& sAppVarsion) noexcept
{
	std::cout << sAppVarsion << '\n';
}
void printUsage(const std::string& sAppName) noexcept
{
	std::cout << "Usage: " << sAppName << " [OPTION]" << '\n';
	std::cout << "Mynes games." << '\n';
	std::cout << "Option:" << '\n';
	std::cout << "  -h --help        Prints this message." << '\n';
	std::cout << "  -v --version     Prints version." << '\n';
	std::cout << "     --touch       Start in touch mode (overrides keyboard mode)." << '\n';
	std::cout << "     --keys        Start in keyboard mode." << '\n';
	std::cout << "  -s --no-sound    Disables sound;" << '\n';
	std::cout << "                   more efficient than setting volume(s) to 0." << '\n';
	std::cout << "  -f --fullscreen  Start in fullscreen." << '\n';
	std::cout << "  -t --testing     Enable testing of custom games." << '\n';
}

void evalNoArg(int& nArgC, char**& aArgV, const std::string& sOption1, const std::string& sOption2, bool& bVar) noexcept
{
	if (aArgV[1] == nullptr) {
		return;
	}
	const bool bIsOption1 = (sOption1 == std::string(aArgV[1]));
	if (bIsOption1 || ((!sOption2.empty()) && (sOption2 == std::string(aArgV[1])))) {
		bVar = true;
		--nArgC;
		++aArgV;
	}
}
int mynesMain(int nArgC, char** aArgV) noexcept
{
	const std::string sMynes = "mynes";
	const std::string& sAppVersion = binconfig::getExecutableVersion();
	bool bNoSound = false;
	bool bTestMode = false;
	bool bFullscreen = false;
	bool bTouchMode = false;
	bool bKeysMode = false;

	MainWindowData oMainWindowData;

	bool bHelp = false;
	bool bVersion = false;

	char* p0ArgVZeroSave = ((nArgC >= 1) ? aArgV[0] : nullptr);
	while (nArgC >= 2) {
		auto nOldArgC = nArgC;
		evalNoArg(nArgC, aArgV, "--help", "-h", bHelp);
		if (bHelp) {
			printUsage(sMynes);
			return EXIT_SUCCESS; //---------------------------------------------
		}
		evalNoArg(nArgC, aArgV, "--version", "-v", bVersion);
		if (bVersion) {
			printVersion(sAppVersion);
			return EXIT_SUCCESS; //---------------------------------------------
		}
		evalNoArg(nArgC, aArgV, "--no-sound", "-s", bNoSound);
		evalNoArg(nArgC, aArgV, "--testing", "-t", bTestMode);
		evalNoArg(nArgC, aArgV, "--fullscreen", "-f", bFullscreen);
		evalNoArg(nArgC, aArgV, "--touch", "", bTouchMode);
		evalNoArg(nArgC, aArgV, "--keys", "", bKeysMode);
		//
		if (nOldArgC == nArgC) {
			std::cerr << "Unknown argument: " << ((aArgV[1] == nullptr) ? "(null)" : std::string(aArgV[1])) << '\n';
			return EXIT_FAILURE; //---------------------------------------------
		}
		aArgV[0] = p0ArgVZeroSave;
	}

	Glib::RefPtr<Gtk::Application> refApp =
			Gtk::Application::create("com.efanomars." + sMynes);

	std::string sErr = mynesSetup(oMainWindowData, sMynes, sAppVersion
								, bNoSound, bTestMode, bFullscreen, bTouchMode, bKeysMode);
	if (! sErr.empty()) {
		std::cerr << sErr << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	}

	auto oPairMainWin = createMainWindow(std::move(oMainWindowData));
	Glib::RefPtr<Gtk::Window>& refMainWin = oPairMainWin.first;
	if (!refMainWin) {
		std::cout << "Error: Couldn't create window: " << oPairMainWin.second << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	}
	return refApp->run(*(refMainWin.operator->()));
}

} //namespace stmg

int main(int argc, char** argv)
{
	return stmg::mynesMain(argc, argv);
}


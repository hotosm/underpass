//
// Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
//
// This file is part of Underpass.
//
//     Underpass is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Underpass is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.
//

#include <iostream>

int
main(int argc, char *argv[])
{
    std::cout << "SRCDIR = " << SRCDIR << std::endl;
    std::cout << "DATADIR = " << DATADIR << std::endl;
    std::cout << "PKGLIBDIR = " << PKGLIBDIR << std::endl;

    std::string changesetFile(DATADIR);
    changesetFile += "/testsuite/testdata/areafilter-test.osc";
    std::cout << "changesetFile = " << changesetFile << std::endl;

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

//
// Copyright (c) 2020, Humanitarian OpenStreetMap Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <dejagnu.h>
#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include <libxml++/libxml++.h>

#include "hottm.hh"
#include "osmstats/osmstats.hh"
#include "osmstats/changeset.hh"
#include "osmstats/changefile.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace changeset;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestCS : public changeset::ChangeSetFile
{
    bool testFile(const std::string &filespec) {
        // std::string basedir="DATADIR";
    };
    bool testMem(const std::string &data);
};

class TestCF : public changefile::ChangeFile
{
    bool testFile(void) {
        // Get input and output file names from command line.
        std::string input_file_name{""};
        std::string output_file_name{""};

        try {
            // Initialize Reader
            osmium::io::Reader reader{input_file_name};

            // Get header from input file and change the "generator" setting to
            // ourselves.
            osmium::io::Header header = reader.header();
            header.set("generator", "osmium_change_tags");

            // Initialize Writer using the header from above and tell it that it
            // is allowed to overwrite a possibly existing file.
            osmium::io::Writer writer{output_file_name, header, osmium::io::overwrite::allow};

            // Read in buffers with OSM objects until there are no more.
            while (osmium::memory::Buffer input_buffer = reader.read()) {
                // Create an empty buffer with the same size as the input buffer.
                // We'll copy the changed data into output buffer, the changes
                // are small, so the output buffer needs to be about the same size.
                // In case it has to be bigger, we allow it to grow automatically
                // by adding the auto_grow::yes parameter.
                osmium::memory::Buffer output_buffer{input_buffer.committed(), osmium::memory::Buffer::auto_grow::yes};

                // Construct a handler as defined above and feed the input buffer
                // to it.
                changefile::ChangeFile handler{output_buffer};
                osmium::apply(input_buffer, handler);

                // Write out the contents of the output buffer.
                writer(std::move(output_buffer));
            }

            // Explicitly close the writer and reader. Will throw an exception if
            // there is a problem. If you wait for the destructor to close the writer
            // and reader, you will not notice the problem, because destructors must
            // not throw.
            writer.close();
            reader.close();
        } catch (const std::exception& e) {
            // All exceptions used by the Osmium library derive from std::exception.
            std::cerr << e.what() << '\n';
            std::exit(1);
        }
    }
};

class TestStateFile : public changeset::StateFile
{
public:
    TestStateFile(const std::string &file, bool memory) : changeset::StateFile(file, memory) {
    };
    // Accessors for the private data
    ptime getTimestamp(void) { return timestamp; };
    long getSequence(void) { return sequence; };
};

int
main(int argc, char* argv[])
{
    std::string basedir=DATADIR;

    // Read the changeset state file
    std::string buf = "---\nlast_run: 2020-10-08 22:30:01.737719000 +00:00\nsequence: 4139993\n";
    TestStateFile statefile(basedir + "/993.state.txt", false);
    if (statefile.getSequence() == 4139992) {
        runtest.pass("Changeset state file from disk");
    } else {
        runtest.fail("Changeset state file from disk");
    }

    TestStateFile mem(buf, true);
    if (mem.getSequence() == 4139993) {
        runtest.pass("Changeset state file from memory");
    } else {
        runtest.fail("Changeset state file from memory");
    }

    // Read a change file state file
    TestStateFile minstate(basedir + "/996.state.txt", false);
    if (minstate.getSequence() == 4230996) {
        runtest.pass("Change file state file from disk");
    } else {
        runtest.fail("Change file state file from disk");
    }

    osmstats::QueryOSMStats os;
    TestCS tests;

    os.startTimer();
    tests.importChanges(basedir + "/foo.osm");
    std::cout << "Operation took " << os.endTimer() << " milliseconds" << std::endl;

    os.startTimer();
    tests.readChanges(basedir + "/993.osm.gz", false);
    std::cout << "Operation took " << os.endTimer() << " milliseconds" << std::endl;

    if (os.connect("mystats")) {
        runtest.pass("QueryOsmStats::connect()");
    } else {
        runtest.fail("QueryOsmStats::connect()");
    }

    // ChangeSet foo = tests.getChange(10);
    // os.applyChange(foo);
    // foo = tests.getChange(15);
    // os.applyChange(foo);
    // foo = tests.getChange(20);
    // os.applyChange(foo);
    // tests.importChanges("/home/rob/projects/HOT/changesets-reduced.osm");
};


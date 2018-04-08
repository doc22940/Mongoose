/**
 * demo.cpp
 * Runs a variety of computations on several input matrices and outputs
 * the results. Does not take any input. This application can be used to
 * test that compilation was successful and that everything is working
 * properly.
 */

#include "Mongoose.hpp"
#include <ctime>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace Mongoose;
using namespace std;

int main(int argn, const char **argv)
{
    const std::string demo_files[12] = {
        "bcspwr01.mtx",
        "bcspwr02.mtx",
        "bcspwr03.mtx",
        "bcspwr04.mtx",
        "bcspwr05.mtx",
        "bcspwr06.mtx",
        "bcspwr07.mtx",
        "bcspwr08.mtx",
        "bcspwr09.mtx",
        "bcspwr10.mtx",
        "jagmesh7.mtx",
        "troll.mtx"
    };

    cout << "********************************************************************************" << endl;
    cout << "Mongoose Graph Partitioning Library, Version " << mongooseVersion() << endl;
    cout << "Copyright (C) 2017-2018" << endl;
    cout << "Scott P. Kolodziej, Nuri S. Yeralan, Timothy A. Davis, William W. Hager" << endl;
    cout << "Mongoose is licensed under Version 3 of the GNU General Public License." << endl;
    cout << "Mongoose is also available under other licenses; contact authors for details." << endl;

    clock_t start = clock();
    double duration;

    for (int k = 0; k < 12; k++)
    {
        cout << "********************************************************************************" << endl;
        cout << "Computing an edge cut for " << demo_files[k] << "..." << endl;
        
        clock_t trial_start = clock();
        Options *options = Options::Create();
        if (!options) return EXIT_FAILURE; // Return an error if we failed.

        options->matchingStrategy = HEMSRdeg;
        options->guessCutType = GuessQP;

        Graph *graph = readGraph("../Matrix/" + demo_files[k]);
        if (!graph)
        {
            return EXIT_FAILURE;
        }

        ComputeEdgeSeparator(graph, options);

        cout << "Partitioning Complete!" << endl;
        cout << "Cut Cost:       " << setprecision(2) << graph->cutCost << endl;
        cout << "Cut Imbalance:  " << setprecision(2) << 100*(graph->imbalance) << "%" << endl;

        double trial_duration = (std::clock() - trial_start) / (double) CLOCKS_PER_SEC;
        cout << "Trial Time:     " << trial_duration*1000 << "ms" << endl;

        options->~Options();
        graph->~Graph();
    }

    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;

    cout << "********************************************************************************" << endl;
    cout << "Total Demo Time:  " << setprecision(2) << duration << "s" << endl;

    cout << endl;
    cout << "               **************************************************               " << endl;
    cout << "               ***************** Demo Complete! *****************               " << endl;
    cout << "               **************************************************               " << endl;
    cout << endl;

    /* Return success */
    return EXIT_SUCCESS;
}
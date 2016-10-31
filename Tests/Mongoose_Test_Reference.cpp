#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_Conditioning.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include <iostream>
#include <fstream>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

using namespace Mongoose;

void runReferenceTest(const std::string inputFile)
{
    // Given a symmetric matrix
    Options *options;
    Graph *G;
    clock_t t;
    
    options = Options::Create();

    if (!options)
    {
        // Ran out of memory
        Logger::log(Test, "Error creating Options struct in Performance Test");
        SuiteSparse_free(options);
        assert(false);
    }

    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        SuiteSparse_free(options);
        SuiteSparse_free(G);
    }

    // An edge separator should be computed with default options
    t = clock();
    int error = ComputeEdgeSeparator(G, options);
    t = clock() - t;

    if (error)
    {
        // Error occurred
    }
    else
    {
        std::ofstream ofs (inputFile + "_result.txt", std::ofstream::out);
        ofs << "InputFile: " << inputFile << std::endl;
        ofs << "TotalTime: " << ((double) t)/CLOCKS_PER_SEC << std::endl;
        ofs << "CutSize: " << G->cutCost << std::endl;
        ofs << "Imbalance: " << G->imbalance << std::endl;

        for (int i = 0; i < G->n; i++)
        {
            ofs << i << " ";
            if (G->partition[i] == 0)
            {
                ofs << "A" << std::endl;
            }
            else
            {
                ofs << "B" << std::endl;
            }
        }
        ofs.close();

        
        std::ifstream ifs ("../Tests/Results/" + inputFile + "_result.txt");
        std::string input;

        // Read file name
        ifs.ignore(200, ' ');
        ifs >> input;
        std::cout << "File Name: " << input << std::endl;

        // Read Total Time
        ifs.ignore(200, ' ');
        ifs >> input;
        double ref_time = std::stod(input);
        double test_time = ((double) t)/CLOCKS_PER_SEC;
        std::cout << "Test Time:      " << test_time << std::endl;
        std::cout << "Reference Time: " <<  ref_time << std::endl;
        

        // Read Cut Size
        ifs.ignore(200, ' ');
        ifs >> input;
        double ref_cut_size = std::stod(input);
        std::cout << "Test Cut Size: "      << G->cutCost    << std::endl;
        std::cout << "Reference Cut Size: " <<  ref_cut_size << std::endl;

        ifs.close();

        assert(test_time <= 2*ref_time && 
            "FAIL: Run time significantly exceeds reference run time");
        assert(fabs(G->cutCost) <= 1.1*fabs(ref_cut_size) && 
            "FAIL: Cut cost significantly exceeds reference cut size");
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);
}
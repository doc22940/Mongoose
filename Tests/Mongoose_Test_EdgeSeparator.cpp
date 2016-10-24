#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_Conditioning.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include <iostream>
#include <fstream>

using namespace Mongoose;

const std::string mm_data[] = {
    "Pd",
    "bcspwr01",
    "bcspwr02",
    "bcspwr03",
    "bcspwr04",
    "bcspwr05",
    "bcspwr06",
    "bcspwr07",
    "bcspwr08",
    "bcspwr09",
    "bcspwr10"
};

void runEdgeSeparatorTests()
{
    Logger::log(Test, "Running Edge Separator Test on all test matrices");

    for (int k = 0; k < 11; k++)
    {
        runEdgeSeparatorTest("../Matrix/" + mm_data[k] + ".mtx");
    }

    Logger::log(Test, "Completed Edge Separator Test on all test matrices");
}

void runEdgeSeparatorTest(const std::string inputFile)
{
    Logger::log(Test, "Running Edge Separator Test on " + inputFile);
        
    // Given a symmetric matrix...
    Options *options;
    Graph *G;
    
    options = Options::Create();
    if (!options)
    {
        // Ran out of memory
        SuiteSparse_free(options);
        assert(false);
    }
    
    // Read graph from file
    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        assert(false);
    }

    // An edge separator should be computed with default options
    int error = ComputeEdgeSeparator(G, options);

    if (error)
    {
        // Error occurred
        assert(false);
    }
    else
    {
        // The graph should be partitioned
        assert (G->partition != NULL);
        for (int i = 0; i < G->n; i++)
        {
            bool equals_0 = (G->partition[i] == 0);
            bool equals_1 = (G->partition[i] == 1);
            assert(equals_0 != equals_1);
        }
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);

    Logger::log(Test, "Edge Separator Test Completed Successfully");
}
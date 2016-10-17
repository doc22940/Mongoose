
#include "Mongoose_Conditioning.hpp"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_Interop.hpp"

namespace Mongoose
{

/******************************************************************************
 * Condition the given graph to make sure it's:
 *   1. symmetric
 *   2. no self edges
 *   3. has positive edge weights
 *   4. has positive node weights
 * Also compute starting edge weights and the sum of edge weights.
 *****************************************************************************/
Graph *conditionGraph
(
    Graph *G,
    Options *O,
    bool resetEW,
    bool resetNW
)
{
    if (!G || !O) return NULL;

    Int n = G->n;
    Int *Gp = G->p;
    Weight *Gx = G->x;

    /* Make sure the matrix is symmetric */
    cs *A = GraphToCSparse3(G);
    cs *T = cs_transpose(A, true);
    cs *R = cs_add(A, T, 0.5, 0.5);
    cs_spfree(T);
    Graph *oG = G; // memory alias.
    G = CSparse3ToGraph(R, resetEW, resetNW);
    oG->~Graph();
    SuiteSparse_free(oG);

    /* If we're out of memory converting CSparse3 to Graph, free R. */
    if (!G)
    {
        Logger::log(Error, 
            "Ran out of memory during Mongoose::conditionGraph\n");
        cs_spfree(R);
        return NULL;
    }

    /* Make edge weights positive. */
    for (Int k = 0; k < n; k++)
    {
        for (Int p = Gp[k]; p < Gp[k+1]; p++) Gx[p] = fabs(Gx[p]);
    }

    /* Return the conditioned graph. */
    return G;
}

} // end namespace Mongoose
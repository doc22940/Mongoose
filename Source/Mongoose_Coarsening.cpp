/**
 * @file mongoose_coarsening.cpp
 * @author Nuri Yeralan, Scott Kolodziej
 * @date 23 Aug 2015
 * @brief Coarsening of a graph given a previously determined matching
 *
 * In order to operate on extremely large graphs, a pre-processing is done
 * to reduce the size of the graph while maintaining its overall structure.
 * Given a matching of vertices with other vertices (e.g. heavy edge matching, 
 * random, etc.), coarsening constructs the new, coarsened graph.
 */

#include "Mongoose_Coarsening.hpp"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

/**
 * @brief Coarsen a Graph given a previously calculated matching
 *
 * Given a Graph @p G, coarsen returns a new Graph that is coarsened according
 * to the matching given by G->matching, G->matchmap, and G->invmatchmap.
 * G->matching must be built such that matching[a] = b+1 and matching[b] = a+1 
 * if vertices a and b are matched. G->matchmap is a mapping from fine to coarse
 * vertices, so matchmap[a] = matchmap[b] = c if vertices a and b are matched 
 * and mapped to vertex c in the coarse graph. Likewise, G->invmatchmap is 
 * one possible inverse of G->matchmap, so invmatchmap[c] = a or 
 * invmatchmap[c] = b if a coarsened vertex c represents the matching of 
 * vertices a and b in the refined graph.
 * 
 * @code
 * Graph coarsened_graph = coarsen(large_graph, options);
 * @endcode
 * 
 * @param G Graph to be coarsened
 * @param O Option struct specifying if debug checks should be done
 * @return A coarsened version of G
 * @note Allocates memory for the coarsened graph, but frees on error.
 */
Graph *coarsen(Graph *G, Options *O)
{
    Logger::tic(CoarseningTiming);

    Int cn = G->cn;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gx = G->x;
    Weight *Gw = G->w;

    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;

    /* Build the coarse graph */
    Graph *C = Graph::Create(G);
    if (!C) return NULL;

    Int *Cp = C->p;
    Int *Ci = C->i;
    Weight *Cx = C->x;
    Weight *Cw = C->w;
    Weight *gains = C->vertexGains;
    Int munch = 0;
    Weight X = 0.0;

    /* Hashtable stores column pointer values. */
    Int *htable = (Int*) SuiteSparse_malloc(cn, sizeof(Int));
    if (!htable)
    {
        C->~Graph();
        SuiteSparse_free(C);
        return NULL;
    }
    for (Int i = 0; i < cn; i++) htable[i] = -1;

    /* For each vertex in the coarse graph. */
    for (Int k = 0; k < cn; k++)
    {
        /* Load up the inverse matching */
        Int v[3] = {-1, -1, -1};
        v[0] = invmatchmap[k];
        v[1] = MONGOOSE_GETMATCH(v[0]);
        if (v[0] == v[1]) { v[1] = -1; }
        else
        {
            v[2] = MONGOOSE_GETMATCH(v[1]);
            if (v[0] == v[2]) { v[2] = -1; }
        }

        Int ps = Cp[k] = munch;     /* The munch start for this column */

        Weight nodeWeight = 0.0;
        Weight sumEdgeWeights = 0.0;
        for (Int i = 0; i < 3 && v[i] != -1; i++)
        {
            /* Read the matched vertex and accumulate the node weight. */
            Int vertex = v[i];
            nodeWeight += Gw[vertex];

            for (Int p = Gp[vertex]; p < Gp[vertex+1]; p++)
            {
                Int toCoarsened = matchmap[Gi[p]];
                if (toCoarsened == k) continue;         /* Delete self-edges */

                /* Read the edge weight and accumulate the sum of edge weights. */
                Weight edgeWeight = Gx[p];
                sumEdgeWeights += Gx[p];

                /* Check the hashtable before scattering. */
                Int cp = htable[toCoarsened];
                if (cp < ps)             /* Hasn't been seen yet this column */
                {
                    htable[toCoarsened] = munch;
                    Ci[munch] = toCoarsened;
                    Cx[munch] = edgeWeight;
                    munch++;
                }
                /* If the entry already exists & we have edge weights,
                 * sum the edge weights here. */
                else
                {
                    Cx[cp] += edgeWeight;
                }
            }
        }

        /* Save the node weight. */
        Cw[k] = nodeWeight;

        /* Save the sum of edge weights and initialize the gain for k. */
        X += sumEdgeWeights;
        gains[k] = -sumEdgeWeights;
    }

    /* Set the last column pointer */
    Cp[cn] = munch;
    C->nz = munch;

    /* Save the sum of edge weights on the graph. */
    C->X = X;
    C->H = 2.0 * X;

    /* Cleanup resources */
    SuiteSparse_free(htable);

    /* If we want to do expensive checks, make sure we didn't break
     * the problem into multiple connected components. */
    if (O->doExpensiveChecks)
    {
        Weight W = 0.0;
        for (Int k = 0; k < cn; k++)
        {
            Int degree = Cp[k+1] - Cp[k];
            assert(degree > 0);

            W += Cw[k];
        }
        assert(W == C->W);
    }

    Logger::toc(CoarseningTiming);

    /* Return the coarse graph */
    return C;
}

} // end namespace Mongoose

#include "mongoose_mex.hpp"

using namespace Mongoose;

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{    
    const char* usage = "Usage: [G_coarse, A_coarse] = mongoose_coarsen_mex(G, (O, A))";
    if(nargout > 3 || nargin < 1 || nargin > 3) mexErrMsgTxt(usage);
    
    const mxArray *matGraph = pargin[0];
    const mxArray *matOptions = (nargin >= 2 ? pargin[1] : NULL);
    const mxArray *matNodeWeights = (nargin >= 3 ? pargin[2] : NULL);
    
    /* Get the graph from the MATLAB inputs. */
    Graph *G = mex_get_graph(matGraph, matNodeWeights);
    
    if(!G)
        mexErrMsgTxt("Unable to get Graph struct");
    
    /* Get the options from the MATLAB inputs. */
    Options *O = mex_get_options(matOptions);
    if(!O)
        mexErrMsgTxt("Unable to get Options struct");

    initialize(G, O);
    match(G, O);
    Graph *G_coarse = coarsen(G, O);

    cs *G_matrix = GraphToCSparse3(G_coarse, false);

    /* Copy the coarsened graph back to MATLAB. */
    pargout[0] = cs_mex_put_sparse(&G_matrix);
    gp_mex_put_double(G_coarse->n, G_coarse->w, &pargout[1]);
    pargout[2] = gp_mex_put_int(G->matchmap, G->n, 1, 0);

    /* Cleanup */
    G->i = NULL;
    G->p = NULL;
    G->x = NULL;
    G->w = NULL;
    G->~Graph();
    SuiteSparse_free(G);
    G_coarse->i = NULL;
    G_coarse->p = NULL;
    G_coarse->x = NULL;
    G_coarse->w = NULL;
    G_coarse->matchmap = NULL;
    G_coarse->~Graph();
    SuiteSparse_free(G_coarse);
    O->~Options();
    SuiteSparse_free(O);
}
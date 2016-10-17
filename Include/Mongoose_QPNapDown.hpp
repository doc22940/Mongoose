#ifndef Mongoose_QPNapDown_hpp
#define Mongoose_QPNapDown_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

Double QPnapdown            /* return lambda */
(
    Double *x,              /* holds y on input, not modified */
    Int n,                  /* size of x */
    Double lambda,          /* initial guess for the shift */
    Double *a,              /* input constraint vector */
    Double b,               /* input constraint scalar */
    Double *breakpts,       /* break points */
    Int *bound_heap,        /* work array */
    Int *free_heap          /* work array */
);

} // end namespace Mongoose

#endif
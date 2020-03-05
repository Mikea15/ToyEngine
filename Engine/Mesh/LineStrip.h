#pragma once

#include "Mesh.h"


    /* 

      A line strip of specified width / length.

      Segments equals the number of 2-triangle sets while width equals the width of the line. 
      Useful for line systems that require a filled line strip.

    */
    class LineStrip : public Mesh
    {
    public:
        LineStrip(float width, unsigned int segments);
    };


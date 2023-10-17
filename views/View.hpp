//
//  View.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef View_h
#define View_h

#include <iostream>
#include "../Timer.hpp"
#include "../BasicTypes.hpp"

namespace ECE141 {

    class View {
    public:
        virtual         ~View() {}
        virtual bool    show(std::ostream& aStream, Timer& aTimer) = 0;
    };

}

#endif /* View_h */

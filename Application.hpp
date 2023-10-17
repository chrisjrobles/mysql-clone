//
//  AppProcessor.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Application_hpp
#define Application_hpp

#include <stdio.h>
#include <iostream>
#include "CmdProcessor.hpp"
#include "DBProcessor.hpp"

namespace ECE141 {

    class Application : public CmdProcessor {
    public:
    
        Application(std::ostream& anOutput);
        virtual ~Application();

        virtual StatusResult    handleInput(std::istream& anInput);
        virtual bool            isKnown(Keywords aKeyword) override;
        virtual CmdProcessor*   recognizes(Tokenizer& aTokenizer) override;
        virtual Statement*      makeStatement(Tokenizer& aTokenizer) override;
        virtual StatusResult    run(Statement* aStmt, const Timer& aTimer) override;
    
    protected:
        DBProcessor *dbProc;
    };
}

#endif /* Application_hpp */

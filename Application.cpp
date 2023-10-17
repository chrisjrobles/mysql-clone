//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Application.hpp"
#include "Config.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {

    Application::Application(std::ostream &anOutput)
        : CmdProcessor(anOutput), dbProc(nullptr) {
    }

    Application::~Application() {
        if (dbProc) delete dbProc;
    }

    // USE: -----------------------------------------------------

    bool Application::isKnown(Keywords aKeyword) {
        static Keywords theKnown[] = { Keywords::quit_kw, Keywords::version_kw, Keywords::help_kw };
        auto theIt = std::find(std::begin(theKnown), std::end(theKnown), aKeyword);
        return theIt != std::end(theKnown);
    }

    CmdProcessor* Application::recognizes(Tokenizer& aTokenizer) {
        if (isKnown(aTokenizer.current().keyword)) {
            return this;
        }

        if (!dbProc) dbProc = new DBProcessor{output};

        return dbProc->recognizes(aTokenizer);
    }

    StatusResult shutdown(std::ostream& anOutput) {
        anOutput << "DB::141 is shutting down.\n";
        return StatusResult(ECE141::userTerminated);
    }

    StatusResult displayVersion(std::ostream& anOutput) {
        anOutput << "Version " << Config::getAppVersion() << ".\n";
        return StatusResult(ECE141::noError);
    }

    StatusResult displayHelp(std::ostream& anOutput) {
        anOutput << "Help system ready.\n";
        return StatusResult(ECE141::noError);
    }

    StatusResult Application::run(Statement* aStmt, const Timer& aTimer) {
        switch (aStmt->getType()) {
        case Keywords::quit_kw: return shutdown(output);
        case Keywords::version_kw: return displayVersion(output);
        case Keywords::help_kw: return displayHelp(output);
        default: break;
        }
        return StatusResult{ Errors::noError };
    }

    // USE: retrieve a statement based on given text input...
    Statement* Application::makeStatement(Tokenizer& aTokenizer) {
        Token theToken = aTokenizer.current();
        if (isKnown(theToken.keyword)) {
            aTokenizer.next(); //skip ahead...
            return new Statement(theToken.keyword);
        }

        return nullptr;
    }

    //build a tokenizer, tokenize input, ask processors to handle...
    StatusResult Application::handleInput(std::istream& anInput) {

        //tokenize the input from aStream...
        Tokenizer theTokenizer(anInput);
        StatusResult theResult = theTokenizer.tokenize();

        while (theResult && theTokenizer.remaining()) {
            if (auto* theProc = recognizes(theTokenizer)) {
                Timer theTimer;
                theTimer.start();
                if (auto* theCmd = theProc->makeStatement(theTokenizer)) {
                    theResult = theProc->run(theCmd, theTimer);
                    if (theResult) {
                        theTokenizer.next();
                        theTokenizer.skipIf(';');
                    }
                    delete theCmd;
                }
            }
            else theTokenizer.next();
        }
        return theResult;
    }
}
/* BZWorkbench
 * Copyright (c) 1993 - 2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef DYNAMICCOLOR_H_
#define DYNAMICCOLOR_H_

#include "../DataEntry.h"
#include "../ColorCommand.h"

class dynamicColor : public DataEntry {

public:

	// default constructor
	dynamicColor();

	static DataEntry* init() { return new dynamicColor(); }

	// getter
	string get(void);

	// bzw methods
	bool parse( std::string& line );
	void finalize();

	// toString
	string toString(void);

	// render
	int render(void);

	// getters
	string& getName() { return this->name; }
	vector<ColorCommand>& getRedCommands() { return this->redCommands; }
	vector<ColorCommand>& getGreenCommands() { return this->greenCommands; }
	vector<ColorCommand>& getBlueCommands() { return this->blueCommands; }
	vector<ColorCommand>& getAlphaCommands() { return this->alphaCommands; }

	// setters
	void setName( const string& _name );
	void setRedCommands( const vector<ColorCommand>& commands ) { this->redCommands = commands; }
	void setGreenCommands( const vector<ColorCommand>& commands ) { this->greenCommands = commands; }
	void setBlueCommands( const vector<ColorCommand>& commands ) { this->blueCommands = commands; }
	void setAlphaCommands( const vector<ColorCommand>& commands ) { this->alphaCommands = commands; }


private:
	string name;

	vector<ColorCommand> redCommands;
	vector<ColorCommand> greenCommands;
	vector<ColorCommand> blueCommands;
	vector<ColorCommand> alphaCommands;
};

#endif /*DYNAMICCOLOR_H_*/

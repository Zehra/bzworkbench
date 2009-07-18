/* BZWorkbench
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef MESHBOX_H_
#define MESHBOX_H_

#include "objects/arc.h"

#include <string>

class meshbox : public arc {

public:
	// construct an empty box
	meshbox();
	
	static DataEntry* init() { return new meshbox(); }

	void setDefaults();

	bool parse( std::string& line );
};

#endif /*MESHBOX_H_*/

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

#include "objects/meshbox.h"

// construct an empty box
meshbox::meshbox() : 
	bz2object("meshbox", "<position><rotation><size><top matref><outside matref><matref><phydrv><obstacle>") {
	topMaterials = vector<string>();
	outsideMaterials = vector<string>();
}

// construct a box from data
meshbox::meshbox(string& data) : 
	bz2object("meshbox", "<position><rotation><size><top matref><outside matref><matref><phydrv><obstacle>", data.c_str()) {
	topMaterials = vector<string>();
	outsideMaterials = vector<string>();
	
	update(data);
}

// getter
string meshbox::get(void) { return toString(); }

// setter
int meshbox::update(string& data) {
	// get the header
	const char* header = getHeader().c_str();
	
	// get the sections
	vector<string> lines = BZWParser::getSectionsByHeader(header, data.c_str());
	
	// quit if there aren't any
	if(lines[0] == BZW_NOT_FOUND)
		return 0;
		
	if(!hasOnlyOne(lines, "meshbox"))
		return 0;
	
	// quit if there are multiple ones
	if(lines.size() > 1) {
		printf("meshbox::update(): Error! Defined %d meshboxes!\n", (int)lines.size());
		return 0;
	}
	
	// get the data
	const char* meshBoxData = lines[0].c_str();
	
	// find occurences of top
	vector<string> tops = BZWParser::getValuesByKey("top matref", header, meshBoxData);
	
	// find occurences of outside
	vector<string> outsides = BZWParser::getValuesByKey("outside matref", header, meshBoxData);
	
	// copy the data over
	if(!bz2object::update(data))
		return 0;
	topMaterials = tops;
	outsideMaterials = outsides;
	return 1;
}

// tostring
string meshbox::toString(void) {
	// get the top materials
	string topmats = string("");
	if(topMaterials.size() > 0) {
		for(vector<string>::iterator i = topMaterials.begin(); i != topMaterials.end(); i++) {
			topmats += string("  top matref ") + i->c_str() + "\n";	
		}
	}
	
	// get the outside materials
	string outsidemats = string("");
	if(outsideMaterials.size() > 0) {
		for(vector<string>::iterator i = outsideMaterials.begin(); i != outsideMaterials.end(); i++) {
			outsidemats += string("  outside matref ") + i->c_str() + "\n";	
		}
	}
	return string("meshbox\n") +
				  BZWLines( this ) +
				  outsidemats +
				  topmats +
				  "end\n";
}

// render
int meshbox::render(void) {
	return 0;	
}

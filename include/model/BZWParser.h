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

#ifndef BZWPARSER_H_
#define BZWPARSER_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

// generic "not found" return value
#define BZW_NOT_FOUND "NOT FOUND"

class Model;

class BZWParser {
 public:

  // unused
  BZWParser() { }
  virtual ~BZWParser() { }

  // init method to get Model reference
  // this MUST be called BEFORE any other method
  static void init( Model* model ) { BZWParser::_modelRef = model; }

  // simplest method:  get a value from a non-repeatable key in a line
  static string value(const char* key, const char* text);

  // simpler method:  get the key from a line
  static string key(const char* text);

  // get the terminator token of a key
  static string terminatorOf(const char* key);

  // get the header of a chunk of BZW text
  static string headerOf(const char* text);

  // get the hierarchy of an object
  static string hierarchyOf(const char* text);

  // get the individual lines out of a section
  static vector<string> getLines(const char* header, const char* section);

  // get all lines in a section that begin with the given key
  static vector<string> getLinesByKey(const char* key, const char* header, const char* section);

  // chunk of a long string of text into an array of sections
  static const vector<string> getSections(const char* text);

  // find sections starting with a particular header from a vector of sections
  // (meant to be used in conjunction with getSections()
  static const vector<string> findSections(const char* header, vector<string>& sections);

  // get all sections from a chunk of text that start with header
  static const vector<string> getSectionsByHeader(const char* header, const char* text);
  static const vector<string> getSectionsByHeader(const char* header, const char* text, const char* footer);
  // use this for sections with subobjects
  static const vector<string> getSectionsByHeader(const char* header, const char* text, const char* footer, const char* internalSectionKeys, const char* sectionsToIgnore);

  // get all values from a key in a section
  static vector<string> getValuesByKey(const char* key, const char* header, const char* section);

  // get lines starting with a key from a set of keys keys in a section, preserving the order
  static vector<string> getLinesByKeys(vector<string> keys, const char* header, const char* section);

  // get the values corresponsding to certain keys in order
  static vector<string> getValuesByKeys(vector<string> keys, const char* header, const char* section );

  // get the values corresponding to a key and a list of faces
  static vector<string> getValuesByKeyAndFaces(const char* key, vector<string> faces, const char* header, const char* section );

  // get all values from a line
  static vector<string> getLineElements(const char* line);
  static vector<string> getLineElements(const char* line, int count);

  // the big tamale: the top-level file loader
  static bool loadFile(const char* filename);

  // get list of integers from a string
  static vector<int> getIntList( const char* line );

  // remove whitespace from either side of a string (includes comments)
  static std::string cutWhiteSpace(std::string line);

  // checks if a string is all whitespace
  static bool allWhitespace( const char* line );

 private:

  static Model* _modelRef;

};

#endif /*BZWPARSER_H_*/

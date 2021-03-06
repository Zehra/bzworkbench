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

#ifndef TRANSFORMATION_H_
#define TRANSFORMATION_H_

#include <stdlib.h>
#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include "QuickLabel.h"
#include "ftoa.h"
#include "Transform.h"

#include <string>
#include <vector>
#include <memory>

using namespace std;

// transformation widget for MasterconfigurationDialog
class TransformWidget : public Fl_Group {
public:

	// unused; prevents the build from breaking
	// (Fl_Group's copy constructor is private)
	TransformWidget(const TransformWidget& t);
	
	// the format for fields is "<type1:|field1|field2|field3|" ... "|fieldN|>"
	TransformWidget(int x, int y, int WIDTH, int HEIGHT, const char* fields, bool active);
	
	virtual ~TransformWidget();
	
	// read the transformation type from the menu
	static void getTransformationCallback(Fl_Widget* w, void* data);
	
	// getters
	string getTransformationString(void);
	TransformType getTransformationType(void);
	vector<string> getSupportedTransformations(void);
	bool isActive(void);
	osg::Vec4 getFields();
	
	// setters
	// changes the type of transformation and changes the fields
	void setTransformationType(string& s);
	void setTransformationType(const char* s);
	void setTransformationType(TransformType t);
	
	// set the fields with string represented values
	void setFields(vector<string> fieldValues);
	
	// set the fields with float values
	void setFields(vector<float> fieldValues);

	// set the fields with a osg::vec4
	void setFields(osg::Vec4 fieldValues);
	
	
	// return string representation in BZW format
	string toString();
	
private:

	// label the menu button with the selection
	void getTransformationCallback_real(TransformWidget* t, Fl_Menu_Button* mb);
	
	// change callbacks based on a new selection
	void changeFields(const char* name);
	
	// data
	vector<string> transformTypes;
	Fl_Menu_Button* typeMenu;
	Fl_Check_Button* activeButton;
	vector<Fl_Float_Input*> fields;
	string format;
};
#endif /*TRANSFORMATION_H_*/

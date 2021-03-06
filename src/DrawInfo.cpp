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

#include "DrawInfo.h"


// default constructor
DrawInfo::DrawInfo() :
	DataEntry("drawinfo", "<dlist><decorative><angvel><extents><sphere><corner><vertex><normal><texcoord><lod>") {

	vertices = vector<Point3D>();
	normals = vector<Point3D>();
	texcoords = vector<Point2D>();
	corners = vector<Index3D>();

	minExtents = Point3D(0.0f, 0.0f, 0.0f);
	maxExtents = Point3D(0.0f, 0.0f, 0.0f);
	spherePosition = Point3D(0.0f, 0.0f, 0.0f);

	angvel = 0.0f;
	sphereRadius = 0.0f;

	dlist = false;
	decorative = false;
		
	currentLOD = NULL;
}

// constructor with data
DrawInfo::DrawInfo(string& data) :
DataEntry("drawinfo", "<dlist><decorative><angvel><extents><sphere><corner><vertex><normal><texcoord><lod>", data.c_str()) {

	vertices = vector<Point3D>();
	normals = vector<Point3D>();
	texcoords = vector<Point2D>();
	corners = vector<Index3D>();

	minExtents = Point3D(0.0f, 0.0f, 0.0f);
	maxExtents = Point3D(0.0f, 0.0f, 0.0f);
	spherePosition = Point3D(0.0f, 0.0f, 0.0f);

	angvel = 0.0f;
	sphereRadius = 0.0f;

	dlist = true;
	decorative = false;

	this->update(data);
}

string DrawInfo::get(void) { return this->toString(); }

// setter
int DrawInfo::update(string& data) {
	const char* _header = this->getHeader().c_str();

	// get the section
	vector<string> lines = BZWParser::getSectionsByHeader(_header, data.c_str());

	// break if there's no section
	if(lines[0] == BZW_NOT_FOUND)
		return 0;

	// break if there's too many
	if(!hasOnlyOne(lines, _header))
		return 0;

	// get the data
	const char* drawInfoData = lines[0].c_str();

	// get the vertices
	vector<string> vertexVals = BZWParser::getValuesByKey("vertex", _header, drawInfoData);

	// get the texcoords
	vector<string> texCoordVals = BZWParser::getValuesByKey("texcoord", _header, drawInfoData);

	// get the normals
	vector<string> normalVals = BZWParser::getValuesByKey("normal", _header, drawInfoData);

	// get the dlist
	vector<string> dlistVals = BZWParser::getValuesByKey("dlist", _header, drawInfoData);

	// get the decorative
	vector<string> decorativeVals = BZWParser::getValuesByKey("decorative", _header, drawInfoData);

	// get angvel
	vector<string> angvelVals = BZWParser::getValuesByKey("angvel", _header, drawInfoData);
	if(angvelVals.size() > 1) {
		printf("mesh::DrawInfo::update(): Error! Defined \"angvel\" %d times!\n", (int)angvelVals.size());
		return 0;	
	}
	if(angvelVals.size() > 0 && !hasNumElements(angvelVals[0], 1))
		return 0;

	// get corners
	vector<string> cornerVals = BZWParser::getValuesByKey("corner", _header, drawInfoData);

	// get spheres
	vector<string> sphereVals = BZWParser::getValuesByKey("sphere", _header, drawInfoData);
	if(sphereVals.size() > 1) {
		printf("mesh::DrawInfo::update(): Error! Defined \"sphere\" %d times!\n", (int)sphereVals.size());
		return 0;	
	}
	if(sphereVals.size() > 0 && !hasNumElements(sphereVals[0], 4))
		return 0;

	// get extents
	vector<string> extentsVals = BZWParser::getValuesByKey("extents", _header, drawInfoData);
	if(extentsVals.size() > 1) {
		printf("mesh::DrawInfo::update(): Error! Defined \"extents\" %d times!\n", (int)sphereVals.size());
		return 0;	
	}
	if(extentsVals.size() > 0 && !hasNumElements(extentsVals[0], 6))
		return 0;

	// need same amount of corners as vertexes
	if(cornerVals.size() != vertexVals.size()) {
		printf("mesh::DrawInfo::update(): Error! Unequal numbers of corners and vertexes\n");
		return 0;
	}

	// need same amount of corners as normals
	if(cornerVals.size() != normalVals.size()) {
		printf("mesh::DrawInfo::update(): Error! Unequal numbers of corners and normals\n");
		return 0;
	}

	// parse the values
	vector<Point3D> vertexData = vector<Point3D>();
	vector<Index3D> cornerData = vector<Index3D>();
	vector<Point3D> vectorData = vector<Point3D>();
	vector<Point2D> texCoordData = vector<Point2D>();

	// validate the vertexes (i.e. make sure they all have 3 values)
	if(vertexVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = vertexVals.begin(); i != vertexVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::DrawInfo::update(): Error! \"vertex\" in \"vertex %s\" needs 3 values!\n", i->c_str());
				return 0;
			}
			vertexData.push_back( Point3D( i->c_str() ) );
		}
	}

	// validate the corners (i.e. make sure they all have 3 values)
	if(cornerVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = cornerVals.begin(); i != cornerVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::DrawInfo::update(): Error! \"corner\" in \"corner %s\" needs 3 values!\n", i->c_str());
				return 0;
			}
			cornerData.push_back( Index3D( i->c_str() ) );
		}
	}

	// validate the normals (i.e. make sure they all have 3 values)
	if(normalVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = normalVals.begin(); i != normalVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::DrawInfo::update(): Error! \"normal\" in \"normal %s\" needs 3 values!\n", i->c_str());
				return 0;
			}
			vectorData.push_back( Point3D( i->c_str() ) );
		}
	}

	// validate the texture coordinates (i.e. make sure they all have 2 values)
	if(vertexVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = texCoordVals.begin(); i != texCoordVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 2) {
				printf("mesh::DrawInfo::update(): Error! \"texcoord\" in \"texcoord %s\" needs 2 values!\n", i->c_str());
				return 0;
			}
			texCoordData.push_back( Point2D( i->c_str() ) );
		}
	}

	// get the "extents" if it exists
	Point3D eLow = Point3D(0.0f, 0.0f, 0.0f), eHigh = Point3D(0.0f, 0.0f, 0.0f);
	if(extentsVals.size() > 0) {
		vector<string> extentsParams = BZWParser::getLineElements( extentsVals[0].c_str() );
		string eLowString = extentsParams[0] + " " + extentsParams[1] + " " + extentsParams[2];
		string eHighString = extentsParams[3] + " " + extentsParams[4] + " " + extentsParams[5];
		eLow = Point3D(eLowString.c_str());
		eHigh = Point3D(eHighString.c_str());
	}

	// get the "sphere" if it exists
	Point3D spherePoint(0.0f, 0.0f, 0.0f);
	float sphereRad = 0;
	if(sphereVals.size() > 0) {
		vector<string> sphereParams = BZWParser::getLineElements( sphereVals[0].c_str() );
		string pString = sphereParams[0] + " " + sphereParams[1] + " " + sphereParams[2];
		sphereRad = atof( sphereParams[3].c_str() );
		spherePoint = Point3D(pString.c_str());
	}
	// get LOD blocks
	vector<string> lodBlocks = BZWParser::getSectionsByHeader( "lod", drawInfoData);
	vector<LOD> lodElements = vector<LOD>();
	if(lodBlocks.size() > 0) {
		for(vector<string>::iterator i = lodBlocks.begin(); i != lodBlocks.end(); i++) {
			lodElements.push_back( LOD( *i ) );
		}
	}

	// do base-class update
	if(!DataEntry::update(data))
		return 0;

	// finally, set the data
	this->vertices = vertexData;
	this->texcoords = texCoordData;
	this->corners = cornerData;
	this->normals = vectorData;
	this->minExtents = eLow;
	this->maxExtents = eHigh;
	this->sphereRadius = sphereRad;
	this->spherePosition = spherePoint;
	this->angvel = (angvelVals.size() > 0 ? atof( angvelVals[0].c_str() ) : 0.0f);
	this->dlist = (dlistVals.size() > 0 ? true : false);
	this->decorative = (decorativeVals.size() > 0 ? true : false);
	this->lods = lodElements;

	return 1;
}

// toString
string DrawInfo::toString(void) {
	// string-ify the vertices, normals, corners, LODs, and texcoords
	string vertexString(""), normalString(""), texcoordString(""), cornerString(""), lodString("");

	if(vertices.size() > 0) {
		for(vector<Point3D>::iterator i = vertices.begin(); i != vertices.end(); i++) {
			vertexString += "    vertex " + i->toString();
		}		
	}

	if(normals.size() > 0) {
		for(vector<Point3D>::iterator i = normals.begin(); i != normals.end(); i++) {
			normalString += "    normal " + i->toString();
		}		
	}

	if(texcoords.size() > 0) {
		for(vector<Point2D>::iterator i = texcoords.begin(); i != texcoords.end(); i++) {
			texcoordString += "    texcoord " + i->toString() + "\n";	
		}
	}

	if(corners.size() > 0) {
		for(vector<Index3D>::iterator i = corners.begin(); i != corners.end(); i++) {
			cornerString += "    corner " + i->toString();	
		}	
	}

	if(lods.size() > 0) {
		for(vector<LOD>::iterator i = lods.begin(); i != lods.end(); i++) {	
			lodString += "    " + i->toString();
		}
	}
	
	string extentsString = "    extents " + 
							ftoa(minExtents.x()) + " " + 
							ftoa(minExtents.y()) + " " + 
							ftoa(minExtents.z()) + " " + 
							ftoa(maxExtents.x()) + " " + 
							ftoa(maxExtents.y()) + " " + 
							ftoa(maxExtents.z()) + "\n";
	string sphereString = "    sphere "+ 
							ftoa(spherePosition.x()) + " " + 
							ftoa(spherePosition.y()) + " " + 
							ftoa(spherePosition.z()) + " " + 
							ftoa(sphereRadius) + "\n";

	return string("drawinfo\n") +
		(dlist == true ? "    dlist\n" : "") +
		(decorative == true ? "    decorative\n" : "") +
		(angvel > 0 ? ("    angvel " + ftoa(angvel) + " \n") : "") +
		extentsString +
		sphereString +
		vertexString +
		normalString +
		cornerString +
		texcoordString +
		lodString +
		"  end\n";
}

// render
int DrawInfo::render(void) {
	return 0;	
}

bool DrawInfo::parse( string& line ) { 
	string key = BZWParser::key( line.c_str() );
	string value = BZWParser::value( key.c_str(), line.c_str() );
	
	if ( currentLOD ) {
		if ( !currentLOD->parse( line ) ) {
			lods.push_back(*currentLOD);
			currentLOD = NULL;
		} 
		return true;
	}
	else if ( key == "dlist" ){
		// display list for all material sets
		dlist = true;
		return true; 
	}
	else if ( key == "decorative" ){
		// older clients with not see this mesh
		decorative = true;
		return true; 
	}
	else if ( key == "angvel" ){
		// <degrees/sec> rotation about initial Z axis
		angvel = atof( value.c_str() );
		return true; 
	}
	else if ( key == "extents" ){
		// <minX> <minY> <minZ> <maxX> <maxY> <maxZ>
		vector<string> values = BZWParser::getLineElements( value.c_str() );
		if(values.size() == 6){
			minExtents.set( atof(values[0].c_str()), atof(values[1].c_str()), atof(values[2].c_str()) );
			maxExtents.set( atof(values[3].c_str()), atof(values[4].c_str()), atof(values[5].c_str()) ); 
		} else
			throw BZWReadError( this, string( "extents should be followed by 6 floats: " ) + line );
		return true;
	}
	else if ( key == "sphere" ){
		// <x> <y> <z> <radiusSquared>
		Point4D temp = Point4D(value.c_str());
		spherePosition.set(temp.x(), temp.y(), temp.z());
		sphereRadius = temp.w();
		return true; 
	}
	else if ( key == "corner" ){
		// <v> <n> <t>         (repeatable)
		corners.push_back( Index3D( value.c_str() ) );
		return true; 
	}
	else if ( key == "vertex" ){
		// vertex 0.0 0.0 0.0         (repeatable)
		// if none present, uses mesh's vertices
		vertices.push_back( Point3D( value.c_str() ) );
		return true; 
	}
	else if ( key == "normal" ){
		// normal 0.0 0.0 0.0         (repeatable)
		// if none present, uses mesh's normals
		normals.push_back( Point3D( value.c_str() ) );
		return true; 
	}
	else if ( key == "texcoord" ){
		// texcoord 0.0 0.0           (repeatable)
		// if none present, uses mesh's texcoords
		texcoords.push_back( Point2D( value.c_str() ) );
		return true; 
	}
	else if ( key == "lod" ){
		// lod                        (repeatable)
		currentLOD = new LOD();
		return true; 
	}
	else if( key == "end" ){
		return false;
	}
	throw BZWReadError( this, string( "Unknown DrawInfo Command: " ) + line );
	return true;
} 

/**
* Helper method: convert an array of strings into an array of floats
*/
vector<float> DrawInfo::parseValues(vector<string>& values) {
	vector<float> ret = vector<float>();

	if(values.size() > 0) {
		for(vector<string>::iterator i = values.begin(); i != values.end(); i++) {
			ret.push_back( atof( i->c_str() ) );	
		}
	}

	return ret;
}

/**
* Helper method: convert an array of floats into a string
*/
string DrawInfo::stringify(vector<float>& values) {
	string ret = string("");

	if(values.size() > 0) {
		for(vector<float>::iterator i = values.begin(); i != values.end(); i++) {
			ret += string(ftoa(*i)) + " ";	
		}	
	}

	return ret;
}
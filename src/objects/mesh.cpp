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

#include "objects/mesh.h"

// default constructor
mesh::mesh(void) :
	bz2object("mesh", "<vertex><normal><texcoord><inside><outside><shift><scale><shear><spin><phydrv><smoothbounce><noclusters><face><drawinfo>") {
	
	vertices = vector<Point3D>();
	texCoords = vector<Point2D>();
	normals = vector<Point3D>();
	insidePoints = vector<Point3D>();
	outsidePoints = vector<Point3D>();
	decorative = false;
	materials = vector< material* >();
	faces = vector<MeshFace*>();
	drawInfo = NULL;
	materialMap = map<string, string>();
}

// constructor with data
mesh::mesh(string& data) :
	bz2object("mesh", "<vertex><normal><texcoord><inside><outside><shift><scale><shear><spin><phydrv><smoothbounce><noclusters><face><drawinfo>", data.c_str()) {
	
	vertices = vector<Point3D>();
	texCoords = vector<Point2D>();
	normals = vector<Point3D>();
	insidePoints = vector<Point3D>();
	outsidePoints = vector<Point3D>();
	decorative = false;
	materials = vector< material* >();
	faces = vector<MeshFace*>();
	drawInfo = NULL;
	materialMap = map<string, string>();
	
	update(data);
}

mesh::~mesh() {
	// clean up
	if (drawInfo != NULL)
		delete drawInfo;

	for (vector<MeshFace*>::iterator itr = faces.begin(); itr != faces.end(); itr++) {
		if ( *itr != NULL )
			delete *itr;
	}
}

// getter
string mesh::get(void) { return toString(); }

// setter
int mesh::update(string& data) {
	const char* header = getHeader().c_str();
	
	// get lines
	vector<string> chunks = BZWParser::getSectionsByHeader(header, data.c_str(), "endface");
	
	// break if there are none
	if(chunks[0] == BZW_NOT_FOUND) {
		printf("mesh not found\n");
		return 0;
	}
		
	// break if too many
	if(!hasOnlyOne(chunks, header))
		return 0;
		
	// get the data
	const char* meshData = chunks[0].c_str();
	
	vector<string> lines = BZWParser::getLines(header, data.c_str());

	MeshFace* face = NULL;
	DrawInfo* drawInfo = NULL;
	material* mat = NULL;
	physics* phydrv = NULL;
	bool smoothbounce = false;
	bool noClusters = false;
	bool driveThrough = false;
	bool shootThrough = false;
	for (vector<string>::iterator itr = lines.begin(); itr != lines.end(); itr++) {
		const char* line = (*itr).c_str();

		string key = BZWParser::key(line);

		if ( key == "lod" ) {
			lodOptions.push_back( BZWParser::value( "lod", line ) );
		}
		else if ( key == "endface" ) {
			if (face == NULL) {
				printf("mesh::update(): Warning! Extra 'endface' keyword found\n");
			} else {
				faces.push_back(face);
				face = NULL;
			}
		}
		else if ( face ) {
			face->parse( line );
		}
		else if ( key == "face" ) {
			if (face != NULL) {
				printf("mesh::update(): Warning! Discarding incomplete mesh face.\n");
				delete face;
			}
			face = new MeshFace();
		}
		else if ( key == "inside" ) {
			insidePoints.push_back( Point3D( BZWParser::value( "inside", line ) ) );
		}
		else if ( key == "outside" ) {
			outsidePoints.push_back( Point3D( BZWParser::value( "outside", line ) ) );
		}
		else if ( key == "vertex" ) {
			vertices.push_back( Point3D( BZWParser::value( "vertex", line ) ) );
		}
		else if ( key == "normal" ) {
			normals.push_back( Point3D( BZWParser::value( "normal", line ) ) );
		}
		else if ( key == "texcoord" ) {
			texCoords.push_back( Point2D( BZWParser::value( "texcoord", line ) ) );
		}
		else if ( key == "phydrv" ) {
			string drvname = BZWParser::value( "phydrv", line );
			physics* phys = (physics*)Model::command( MODEL_GET, "phydrv", drvname.c_str() );
			if (phys != NULL)
				phydrv = phys;
			else
				printf("mesh::update(): Error! Couldn't find physics driver %s\n", drvname.c_str());
		}
		else if ( key == "smoothbounce" ) {
			smoothbounce = true;
		}
		else if ( key == "noclusters" ) {
			noClusters = true;
		}
		else if ( key == "decorative" ) {
			decorative = true;
		}
		else if ( key == "drawinfo" ) {
			if (drawInfo != NULL) {
				printf("mesh::update(): Warning! Multiple drawinfo sections, using first.\n");
			}
			else {
				drawInfo = new DrawInfo();
				if ( !drawInfo->parse( itr ) ) {
					printf("mesh::update(): Error! Invalid drawInfo.\n");
					delete drawInfo;
					return 0;
				}
			}
		}
		else if ( key == "matref" ) {
			string matname = BZWParser::value( "matref", line );
			material* matref = (material*)Model::command( MODEL_GET, "material", matname );
			if (matref != NULL)
				mat = matref;
			else
				printf("mesh::update(): Error! Couldn't find physics driver %s\n", matname.c_str());
		}
		else {
			printf("mesh::update(): Warning! Unrecognized key: %s\n", key.c_str());
		}
	}
	
	
	
	// get materials in the order they come in relation to faces.
	// since we know how many faces there are in faceVals, we can
	// deduce which materials refer to which faces by counting the number
	// of faces between matrefs and later mapping those faces in faceVals
	// to those materials since in both matrefs and faceVals the faces
	// come in the same order.
	/*vector<string> matFaceKeys = vector<string>();
	matFaceKeys.push_back("face");
	matFaceKeys.push_back("matref");
	vector<string> matrefs = BZWParser::getLinesByKeys(matFaceKeys, header, meshDataNoDrawInfo);
	
	// get drawinfo
	vector<string> drawInfoVals = BZWParser::getSectionsByHeader("drawinfo", meshData);
	if(drawInfoVals.size() > 1) {
		printf("mesh::update(): Error! Defined \"drawinfo\" %d times\n", (int)drawInfoVals.size());
		return 0;
	}
		
	// get noclusters
	vector<string> noClusterVals = BZWParser::getValuesByKey("noclusters", header, meshData);
	
	// get smoothbounce
	vector<string> smoothBounceVals = BZWParser::getValuesByKey("smoothbounce", header, meshData);
	
	// parse faces
	vector<MeshFace> faceParams = vector<MeshFace>();
	if(faceVals.size() > 0) {
		for(vector<string>::iterator i = faceVals.begin(); i != faceVals.end(); i++) {
			faceParams.push_back( MeshFace(*i) );	
		}	
	}
	
	// parse material map.
	// deduce which faces map to which materials by counting the number of faces between
	// matrefs and mapping faces from faceParams to materials
	map<string, string> matmap = map<string, string>();
	if(matrefs.size() > 0 && faceParams.size() > 0) {
		string activeMaterial = "";		// the current material that faces will be mapped to
		vector<string> lineElements;	// temporary container for elements of each matrefs line
		unsigned int faceParamsIndex = 0;		// used to index faceParams to map its elements to materials
		for(vector<string>::iterator i = matrefs.begin(); i != matrefs.end(); i++) {
			// get the line elements
			lineElements = BZWParser::getLineElements(i->c_str());
			if(lineElements.size() == 0)
				continue;
			
			// see if we came across a face
			if(lineElements[0] == "face") {
				// if so, then map the face to activeMaterial
				matmap[ faceParams[faceParamsIndex].toString() ] = string(activeMaterial);
				faceParamsIndex++;
			}
			// see if we came across a valid matref
			else if(lineElements[0] == "matref") {
				// if so, then reset the active material
				if(lineElements.size() > 1)
					activeMaterial = lineElements[1];
				else
					activeMaterial = "-1";
				
			}
			
			// break if there are no more faces to map
			if(faceParamsIndex >= faceParams.size())
				break;
		}
	}
	
	// parse drawinfo
	DrawInfo drawInfoParam = DrawInfo();
	bool doDrawInfo = false;
	if(drawInfoVals[0] != BZW_NOT_FOUND) {
		drawInfoParam = DrawInfo(drawInfoVals[0]);
		doDrawInfo = true;
	}
	
	// parse the values
	vector<Point3D> vertexData = vector<Point3D>();
	vector<Vector3D> vectorData = vector<Vector3D>();
	vector<TexCoord2D> texCoordData = vector<TexCoord2D>();
	vector<Point3D> insideData = vector<Point3D>();
	vector<Point3D> outsideData = vector<Point3D>();
	
	// validate the vertexes (i.e. make sure they all have 3 values)
	if(vertexVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = vertexVals.begin(); i != vertexVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::update(): Error! \"vertex\" in \"vertex %s\" needs 3 values!\n", i->c_str());
				return 0;
			}
			vertexData.push_back( Point3D( i->c_str() ) );
		}
	}
	
	// validate the normals (i.e. make sure they all have 3 values)
	if(normalVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = normalVals.begin(); i != normalVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::update(): Error! \"normal\" in \"normal %s\" needs 3 values!\n", i->c_str());
				return 0;
			}
			vectorData.push_back( Vector3D( i->c_str() ) );
		}
	}
	
	// validate the texture coordinates (i.e. make sure they all have 2 values)
	if(vertexVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = texCoordVals.begin(); i != texCoordVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 2) {
				printf("mesh::update(): Error! \"texcoord\" in \"texcoord %s\" needs 2 values!\n", i->c_str());
				return 0;
			}
			texCoordData.push_back( TexCoord2D( i->c_str() ) );
		}
	}
	
	// validate the inside points
	if(insidePointVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = insidePointVals.begin(); i != insidePointVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::update(): Error! \"inside\" in \"inside %s\" needs 1 value!\n", i->c_str());
				return 0;	
			}
			insideData.push_back( Point3D( i->c_str() ) );
		}	
	}
	
	// validate the outside points
	if(outsidePointVals.size() > 0) {
		vector<string> tmp;
		for(vector<string>::iterator i = outsidePointVals.begin(); i != outsidePointVals.end(); i++) {
			tmp = BZWParser::getLineElements( i->c_str() );
			if(tmp.size() != 3) {
				printf("mesh::update(): Error! \"outside\" in \"outside %s\" needs 1 value!\n", i->c_str());
				return 0;	
			}
			outsideData.push_back( Point3D( i->c_str() ) );
		}	
	}
	
	// do base-class update
	string bz2object_data = string(meshDataNoSubobjects);
	if(!bz2object::update(bz2object_data)) {
		printf("mesh::update(): Error! could not do bz2object update!\n");
		return 0;
	}
		
	// load data in
	vertices = vertexData;
	texCoords = texCoordData;
	normals = vectorData;
	drawInfo = drawInfoParam;
	useDrawInfo = doDrawInfo;
	insidePoints = insideData;
	outsidePoints = outsideData;
	faces = faceParams;
	noClusters = (noClusterVals.size() == 0 ? false : true);
	smoothBounce = (smoothBounceVals.size() == 0 ? false : true);
	materialMap = matmap;*/
	
	return 1;
}

// to string
string mesh::toString(void) {
	// string-ify the vertices, normals, texcoords, inside points, outside points, and faces
	/*string vertexString(""), normalString(""), texcoordString(""), insideString(""), outsideString(""), faceString("");
	
	if(vertices.size() > 0) {
		for(vector<Point3D>::iterator i = vertices.begin(); i != vertices.end(); i++) {
			vertexString += "  vertex " + i->toString();
		}		
	}
	
	if(normals.size() > 0) {
		for(vector<Vector3D>::iterator i = normals.begin(); i != normals.end(); i++) {
			normalString += "  normal " + i->toString();
		}		
	}
	
	if(texCoords.size() > 0) {
		for(vector<TexCoord2D>::iterator i = texCoords.begin(); i != texCoords.end(); i++) {
			texcoordString += "  texcoord " + i->toString() + "\n";	
		}
	}
	
	// special instance:
	// make sure to keep the order of faces and materials constant
	if(faces.size() > 0) {
		string currMaterial = "";
		for(vector<MeshFace>::iterator i = faces.begin(); i != faces.end(); i++) {
			// see if we came across a new material
			if(currMaterial != materialMap[ i->toString() ]) {
				// if so, set the current material and append the matref to the faceString
				currMaterial = materialMap[ i->toString() ];
				faceString += "  matref " + currMaterial + "\n";
			}
			faceString += "  " + i->toString();	
		}	
	}
	
	if(insidePoints.size() > 0) {
		for(vector<Point3D>::iterator i = insidePoints.begin(); i != insidePoints.end(); i++) {
			insideString += "  inside " + i->toString();
		}		
	}
	
	if(outsidePoints.size() > 0) {
		for(vector<Point3D>::iterator i = outsidePoints.begin(); i != outsidePoints.end(); i++) {
			outsideString += "  outside " + i->toString();
		}		
	}
	
	return string("mesh\n") +
				  (noClusters == true ? "  noclusters\n" : "") +
				  (smoothBounce == true ? "  smoothbounce\n" : "") +
				  insideString +
				  outsideString +
				  vertexString +
				  normalString +
				  texcoordString +
				  faceString +
				  (useDrawInfo == true ? "  " + drawInfo.toString() : "") + "\n" +
				  "end\n";*/

	return string(); // FIXME: correct code
}

// render
int mesh::render(void) {
	return 0;
}

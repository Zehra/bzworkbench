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

#include "model/Model.h"
#include "windows/View.h"

#include "model/BZWParser.h"

#include "DataEntry.h"

#include "objects/arc.h"
#include "objects/base.h"
#include "objects/box.h"
#include "objects/cone.h"
#include "objects/define.h"
#include "objects/dynamicColor.h"
#include "objects/group.h"
#include "objects/info.h"
#include "objects/link.h"
#include "objects/material.h"
#include "objects/mesh.h"
#include "objects/options.h"
#include "objects/physics.h"
#include "objects/pyramid.h"
#include "objects/sphere.h"
#include "objects/teleporter.h"
#include "objects/tetra.h"
#include "objects/texturematrix.h"
#include "objects/waterLevel.h"
#include "objects/weapon.h"
#include "objects/world.h"
#include "objects/zone.h"

#include "objects/bz2object.h"
#include <FL/Fl_Progress.H>

using namespace std;

Model* Model::modRef;

Model::Model() : Observable()
{

	this->worldData = new world();
	this->optionsData = new options();
	this->waterLevelData = new waterLevel();
	this->infoData = new info();

	// make a default material
	this->defaultMaterial = new material();
	this->defaultMaterial->setAmbient( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );
	this->defaultMaterial->setDiffuse( osg::Vec4( 1, 1.0, 1.0, 1.0 ) );
	this->defaultMaterial->setSpecular( osg::Vec4( 0.0, 0.0, 0.0, 0.0) );
	this->defaultMaterial->setEmission( osg::Vec4( 0.0, 0.0, 0.0, 1.0) );

	this->defaultMaterial->setTexture( "tetrawall" );

	this->objects = objRefList();
	modRef = this;

	// this->cmap = map<string, DataEntry* (*)(string&)>();

	this->supportedObjects = string("");
	this->objectHierarchy = string("");
	this->objectTerminators = string("");

	this->unusedData = vector<string>();

}

// constructor that takes information about which objects to support
Model::Model(const char* _supportedObjects, const char* _objectHierarchy, const char* _objectTerminators) : 
	Observable() 
{
	this->worldData = new world();
	this->optionsData = new options();
	this->waterLevelData = new waterLevel();
	this->infoData = new info();

	// make a default material
	this->defaultMaterial = new material();
	this->defaultMaterial->setAmbient( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );
	this->defaultMaterial->setDiffuse( osg::Vec4( 1, 1.0, 1.0, 1.0 ) );
	this->defaultMaterial->setSpecular( osg::Vec4( 0.0, 0.0, 0.0, 0.0) );
	this->defaultMaterial->setEmission( osg::Vec4( 0.0, 0.0, 0.0, 1.0) );

	this->defaultMaterial->setTexture( "tetrawall" );

	this->objects = objRefList();
	modRef = this;

	// this->cmap = map<string, DataEntry* (*)(string&)>();

	this->supportedObjects = _supportedObjects;
	this->objectHierarchy = _objectHierarchy;
	this->objectTerminators = _objectTerminators;

	this->unusedData = vector<string>();
}


Model::~Model()
{

	if(worldData)
		delete worldData;

	if(optionsData)
		delete optionsData;

	if(waterLevelData)
		delete waterLevelData;

	if(infoData)
		delete infoData;

	if(objects.size() > 0)
		for(Model::objRefList::iterator i = objects.begin(); i != objects.end(); ++i)
			if((i->get())) {
				*i = NULL;	// this will deref the object, calling a destructor
			}

	if(groups.size() > 0)
		for(map<string, define*>::iterator i = groups.begin(); i != groups.end(); ++i)
			if((i->second)) {
				delete i->second;
				i->second = NULL;
			}

}

// getters specific to the model
const string Model::getSupportedObjects() { return modRef->_getSupportedObjects(); }
const string Model::_getSupportedObjects() { return supportedObjects; }

const string Model::getSupportedHierarchies() { return modRef->_getSupportedHierarchies(); }
const string Model::_getSupportedHierarchies() { return objectHierarchy; }

const string Model::getSupportedTerminators() { return modRef->_getSupportedTerminators(); }
const string Model::_getSupportedTerminators() { return objectTerminators; }

// the query method
DataEntry* Model::command(const string& command, const string& object, const string& name, const string& data) {
	return modRef->_command(command, object, name, data);
}

// the *real* query method
// TODO: add more features
DataEntry* Model::_command(const string& _command, const string& object, const string& name, const string& _data) {
	// see if this is a "get" command
	if( _command == MODEL_GET ) {
		// determine which type of object

		// handle dynamicColor
		if( object == "dynamicColor" ) {
			return ( this->dynamicColors.count( name ) > 0 ? this->dynamicColors[name] : NULL );
		}

		// handle texturematrices
		else if( object == "texturematrix" ) {
			return ( this->textureMatrices.count( name ) > 0 ? this->textureMatrices[name] : NULL );
		}

		// handle physics drivers
		else if( object == "phydrv" ) {
			return ( this->phys.count( name ) > 0 ? this->phys[name].get() : NULL );
		}

		// handle materials
		else if( object == "material" ) {
			// materials can also be reference by name or index
			// look for match by name
			DataEntry* obj = (this->materials.count( name ) > 0 ? this->materials[name].get() : NULL );
			// if no name match, then convert name to a number and try to get material by index
			if(obj == NULL){
				int index = atoi( name.c_str() );
				//printf("material index = %i\n material size: %i\n ", index, materials.size());
				int c = 0;
				for ( map<string, osg::ref_ptr< material > >::const_iterator it = materials.begin(); it != materials.end(); it++, c++ ) {
					if(c == index){
						//printf("found: %i key = %s\n", c, it->first.c_str());
						obj = it->second.get();
						break;
					}
				}
			}
			
			return obj;
		}

		// handle teleporter links
		else if( object == "link" ) {
			return (this->links.count( name ) > 0 ? this->links[name].get() : NULL );
		}

		// handle definitions
		else if( object == "define" ) {
			return (this->groups.count( name ) > 0 ? this->groups[name] : NULL );
		}

		// handle all other objects
		else {
			for( objRefList::iterator i = this->objects.begin(); i != this->objects.end(); i++) {
				if( (*i)->getName() == name )
					return i->get();
			}
		}
	}

	return NULL;
}

// the static build method
bool Model::build( std::istream& data ) { return modRef->_build(data); }

bool Model::_build( std::istream& data ) {
	//clear errors
	errors = "";
	int oc = 0;
	_newWorld();

	string buff, header;
	int lineCount = 0;

	world* worldObj = NULL;
	waterLevel* waterLevelObj = NULL;
	options* optionsObj = NULL;
	info* infoObj = NULL;
	material* materialObj = NULL;
	physics* physicsObj = NULL;
	dynamicColor* dyncolObj = NULL;
	define* defineObj = NULL;
	Tlink* linkObj = NULL;
	texturematrix* texmatObj = NULL;
	bz2object* object = NULL;
	
	// get length of file:
	data.seekg (0, ios::end);
	int filelength = data.tellg();
	data.seekg (0, ios::beg);
	int amountParsed = 0;
	string lastObj;
	//setup progress bar window
	Fl_Window* progressWin = new Fl_Window(320,90, "Loading BZW File");
	progressWin->begin();                         // add progress bar to it..
	Fl_Box* pbox = new Fl_Box(FL_FLAT_BOX,10,20,300,30,"...");
	pbox->align(FL_ALIGN_LEFT | FL_ALIGN_TOP| FL_ALIGN_INSIDE| FL_ALIGN_CLIP | FL_ALIGN_WRAP);
	pbox->labelfont(FL_BOLD);
	pbox->labelsize(12);
    Fl_Progress* progress = new Fl_Progress(10,50,300,30);
    progress->minimum(0);               // set progress bar attribs..
    progress->maximum(filelength);
    progressWin->end();                           // end of adding to window
	
	//show progress
	progress->value(0);
	progressWin->set_modal();
	progressWin->show();

	while(!data.eof()) {
		// read in lines until we find a key
		getline( data, buff );
		amountParsed +=	buff.length();
		lineCount++;
		buff = BZWParser::cutWhiteSpace( buff );
		header = BZWParser::key( buff.c_str() );

		try {
			if ( header == "" )
				continue; // if the line is blank move to the next one

			// first check if we are currently parsing an object
			else if ( worldObj ) {
				if ( !worldObj->parse( buff ) ) {
					worldObj->finalize();
					this->worldData = worldObj;
					worldObj = NULL;

					// tell the observers we have a different world
					ObserverMessage obs( ObserverMessage::UPDATE_WORLD, worldData );
					this->notifyObservers( &obs );
				}
			}
			else if ( waterLevelObj ) {
				if ( !waterLevelObj->parse( buff ) ) {
					waterLevelObj->finalize();
					this->waterLevelData = waterLevelObj;
					waterLevelObj = NULL;

					// tell the observers we have a different world
					ObserverMessage obs( ObserverMessage::UPDATE_WORLD, worldData );
					this->notifyObservers( &obs );
				}
			}
			else if ( optionsObj ) {
				if ( !optionsObj->parse( buff ) ) {
					optionsObj->finalize();
					this->optionsData = optionsObj;
					optionsObj = NULL;
				}
			}
			else if ( infoObj ) {
				if ( !infoObj->parse( buff ) ) {
					infoObj->finalize();
					this->infoData = infoObj;
					infoObj = NULL;
				}
			}
			else if ( materialObj ) {
				if ( !materialObj->parse( buff ) ) {
					materialObj->finalize();
					this->materials[ materialObj->getName() ] = materialObj;
					materialObj = NULL;
				}
			}
			else if ( physicsObj ) {
				if ( !physicsObj->parse( buff ) ) {
					physicsObj->finalize();
					this->phys[ physicsObj->getName() ] = physicsObj;
					physicsObj = NULL;
				}
			}
			else if ( dyncolObj ) {
				if ( !dyncolObj->parse( buff ) ) {
					dyncolObj->finalize();
					this->dynamicColors[ dyncolObj->getName() ] = dyncolObj;
					dyncolObj = NULL;
				}
			}
			else if ( defineObj ) {
				if ( !defineObj->parse( buff ) ) {
					defineObj->finalize();
					this->groups[ defineObj->getName() ] = defineObj;
					defineObj = NULL;
				}
			}
			else if ( linkObj ) {
				if ( !linkObj->parse( buff ) ) {
					linkObj->finalize();
					this->links[ linkObj->getName() ] = linkObj;
					linkObj = NULL;
				}
			}
			else if ( texmatObj ) {
				if ( !texmatObj->parse( buff ) ) {
					texmatObj->finalize();
					this->textureMatrices[ texmatObj->getName() ] = texmatObj;
					texmatObj = NULL;
				}
			}
			else if ( object ) {
				if ( !object->parse( buff ) ) {
					object->finalize();
					_addObject( object );
					object = NULL;
				}
			}

			// find what object to parse based on its header
			else if( header == "world" ) {
				lastObj = header;
				worldObj = (world*)this->cmap["world"]();
			}
			else if( header == "waterlevel" ) {
				lastObj = header;
				waterLevelObj = (waterLevel*)this->cmap["waterLevel"]();
			}
			else if( header == "options" ) {
				lastObj = header;
				optionsObj = (options*)this->cmap["options"]();
			}
			else if( header == "info" ) {
				lastObj = header;
				infoObj = (info*)this->cmap["info"]();
			}
			else if( header == "material" ) {
				lastObj = header;
				materialObj = (material*)this->cmap["material"]();
			}
			else if( header == "physics" ) {
				lastObj = header;
				physicsObj = (physics*)this->cmap["physics"]();
			}
			else if( header == "dynamiccolor" ) {
				lastObj = header;
				dyncolObj = (dynamicColor*)this->cmap["dynamicColor"]();
			}
			else if( header == "define" ) {
				lastObj = header;
				defineObj = (define*)this->cmap["define"]();
				defineObj->parse(buff);
			}
			else if( header == "link" ) {
				lastObj = header;
				linkObj = (Tlink*)this->cmap["link"]();
			}
			else if( header == "texturematrix" ) {
				lastObj = header;
				texmatObj = (texturematrix*)this->cmap["texturematrix"]();
			}
			else {
				if( this->cmap.count(header) > 0 ) {
					lastObj = header;
					object = (bz2object*)cmap[header]();
					object->parse( buff );
				}
				else {
					BZWReadError err = BZWReadError(NULL,"Model::build(): Skipping undefined object \"" + buff + "\"", lineCount);
					appendError(err);
					//printf("Model::build(): Skipping undefined object \"%s\"\n", buff.c_str());
					//this->unusedData.push_back( buff );
				}
			}
		}
		catch ( BZWReadError err ) { // catch any read errors
			err.line = lineCount;
			appendError(err);
		}
		// update progress bar
		if(header == "end"){ //limit how often progress is updated
			oc++;
			if(oc == 10){// limit to every 10 objects
				oc = 0;
				progress->value(amountParsed);
				float percentage = ((float)amountParsed/(float)filelength)*100;
				printf("%f\n", percentage);
				string progressLabel = itoa((int)percentage) + "%";
				progress->label( progressLabel.c_str() );
				string progressText = "Processed: " + lastObj;
				pbox->label(progressText.c_str());
				Fl::check();
			}
		}
	}

	// need a world so if we didn't find one make a default one
	if (!worldData)
		worldData = new world();
	
	//cleanup progress bar window
	progress->value(filelength);
	progress->label( "100%" );
	Fl::wait(0.22);
	progressWin->hide();
	delete(progress);
	delete(progressWin);
	
	// return false to report errors
	if(errors.length() > 0)
		return false;
	
	return true;
}

// BZWB-specific API
world* Model::getWorldData() { return modRef->_getWorldData(); }
options* Model::getOptionsData() { return modRef->_getOptionsData(); }
waterLevel* Model::getWaterLevelData() { return modRef->_getWaterLevelData(); }
info* Model::getInfoData() { return modRef->_getInfoData(); }

// plug-in specific API

/**
 * Register an object with the model.  It maps the object's header string with a function that will initialize the object
 * (usually, this is the static init() method in the built-in objects).
 * Returns true of the object was added; false if there's something already registered with that name
 */
bool Model::registerObject(string& name, DataEntry* (*init)()) { return modRef->_registerObject(name, init); }
bool Model::registerObject(const char* name, const char* hierarchy, const char* terminator, DataEntry* (*init)(), ConfigurationDialog* (*config)(DataEntry*))
	{ return modRef->_registerObject(name, hierarchy, terminator, init, config); }

bool Model::_registerObject(string& name, DataEntry* (*init)()) {
	return this->_registerObject( name.c_str(), "", "end", init, NULL);
}

bool Model::_registerObject(const char* _name, const char* _hierarchy, const char* _terminator, DataEntry* (*init)(), ConfigurationDialog* (*config)(DataEntry*)) {
	// catch NULLs
	if(_name == NULL)
		return false;
	if(_terminator == NULL)
		_terminator = "end";

	string name = _name;
	string hierarchy = (_hierarchy != NULL ? _hierarchy : "");
	string terminator = _terminator;

	// add support for this object
	if(!this->_addObjectSupport( _name ))
		return false;

	// add support for this terminator
	if(!this->_addTerminatorSupport( _name, _terminator ))
		return false;

	// add support for this hierarchy
	this->_addSupportedHierarchy( _hierarchy );

	// add the initializer for this object
	this->cmap[ name ] = init;

	// add the configuration dialog for this object
	this->configMap[ name ] = config;

	return true;

}

// is an object supported?
bool Model::isSupportedObject(const char* name) { return modRef->_isSupportedObject(name); }
bool Model::_isSupportedObject( const char* name ) {
	if( name == NULL || strlen(name) == 0)
		return false;

	return (this->supportedObjects.find( string("<") + name + ">", 0 ) != string::npos);
}

// is the terminator supported?
bool Model::isSupportedTerminator(const char* name, const char* end) { return modRef->_isSupportedTerminator(name, end); }
bool Model::_isSupportedTerminator( const char* name, const char* end ) {
	if((name == NULL || strlen(name) == 0) || (end == NULL || strlen(end) == 0))
		return false;

	return (this->objectTerminators.find( string("<") + name + "|" + end + ">", 0 ) != string::npos);
}

// is the hierarchy supported?
bool Model::isSupportedHierarchy(const char* name) { return modRef->_isSupportedHierarchy(name); }
bool Model::_isSupportedHierarchy( const char* name ) {
	if(name == NULL || strlen(name) == 0)
		return false;

	return (this->objectHierarchy.find( name, 0 ) != string::npos);
}

/**************************
 *
 * methods to manipulate Model::supportedObjects, Model::objectTerminators, and Model::objectHierarchy
 *
 **************************/

// add the name of an object to the list of supported objects; return false if it's already there
bool Model::addObjectSupport(const char* name) { return modRef->_addObjectSupport(name); }
bool Model::_addObjectSupport(const char* name) {
	if(name == NULL || this->_isSupportedObject( name ))
		return false;

	this->supportedObjects += string("<") + name + ">";
	return true;
}

// remove the name of an object from the list of supported objects; return false if not found
bool Model::removeObjectSupport(const char* name) { return modRef->_removeObjectSupport(name); }
bool Model::_removeObjectSupport(const char* name) {
	if(!this->_isSupportedObject( name ))
		return false;

	// get the start of the object
	string::size_type index = this->supportedObjects.find( string("<") + name + ">", 0 );

	// get the end of the object
	string::size_type end = this->supportedObjects.find( ">", index+1 );

	// cut out the element
	string frontChunk = "", endChunk = "";

	if(index > 0)
		frontChunk = this->supportedObjects.substr(0, index);

	if(end < supportedObjects.size() - 1)
		endChunk = this->supportedObjects.substr(end + 1);

	// regroup the string
	this->supportedObjects = frontChunk + endChunk;

	return true;

}

// add support for an object hierarchy
bool Model::addSupportedHierarchy(const char* name) { return modRef->_addSupportedHierarchy(name); }
bool Model::_addSupportedHierarchy(const char* name) {
	if(name == NULL || _isSupportedHierarchy( name ))
		return false;

	this->objectHierarchy += name;
	return true;
}

// remove support for an object hierarchy
bool Model::removeSupportedHierarchy(const char* name) { return modRef->_removeSupportedHierarchy(name); }
bool Model::_removeSupportedHierarchy(const char* name) {
	if(!this->_isSupportedHierarchy( name ))
		return false;

	// get the start of the hierarchy
	string::size_type index = this->objectHierarchy.find( name, 0 );

	// get the end of the hierarchy
	string::size_type end = this->objectHierarchy.find( "<", index + strlen(name) );

	// cut out the element
	string frontChunk = "", endChunk = "";

	if(index > 0)
		frontChunk = this->objectHierarchy.substr(0, index);

	if(end < objectHierarchy.size() - 1)
		endChunk = this->objectHierarchy.substr(end + 1);

	// regroup the string
	this->objectHierarchy = frontChunk + endChunk;

	return true;
}


// add support for an object terminator
bool Model::addTerminatorSupport(const char* name, const char* end) { return modRef->_addTerminatorSupport(name, end); }
bool Model::_addTerminatorSupport(const char* name, const char* end) {
	if(name == NULL || this->_isSupportedTerminator( name, end ))
		return false;

	this->objectTerminators += string("<") + name + "|" + end + ">";
	return true;
}

// remove support for an object terminator
bool Model::removeTerminatorSupport(const char* name, const char* end) { return modRef->_removeTerminatorSupport(name, end); }
bool Model::_removeTerminatorSupport(const char* name, const char* end) {
	if(!this->_isSupportedTerminator(name, end))
		return false;

	string term = string(name) + "|" + string(end);

	// get the start of the object
	string::size_type index = this->objectTerminators.find( string("<") + term + ">", 0 );

	// get the end of the object
	string::size_type endIndex = this->objectTerminators.find( ">", index+1 );

	// cut out the element
	string frontChunk = "", endChunk = "";

	if(index > 0)
		frontChunk = this->objectTerminators.substr(0, index);

	if(endIndex < this->objectTerminators.size() - 1)
		endChunk = this->objectTerminators.substr(endIndex + 1);

	// regroup the string
	this->objectTerminators = frontChunk + endChunk;

	return true;
}

// the universal getter--returns the entire working model as a string of BZW-formatted text
string& Model::toString() { return modRef->_toString(); }
string& Model::_toString() {
	// iterate through all objects and have them print themselves out in string format
	static string ret = "";
	ret.clear();

	string worldDataString = (this->worldData != NULL ? this->worldData->toString() : "\n");
	string optionsDataString = (this->optionsData != NULL ? this->optionsData->toString() : "\n");
	string waterLevelString = (this->waterLevelData != NULL && this->waterLevelData->getHeight() > 0.0 ? this->waterLevelData->toString() : "\n");
	string infoString = (this->infoData != NULL ? this->infoData->toString() : "\n");

	// global data
	ret += "\n#--Info------------------------------------------\n\n" + infoString;
	ret += "\n#--World-----------------------------------------\n\n" + worldDataString;
	ret += "\n#--Options---------------------------------------\n\n" + optionsDataString;
	ret += "\n#--Water Level-----------------------------------\n\n" + waterLevelString;

	// physics drivers
	ret += "\n#--Physics Drivers-------------------------------\n\n";
	if(this->phys.size() > 0) {
		for(map< string, osg::ref_ptr< physics > >::iterator i = this->phys.begin(); i != this->phys.end(); i++) {
			ret += i->second->toString() + "\n";
		}
	}

	// dynamic colors
	ret += "\n#--Dynamic Colors--------------------------------\n\n";
	if(this->dynamicColors.size() > 0) {
		for(map< string, dynamicColor* >::iterator i = this->dynamicColors.begin(); i != this->dynamicColors.end(); i++) {
			ret += i->second->toString() + "\n";
		}
	}

	// texture matrices
	ret += "\n#--Texture Matrices------------------------------\n\n";
	if(this->textureMatrices.size() > 0) {
		for(map< string, texturematrix* >::iterator i = this->textureMatrices.begin(); i != this->textureMatrices.end(); i++) {
			ret += i->second->toString() + "\n";
		}
	}

	// materials
	ret += "\n#--Materials-------------------------------------\n\n";
	if(this->materials.size() > 0) {
		for(map< string, osg::ref_ptr< material > >::iterator i = this->materials.begin(); i != this->materials.end(); i++) {
			ret += i->second->toString() + "\n";
		}
	}

	// group defintions
	ret += "\n#--Group Definitions-----------------------------\n\n";
	if(this->groups.size() > 0) {
		for(map< string, define* >::iterator i = this->groups.begin(); i != this->groups.end(); i++) {
			ret += i->second->toString() + "\n";
		}
	}

	// all other objects
	ret += "\n#--Objects---------------------------------------\n\n";
	if(this->objects.size() > 0) {
		for(objRefList::iterator i = this->objects.begin(); i != this->objects.end(); i++) {
			ret += (*i)->toString() + "\n";
		}
	}

	// links
	ret += "\n#--Teleporter Links------------------------------\n\n";
	if(this->links.size() > 0) {
		for(map< string, osg::ref_ptr< Tlink > >::iterator i = this->links.begin(); i != this->links.end(); i++) {
			ret += i->second->toString() + "\n";
		}
	}

	// unused dats
	ret += "\n#--Unused Data-----------------------------------\n\n";
	if(this->unusedData.size() > 0) {
		for(vector<string>::iterator i = this->unusedData.begin(); i != this->unusedData.end(); i++) {
			ret += (*i) + "\n";
		}
	}

	return ret;
}

// BZWB-specific API
Model::objRefList& 				Model::getObjects() 		{ return modRef->_getObjects(); }
map< string, osg::ref_ptr< material > >& 		Model::getMaterials() 		{ return modRef->_getMaterials(); }
map< string, texturematrix* >&	Model::getTextureMatrices() { return modRef->_getTextureMatrices(); }
std::map< std::string, dynamicColor* >& Model::getDynamicColors() { return modRef->_getDynamicColors(); }
map< string, osg::ref_ptr< physics > >& 		Model::getPhysicsDrivers() 	{ return modRef->_getPhysicsDrivers(); }
map< string, osg::ref_ptr< Tlink > >&		 	Model::getTeleporterLinks() { return modRef->_getTeleporterLinks(); }
map< string, define* >&			Model::getGroups() 			{ return modRef->_getGroups(); }
void					Model::addObject( bz2object* obj ) { modRef->_addObject( obj ); }
void					Model::removeObject( bz2object* obj ) { modRef->_removeObject( obj ); }
void					Model::setSelected( bz2object* obj ) { modRef->_setSelected( obj ); }
void					Model::setUnselected( bz2object* obj ) { modRef->_setUnselected( obj ); }
void					Model::selectAll() { modRef->_selectAll(); }
void					Model::unselectAll() { modRef->_unselectAll(); }
bool					Model::isSelected( bz2object* obj ) { return modRef->_isSelected( obj ); }

// add an object to the Model
void Model::_addObject( bz2object* obj ) {
	if( obj == NULL )
		return;

	this->objects.push_back( obj );

	// tell all observers
	ObserverMessage obs( ObserverMessage::ADD_OBJECT, obj );
	this->notifyObservers( &obs );
}

// remove an object by instance
void Model::_removeObject( bz2object* obj ) {
	if(objects.size() <= 0)
		return;

	objRefList::iterator itr = objects.begin();
	for(unsigned int i = 0; i < this->objects.size() && itr != this->objects.end(); i++, itr++) {
		if( this->objects[i] == obj ) {

			ObserverMessage obs( ObserverMessage::REMOVE_OBJECT, obj );
			this->notifyObservers( &obs );
			objects.erase( itr );

			break;
		}
	}
}

void Model::_removeMaterial( material* mat ) {
	if (materials.size() <= 0)
		return;

	map< string, osg::ref_ptr< material > >::iterator i;
	for ( i = materials.begin(); i != materials.end(); i++ ) {
		if ( i->second == mat ) {

			UpdateMessage msg( UpdateMessage::REMOVE_MATERIAL, mat );
			// make sure all the objects are informed of the material being removed.
			for ( objRefList::iterator j = objects.begin(); j != objects.end(); j++ ) {
				(*j)->update( msg );
			}

			// make sure the material is removed from other materials too
			for ( map< string, osg::ref_ptr< material > >::iterator j = materials.begin(); j != materials.end(); j++ ) {
				if ( j->second != mat ) {
					j->second->removeMaterial( mat );
				}
			}

			materials.erase( i );
			break;
		}
	}
}

void Model::_removePhysicsDriver( physics* phydrv ) {
	if (phys.size() <= 0)
		return;

	map< string, osg::ref_ptr< physics > >::iterator i;
	for ( i = phys.begin(); i != phys.end(); i++ ) {
		if ( i->second == phydrv ) {

			UpdateMessage msg( UpdateMessage::REMOVE_PHYDRV, phydrv );
			// make sure all the objects are informed of the phydrv being removed.
			for ( objRefList::iterator j = objects.begin(); j != objects.end(); j++ ) {
				(*j)->update( msg );
			}

			phys.erase( i );
			break;
		}
	}
}

void Model::_removeTextureMatrix( texturematrix* texmat ) {
	if (textureMatrices.size() <= 0)
		return;

	map< string, texturematrix* >::iterator i;
	for ( i = textureMatrices.begin(); i != textureMatrices.end(); i++ ) {
		if ( i->second == texmat ) {

			// make sure the texture matrix is removed from any materials
			for ( map< string, osg::ref_ptr< material > >::iterator j = materials.begin(); j != materials.end(); j++ ) {
				for (int k = 0; k < j->second->getTextureCount(); k++ ) {
					if (j->second->getTextureMatrix( k ) == texmat) {
						j->second->setTextureMatrix( NULL );
					}
				}
			}

			textureMatrices.erase( i );
			delete texmat;
			break;
		}
	}
}

void Model::_removeDynamicColor( dynamicColor* dyncol ) {
	if (dynamicColors.size() <= 0)
		return;

	map< string, dynamicColor* >::iterator i;
	for ( i = dynamicColors.begin(); i != dynamicColors.end(); i++ ) {
		if ( i->second == dyncol ) {

			// make sure the dynamic color is removed from any materials
			for ( map< string, osg::ref_ptr< material > >::iterator j = materials.begin(); j != materials.end(); j++ ) {
				if (j->second->getDynamicColor() == dyncol) {
					j->second->setDynamicColor( NULL );
				}
			}

			dynamicColors.erase( i );
			delete dyncol;
			break;
		}
	}
}

void Model::_removeGroup( define* def ) {
	if (groups.size() <= 0)
		return;

	map< string, define* >::iterator i;
	for ( i = groups.begin(); i != groups.end(); i++ ) {
		if ( i->second == def ) {

			// make sure the define is removed from groups
			for ( objRefList::iterator j = objects.begin(); j != objects.end(); j++ ) {
				group* grp = dynamic_cast< group* >( j->get() );

				if ( grp != NULL && grp->getDefine() == def) {
					_removeObject( grp );
				}
			}

			groups.erase( i );
			delete def;
			break;
		}
	}
}

// set an object as selected and update it
void Model::_setSelected( bz2object* obj ) {
	if( selectedObjects.size() < 0 )
		return;

	for(objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		if( *i == obj ) {
			return;		// this object is already selected.
		}
	}

	obj->setSelected( true );
	obj->setChanged( true );

	this->selectedObjects.push_back( obj );

	// tell the view to mark this object as selected
	ObserverMessage obs_msg( ObserverMessage::UPDATE_OBJECT, obj );

	this->notifyObservers( &obs_msg );
}

// set an object as unselected and update it
void Model::_setUnselected( bz2object* obj ) {
	if( this->selectedObjects.size() < 0)
		return;

	for(objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		if( *i == obj ) {
			obj->setSelected( false );
			obj->setChanged( true );
			this->selectedObjects.erase(i);
			break;
		}
	}

	// tell the view to mark this object as unselected
	ObserverMessage obs_msg( ObserverMessage::UPDATE_OBJECT, obj );

	this->notifyObservers( &obs_msg );
}

// determine whether or not an object is selected
bool Model::_isSelected( bz2object* obj ) {
	if( this->selectedObjects.size() < 0)
		return false;

	if(obj == NULL)
		return false;

	for(objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		if( *i == obj ) {
			return true;
		}
	}

	return false;
}

// select all objects
void Model::_selectAll() {
	unselectAll();

	Model::objRefList objs = Model::getObjects();
	for ( Model::objRefList::iterator i = objs.begin(); i != objs.end(); i++ ) {
		setSelected( (*i).get() );
	}
}

// unselect all objects
void Model::_unselectAll() {
	if( this->selectedObjects.size() <= 0)
		return;

	for(objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		(*i)->setSelected( false );
		(*i)->setChanged( true );
		// tell the view to mark this object as unselected
		ObserverMessage obs_msg( ObserverMessage::UPDATE_OBJECT, i->get() );
		this->notifyObservers( &obs_msg );
	}

	selectedObjects.clear();
	this->notifyObservers( NULL );

}

// get selection
Model::objRefList& Model::getSelection() { return modRef->_getSelection(); }

// build an object from the object registry
DataEntry* Model::buildObject( const char* header ) { return modRef->_buildObject( header ); }
DataEntry* Model::_buildObject( const char* header ) {
	string name = string(header);

	if( this->cmap.find( name ) == this->cmap.end() )
		return NULL;

	return this->cmap[name]();
}

// cut objects from the scene
bool Model::cutSelection() { return modRef->_cutSelection(); }
bool Model::_cutSelection() {
	if( this->selectedObjects.size() <= 0)
		return false;

	this->objectBuffer.clear();

	// remove objects from the scene, but move them into the cut/copy buffer first so they're still referenced
	for( objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		this->objectBuffer.push_back( *i );
	}
	for( objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		this->_removeObject( i->get() );
	}

	this->selectedObjects.clear();
	this->notifyObservers( NULL );

	return true;
}

// copy objects from the scene
bool Model::copySelection() { return modRef->_copySelection(); }
bool Model::_copySelection() {
	if( this->selectedObjects.size() <= 0)
		return false;

	this->objectBuffer.clear();

	// copy objects into the object buffer.
	for( objRefList::iterator i = this->selectedObjects.begin(); i != this->selectedObjects.end(); i++) {
		this->objectBuffer.push_back( *i );
	}

	return true;
}

// paste the objectBuffer to the scene
bool Model::pasteSelection() { return modRef->_pasteSelection(); }
bool Model::_pasteSelection() {
	if( this->objectBuffer.size() <= 0)
		return false;


	this->_unselectAll();

	// paste objects into the scene
	// create new instances; don't pass references
	for( vector< osg::ref_ptr<bz2object> >::iterator i = this->objectBuffer.begin(); i != this->objectBuffer.end(); i++) {
		bz2object* obj = SceneBuilder::cloneBZObject( i->get() );
		if(!obj) {
			printf("error! could not create new instance of \"%s\"\n", (*i)->getHeader().c_str() );
			continue;
		}

		obj->setPos( obj->getPos() + osg::Vec3(10.0, 10.0, 0.0) );

		this->_addObject( obj );
		this->_setSelected( obj );
	}

	this->notifyObservers(NULL);

	return true;
}

// delete a selection
bool Model::deleteSelection() { return modRef->_deleteSelection(); }
bool Model::_deleteSelection() {
	if( this->selectedObjects.size() <= 0)
		return false;

	// remove objects from the scene WITHOUT first referencing it (i.e. this will ensure it gets deleted)
	Model::objRefList::iterator itr = this->selectedObjects.begin();
	for(; itr != this->selectedObjects.end(); ) {
		bz2object* obj = itr->get();
		this->_removeObject( obj );

		this->selectedObjects.erase( itr );
		itr = this->selectedObjects.begin();
	}

	this->selectedObjects.clear();

	this->notifyObservers(NULL);

	return true;
}

// make a new world
bool Model::newWorld() { return modRef->_newWorld(); }
bool Model::_newWorld() {
	// clear previous objects
	clear();

	// set new world data
	this->optionsData = new options();
	this->worldData = new world();
	this->waterLevelData = new waterLevel();
	this->infoData = new info();

	return true;
}

// assign a material and make sure the Model has a reference to it
void Model::assignMaterial( const string& matref, bz2object* obj ) { modRef->_assignMaterial( matref, obj ); }
void Model::assignMaterial( material* matref, bz2object* obj ) { modRef->_assignMaterial( matref, obj ); }

void Model::_assignMaterial( const string& matref, bz2object* obj ) {
	material* mat;

	// do we have this material?
	if( this->materials.count( matref ) > 0 )
		mat = this->materials[matref].get();	// then load it from our mapping
	else
		mat = this->defaultMaterial.get();	// otherwise, use the default material

	// give the material to the object (and it will update itself)
	UpdateMessage msg( UpdateMessage::UPDATE_MATERIAL, mat );
	obj->update( msg );
}

void Model::_assignMaterial( material* matref, bz2object* obj ) {

	// if the material reference was NULL, just use the normal default material
	if( matref == NULL ) {
		printf(" NULL material, using defaults\n" );
		SceneBuilder::assignBZMaterial( defaultMaterial.get(), obj );
		return;
	}

	// otherwise, make sure this material exists (if not, then add it)
	if( this->materials.count( matref->getName() ) == 0 )
		materials[ matref->getName() ] = matref;

	// give the material to the object (and it will update itself)
	UpdateMessage msg( UpdateMessage::UPDATE_MATERIAL, matref );
	obj->update( msg );
}

// link two teleporters together if they are not explicitly linked
// return TRUE if a new link was created; FALSE if it already exists
bool Model::linkTeleporters( teleporter* from, teleporter* to ) {
	return modRef->_linkTeleporters( from, to );
}

bool Model::_linkTeleporters( teleporter* from, teleporter* to ) {
	// make sure that we don't already have a link
	if( this->links.size() > 0 ) {
		for( map< string, osg::ref_ptr< Tlink > >::iterator i = this->links.begin(); i != this->links.end(); i++ ) {
			if( (i->second->getTo() == from && i->second->getFrom() == to) ||
				(i->second->getTo() == to && i->second->getFrom() == from) )
					return false;
		}
	}

	// no such link exists; create it
	Tlink* newLink = new Tlink();
	string newLinkName = SceneBuilder::makeUniqueName("link");

	newLink->setName( newLinkName );
	newLink->setFrom( from );
	newLink->setTo( to );

	printf("  linked %s to %s\n", from->getName().c_str(), to->getName().c_str() );

	// add the link to the database
	this->links[ newLinkName ] = newLink;

	// tell the view to add it
	// ObserverMessage msg( ObserverMessage::ADD_OBJECT, newLink );
	// notifyObservers( &msg );

	return true;
}

ConfigurationDialog* Model::configureObject( DataEntry* d) { return modRef->_configureObject( d ); }
// configure an object
ConfigurationDialog* Model::_configureObject( DataEntry* d ) {
	if( d == NULL )
		return NULL;

	if( this->configMap.count( d->getHeader() ) == 0 )
		return NULL;

	if( this->configMap[ d->getHeader() ] == NULL )
		return NULL;

	return this->configMap[ d->getHeader() ](d);
}

bool Model::renameMaterial( std::string oldName, std::string newName ) { return modRef->_renameMaterial( oldName, newName ); }
bool Model::_renameMaterial( std::string oldName, std::string newName ) {
	// first check for conflicts and find the material
	map< string, osg::ref_ptr< material > >::iterator matIter;
	material* mat = NULL;
	for ( map< string, osg::ref_ptr< material > >::iterator i = materials.begin(); i != materials.end(); i++ ) {
		if ( i->first == newName ) {
			printf( "Model::_renameMaterial(): Error! Cannot change %s to %s due to naming conflict.\n", oldName.c_str(), newName.c_str() );
			return false;
		}
		else if ( i->first == oldName ) {
			matIter = i;
			mat = i->second.get();
		}
	}

	if ( mat == NULL ) {
		printf( "Model::_renameMaterial(): Error! Could not find material to rename\n" );
		return false;
	}

	// check new name for validity
	for ( unsigned int i = 0; i < newName.size(); i++ ) {
		if ( !TextUtils::isAlphanumeric( newName[i] ) && !TextUtils::isPunctuation( newName[i] ) ) {
			printf( "Model::_renameMaterial(): Error! Invalid name.\n" );
			return false;
		}
	}

	// remove old entry and make new entry
	materials.erase( matIter );
	materials[ newName ] = mat;

	return true;
}

bool Model::renameDynamicColor( std::string oldName, std::string newName ) { return modRef->_renameDynamicColor( oldName, newName ); }
bool Model::_renameDynamicColor( std::string oldName, std::string newName ) {
	// first check for conflicts and find the material
	map< string, dynamicColor* >::iterator matIter;
	dynamicColor* mat = NULL;
	for ( map< string, dynamicColor* >::iterator i = dynamicColors.begin(); i != dynamicColors.end(); i++ ) {
		if ( i->first == newName ) {
			printf( "Model::_renameDynamicColor(): Error! Cannot change %s to %s due to naming conflict.\n", oldName.c_str(), newName.c_str() );
			return false;
		}
		else if ( i->first == oldName ) {
			matIter = i;
			mat = i->second;
		}
	}

	if ( mat == NULL ) {
		printf( "Model::_renameDynamicColor(): Error! Could not find dynamic color to rename\n" );
		return false;
	}

	// check new name for validity
	for ( unsigned int i = 0; i < newName.size(); i++ ) {
		if ( !TextUtils::isAlphanumeric( newName[i] ) && !TextUtils::isPunctuation( newName[i] ) ) {
			printf( "Model::_renameDynamicColor(): Error! Invalid name.\n" );
			return false;
		}
	}

	// remove old entry and make new entry
	dynamicColors.erase( matIter );
	dynamicColors[ newName ] = mat;

	return true;
}

bool Model::renameTextureMatrix( std::string oldName, std::string newName ) { return modRef->_renameTextureMatrix( oldName, newName ); }
bool Model::_renameTextureMatrix( std::string oldName, std::string newName ) {
	// first check for conflicts and find the material
	map< string, texturematrix* >::iterator matIter;
	texturematrix* mat = NULL;
	for ( map< string, texturematrix* >::iterator i = textureMatrices.begin(); i != textureMatrices.end(); i++ ) {
		if ( i->first == newName ) {
			printf( "Model::_renameTextureMatrix(): Error! Cannot change %s to %s due to naming conflict.\n", oldName.c_str(), newName.c_str() );
			return false;
		}
		else if ( i->first == oldName ) {
			matIter = i;
			mat = i->second;
		}
	}

	if ( mat == NULL ) {
		printf( "Model::_renameTextureMatrix(): Error! Could not find texture matrix to rename\n" );
		return false;
	}

	// check new name for validity
	for ( unsigned int i = 0; i < newName.size(); i++ ) {
		if ( !TextUtils::isAlphanumeric( newName[i] ) && !TextUtils::isPunctuation( newName[i] ) ) {
			printf( "Model::_renameTextureMatrix(): Error! Invalid name.\n" );
			return false;
		}
	}

	// remove old entry and make new entry
	textureMatrices.erase( matIter );
	textureMatrices[ newName ] = mat;

	return true;
}

bool Model::renamePhysicsDriver( std::string oldName, std::string newName ) { return modRef->_renamePhysicsDriver( oldName, newName ); }
bool Model::_renamePhysicsDriver( std::string oldName, std::string newName ) {
	// first check for conflicts and find the material
	map< string, osg::ref_ptr< physics > >::iterator matIter;
	physics* mat = NULL;
	for ( map< string, osg::ref_ptr< physics > >::iterator i = phys.begin(); i != phys.end(); i++ ) {
		if ( i->first == newName ) {
			printf( "Model::_renamePhysicsDriver(): Error! Cannot change %s to %s due to naming conflict.\n", oldName.c_str(), newName.c_str() );
			return false;
		}
		else if ( i->first == oldName ) {
			matIter = i;
			mat = i->second.get();
		}
	}

	if ( mat == NULL ) {
		printf( "Model::_renamePhysicsDriver(): Error! Could not find physics driver to rename\n" );
		return false;
	}

	// check new name for validity
	for ( unsigned int i = 0; i < newName.size(); i++ ) {
		if ( !TextUtils::isAlphanumeric( newName[i] ) && !TextUtils::isPunctuation( newName[i] ) ) {
			printf( "Model::_renamePhysicsDriver(): Error! Invalid name.\n" );
			return false;
		}
	}

	// remove old entry and make new entry
	phys.erase( matIter );
	phys[ newName ] = mat;

	return true;
}

bool Model::renameTeleporterLink( std::string oldName, std::string newName ) { return modRef->_renameTeleporterLink( oldName, newName ); }
bool Model::_renameTeleporterLink( std::string oldName, std::string newName ) {
	// first check for conflicts and find the material
	map< string, osg::ref_ptr< Tlink > >::iterator matIter;
	Tlink* mat = NULL;
	for ( map< string, osg::ref_ptr< Tlink > >::iterator i = links.begin(); i != links.end(); i++ ) {
		if ( i->first == newName ) {
			printf( "Model::_renameTeleporterLink(): Error! Cannot change %s to %s due to naming conflict.\n", oldName.c_str(), newName.c_str() );
			return false;
		}
		else if ( i->first == oldName ) {
			matIter = i;
			mat = i->second.get();
		}
	}

	if ( mat == NULL ) {
		printf( "Model::_renameTeleporterLink(): Error! Could not find teleporter link to rename\n" );
		return false;
	}

	// check new name for validity
	for ( unsigned int i = 0; i < newName.size(); i++ ) {
		if ( !TextUtils::isAlphanumeric( newName[i] ) && !TextUtils::isPunctuation( newName[i] ) ) {
			printf( "Model::_renameTeleporterLink(): Error! Invalid name.\n" );
			return false;
		}
	}

	// remove old entry and make new entry
	links.erase( matIter );
	links[ newName ] = mat;

	return true;
}

bool Model::renameGroup( std::string oldName, std::string newName ) { return modRef->_renameGroup( oldName, newName ); }
bool Model::_renameGroup( std::string oldName, std::string newName ) {
	// first check for conflicts and find the material
	map< string, define* >::iterator matIter;
	define* mat = NULL;
	for ( map< string, define* >::iterator i = groups.begin(); i != groups.end(); i++ ) {
		if ( i->first == newName ) {
			printf( "Model::_renameGroup(): Error! Cannot change %s to %s due to naming conflict.\n", oldName.c_str(), newName.c_str() );
			return false;
		}
		else if ( i->first == oldName ) {
			matIter = i;
			mat = i->second;
		}
	}

	if ( mat == NULL ) {
		printf( "Model::_renameGroup(): Error! Could not find define to rename\n" );
		return false;
	}

	// check new name for validity
	for ( unsigned int i = 0; i < newName.size(); i++ ) {
		if ( !TextUtils::isAlphanumeric( newName[i] ) && !TextUtils::isPunctuation( newName[i] ) ) {
			printf( "Model::_renameGroup(): Error! Invalid name.\n" );
			return false;
		}
	}

	// remove old entry and make new entry
	groups.erase( matIter );
	groups[ newName ] = mat;

	return true;
}

// group objects together
void Model::groupObjects( vector< osg::ref_ptr< bz2object > >& _objects ) { modRef->_groupObjects( _objects ); }
void Model::_groupObjects( vector< osg::ref_ptr< bz2object > >& _objects ) {
	// create a "define" object for these objects if one does not exist yet
	define* def = new define();
	if( !def || _objects.size() == 0 )
		return;

	// unselect all objects
	for( vector< osg::ref_ptr< bz2object > >::iterator i = _objects.begin(); i != _objects.end(); i++) {
		this->_setUnselected( i->get() );
	}

	string defName = SceneBuilder::makeUniqueName("define");

	// assign the objects
	def->setObjects( _objects );

	// set the name
	def->setName( defName );

	// make a group
	group* grp = new group();

	// assign it to the define (the group will re-name itself automatically)
	grp->setDefine( def );

	// add this group
	this->_addObject( grp );

	// add this definition
	groups[ defName ] = def;

	// remove all the objects within the passed vector (they are now part of the define)
	for( vector< osg::ref_ptr< bz2object > >::iterator i = _objects.begin(); i != _objects.end(); i++ ) {
		this->_removeObject( i->get() );
	}

	// set the group as selected
	this->_setSelected( grp );
}

// ungroup objects
void Model::ungroupObjects( group* g ) { modRef->_ungroupObjects( g ); }
void Model::_ungroupObjects( group* g ) {
	// get the objects from the group
	vector< osg::ref_ptr< bz2object > > objs = g->getDefine()->getObjects();

	// get the group's position so the objects can be translated to their current position in the group relative to the world
	osg::Vec3 p = g->getPos();

	if( objects.size() > 0 ) {
		for( vector< osg::ref_ptr< bz2object > >::iterator i = objs.begin(); i != objs.end(); i++ ) {
			(*i)->setPos( (*i)->getPos() + p );
			this->_addObject( i->get() );		// add the object
			this->_setSelected( i->get() );	// select the object
		}
	}

	// see if we need to remove the associated define
	bool noRefs = true;	// set to false if other groups are referencing this group's "define"
	define* def = g->getDefine();
	for( vector< osg::ref_ptr< bz2object > >::iterator i = this->objects.begin(); i != this->objects.end(); i++ ) {
		group* grp = dynamic_cast< group* > ( i->get() );
		if( !grp || grp == g)
			continue;

		// if the associated "define" has the same name as the define from this group, then we don't erase it
		if( grp->getDefine() == def ) {
			noRefs = false;
			break;
		}
	}

	// remove the group itself
	this->_removeObject( g );

	// if no references to the define were found, then remove this define
	if( noRefs ) {
		this->groups.erase( g->getDefine()->getName() );
	}

}

void Model::clear() {
	// clear materials
	this->materials.clear();

	// clear physics drivers
	this->phys.clear();

	// clear dynamic colors
	for (map< string, dynamicColor* >::iterator i = dynamicColors.begin(); i != dynamicColors.end(); i++ ) {
		if ( i->second != NULL )
			delete i->second;
	}
	this->dynamicColors.clear();

	// clear teleporter links
	this->links.clear();

	// clear texture matrices
	for (map< string, texturematrix* >::iterator i = textureMatrices.begin(); i != textureMatrices.end(); i++ ) {
		if ( i->second != NULL )
			delete i->second;
	}
	this->textureMatrices.clear();

	// clear out the previous objects
	this->_unselectAll();
	for (objRefList::iterator i = objects.begin(); i != objects.end(); i++ ) {
		ObserverMessage obs( ObserverMessage::REMOVE_OBJECT, i->get() );
		notifyObservers( &obs );
	}
	this->objects.clear();
	this->notifyObservers( NULL );

	if (worldData != NULL)
		delete worldData;
	worldData = NULL;

	if (optionsData != NULL)
		delete optionsData;
	optionsData = NULL;

	if (waterLevelData != NULL)
		delete waterLevelData;
	waterLevelData = NULL;

	if (infoData != NULL)
		delete infoData;
	infoData = NULL;
	
	//clear stateCache
	SceneBuilder::clearStateCache();
}

void Model::appendError( BZWReadError err ) {	
	if (err.bzobject != NULL)
		errors += err.bzobject->getHeader() + ": ";
	errors += err.message + " at line " + itoa( err.line ) + "\n";
}

std::string Model::getErrors() {
	if(errors.length() > 0)
		return "The following parse errors occured.\n\n" + errors;
	return "";
}



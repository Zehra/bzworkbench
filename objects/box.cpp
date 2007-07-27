#include "../include/objects/box.h"

// constructors
box::box() : bz2object("box", "<position><rotation><size>") {
	this->addChild( SceneBuilder::buildNode( "share/box/box.obj" ) );
	this->setName( SceneBuilder::nameNode("box") );
	
	this->setPosition( osg::Vec3(0.0, 0.0, 0.0) );
	this->setScale( osg::Vec3(10.0, 10.0, 10.0) );
	SceneBuilder::markUnselected( this );
}

// constructor with binary data
box::box( osg::Vec3 position, float rotation, osg::Vec3 scale ) : bz2object("box", "<position><rotation><size>") {
	this->addChild( SceneBuilder::buildNode( "share/box/box.obj" ) );
	this->setName( SceneBuilder::nameNode("box") );
	
	this->setPosition( position );
	this->setRotationZ( rotation );
	this->setScale( scale );
	SceneBuilder::markUnselected( this );
}

box::box(string& data) : bz2object("box", "<position><rotation><size>", data.c_str()) {
	this->addChild( SceneBuilder::buildNode("share/box/box.obj") );
	this->setName( SceneBuilder::nameNode("share/box/box.obj") );
	SceneBuilder::markUnselected( this );
	
	if( data.length() <= 1 ) {
		this->setPosition( osg::Vec3(0.0, 0.0, 0.0) );
		this->setScale( osg::Vec3(10.0, 10.0, 10.0) );
	}
	else 
		this->update(data);	
}

// nothing to destroy...
box::~box() { }

// getter
string box::get(void) {
	return this->toString();
}

// setter (string data)
int box::update(string& data) {
	return this->update( data );
}

// setter (with binary data)
int box::update(string& data, UpdateMessage& message) {
	
	// do the bz2object update
	int result = bz2object::update(data);
	
	// bail if the update failed
	if(result == 0) {
		printf(" update failure\n");
		return result;
	}
	
	// if we changed the scale, update the texture coordinates (i.e. the scale might have changed)
	// NOTE: it is expected that message.data will point to an osg::Vec3, which contains the scaling FACTOR
	if( message.type == UpdateMessage::SET_SCALE ) {
		
		// extract the scale factor from the message
		osg::Vec3* scaleFactor = (osg::Vec3*)message.data;
		
		// get the geometries from the box mesh
		GeometryExtractorVisitor geoExtractor = GeometryExtractorVisitor( this );
		vector< osg::Geometry* > geos = geoExtractor.getGeometries();
		
		// there should be 2 geometries (One Geometry makes up the walls, the other the floor/ceiling combo.)
		// if there isn't, then bail
		if( geos.size() != 2 ) {
			printf(" error! %d geometries (expected 2)\n", geos.size());
			return result;
		}
		
		// handle an X or Y scale if needed
		if( scaleFactor->x() != 0.0 || scaleFactor->y() != 0.0 ) {
			// the first geometry contains the primitives for the walls
			osg::Geometry::PrimitiveSetList wallPrimitives = geos[0]->getPrimitiveSetList();
		
			// get the geometric vertices
			osg::Vec3Array* vertexArray = dynamic_cast<osg::Vec3Array*>( geos[0]->getVertexArray() );
			if(!vertexArray) {
				printf("not a Vec3Array\n");
			}
			
			// get the texture coordinates (the box.obj model uses 2D texture coordinates, and has only one texture coordinate array)
			osg::Vec2Array* texCoords = dynamic_cast<osg::Vec2Array*> ( geos[0]->getTexCoordArray(0) );
			if(!texCoords) {
				printf("not a Vec2Array\n");
			}
			
			/*
			 * Basically, we're going to compute the normals for each of the faces, so we can compute the dot product
			 * between the scaleFactor and the normal, and then the angle between the two vectors.  The angle will
			 * be used to compute the scaling factor.
			 * 
			 * For example,
			 * (0 or pi) radians ==> scaleFactor || normal ==> no scaling; and
			 * pi/2 radians ==> scaleFactor _|_ normal ==> full scaling
			 * 
			 * NOTE: We know in advance here that OSG will load box.obj's data as triangle strips.
			 */
			
			if( vertexArray != NULL && texCoords != NULL ) {
				// iterate through the primitives
				for( osg::Geometry::PrimitiveSetList::iterator i = wallPrimitives.begin(); i != wallPrimitives.end(); i++) {
					// something's seriously wrong if these aren't triangle strips
					if( (*i)->getMode() != osg::PrimitiveSet::TRIANGLE_STRIP ) {
						printf(" error! encountered a mesh without triangle strips!\n");
						continue;
					}
					
					// indexes to points in the mesh that form a face
					unsigned int i1 = (*i)->index(0);
					unsigned int i2 = (*i)->index(1);
					unsigned int i3;
					
					// working copies of points
					osg::Vec3 p1, p2, p3;
					
					// working vectors
					osg::Vec3 v1, v2, n;
					
					// working dot product
					double dp;
					
					// working angle value (in radians)
					double angle;
					
					// keep track of indices to which texture coordinates we've scaled (i.e. no repeats)
					// NOTE: tmp is a reflexive map, used in place of a vector for it's faster insertion and searching capabilities
					map<unsigned int, unsigned int> tmp = map<unsigned int, unsigned int>();
					tmp[i1] = i1;
					tmp[i2] = i2;
					
					// iterate through the rest of the indexes to get the faces by indexing vertexes with the primitiveset elements
					for( unsigned int j = 2; j < (*i)->getNumIndices(); j++) {
						// assign the next index in the strip
						i3 = (*i)->index(j);
						
						// get the points
						p1 = (*vertexArray)[i1];
						p2 = (*vertexArray)[i2];
						p3 = (*vertexArray)[i3];
						
						// compute vectors from the face (the vectors will be along two of the edges)
						v1 = p2 - p1;
						v2 = p2 - p3;
						
						// compute one of the normals by finding the cross product between v1 and v2
						n = (v1 ^ v2);
						
						// compute the dot product between the normal and the scaleFactor
						dp = n * (*scaleFactor);
						
						// find the angle (in radians)
						// dp = |n| * |scaleFactor| * cos(angle)
						angle = acos( dp / ( n.length() * scaleFactor->length() ) );
						
						// no need to scale if the scaling is happening parallel to the face (i.e. angle is 0 or pi)
						if( fabs(angle) < 0.001 || fabs( fabs( angle ) - osg::PI ) < 0.001 )
							continue;
						
						// handle a scale in the X direction
						if( scaleFactor->x() != 0.0 ) {
							if( j == 2 ) {		// only do this if this is the first loop
								double x1 = (*texCoords)[i1].x();
								double y1 = (*texCoords)[i1].y();
								
								double x2 = (*texCoords)[i2].x();
								double y2 = (*texCoords)[i2].y();
								
								// multipliers to keep the texture in shape
								double multx1 = ( x1 != 0.0 ? 0.4 : 0.0 );
								double multx2 = ( x2 != 0.0 ? 0.4 : 0.0 );
								
								double multy1 = ( y1 != 0.0 ? 1.0 : 0.0 );
								double multy2 = ( y2 != 0.0 ? 1.0 : 0.0 );
								
								// reset the texture coordinates for the first two points
								(*texCoords)[i1].set( x1 + multx1 * ( scaleFactor->x() * sin(angle) ),
													  y1 + multy1 * ( scaleFactor->x() * cos(angle) ) );
													  
								(*texCoords)[i2].set( x2 + multx2 * ( scaleFactor->x() * sin(angle) ),
													  y2 + multy2 * ( scaleFactor->x() * cos(angle) ) );
								
							}
							
							// if we haven't touched this texture coordinate yet, then update it
							if( tmp.count(i3) == 0 ) {
								// reset the texture coordinates for the third point, and every subsequent point in the strip
								double x = (*texCoords)[i3].x(),
									   y = (*texCoords)[i3].y();
								
								double multx = ( x != 0.0 ? 0.4 : 0.0 );
								double multy = ( y != 0.0 ? 1.0 : 0.0 );
								
								(*texCoords)[i3].set( x + multx * (scaleFactor->x() * sin(angle) ),
													  y + multy * (scaleFactor->x() * cos(angle) ) );
													  
								tmp[i3] = i3;
							
							}		  
						}
						
						// handle a scale in the Y direction
						if( scaleFactor->y() != 0.0 ) {
							if( j == 2 ) {		// only do this if this is the first loop
								double x1 = (*texCoords)[i1].x();
								double y1 = (*texCoords)[i1].y();
								
								double x2 = (*texCoords)[i2].x();
								double y2 = (*texCoords)[i2].y();
								
								// multipliers to keep the texture in shape
								double multx1 = ( x1 != 0.0 ? 0.5 : 0.0 );
								double multx2 = ( x2 != 0.0 ? 0.5 : 0.0 );
								
								double multy1 = ( y1 != 0.0 ? 1.0 : 0.0 );
								double multy2 = ( y2 != 0.0 ? 1.0 : 0.0 );
								
								// reset the texture coordinates for the first two points
								(*texCoords)[i1].set( x1 + multx1 * ( scaleFactor->y() * sin(angle) ),
													  y1 + multy1 * ( scaleFactor->y() * cos(angle) ) );
													  
								(*texCoords)[i2].set( x2 + multx2 * ( scaleFactor->y() * sin(angle) ),
													  y2 + multy2 * ( scaleFactor->y() * cos(angle) ) );
								
							}
							
							// if we haven't touched this texture coordinate yet, then update it
							if( tmp.count( i3 ) == 0 ) {
								// reset the texture coordinates for the third point, and every subsequent point in the strip
								double x = (*texCoords)[i3].x(),
									   y = (*texCoords)[i3].y();
								
								double multx = ( x != 0.0 ? 0.5 : 0.0 );
								double multy = ( y != 0.0 ? 1.0 : 0.0 );
								
								(*texCoords)[i3].set( x + multx * (scaleFactor->y() * sin(angle) ),
													  y + multy * (scaleFactor->y() * cos(angle) ) );
													  
								tmp[i3] = i3;
													  
							}
						}
						
						
						// finally, advance the face indexes
						i1 = i2;
						i2 = i3;
					}
				}
			}
			
			// finally, tell OSG to rebuild the display list for this geometry
			geos[0]->dirtyDisplayList();
		}
		
		// handle a Z scale if needed
		if( scaleFactor->z() != 0.0 ) {
			// the second geometry contains the primitives for the ceiling/floor
			osg::Geometry::PrimitiveSetList floorCeilingPrimitives = geos[1]->getPrimitiveSetList();
			
		}
		
	}
	
	return 1;
}

// toString
string box::toString(void) {
	return string("box\n") +
				  this->BZWLines() +
				  "end\n";
}


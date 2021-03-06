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

#include "objects/sphere.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

const char* sphere::sideNames[MaterialCount] = { "edge", "bottom" };

sphere::sphere() :
bz2object("sphere", "<position><rotation><size><radius><flatshading><name><divisions><shift><shear><spin><scale><smoothbounce><phydrv><matref><hemisphere><drivethrough><shootthrough><passable><texsize>" ) {
	setDefaults();
}

void sphere::setDefaults() {
	osg::Group* group = new osg::Group();
	osg::Geode* outside = new osg::Geode();
	osg::Geode* bottom = new osg::Geode();
	group->addChild( outside );
	group->addChild( bottom );
	setThisNode( group );

	SceneBuilder::assignTexture( "boxwall", group->getChild( 0 ) );
	SceneBuilder::assignTexture( "roof", group->getChild( 1 ) );

	MaterialSlot mslotEdge;
	mslotEdge.defaultMaterial = group->getChild( 0 )->getStateSet();
	mslotEdge.node = group->getChild( 0 );
	MaterialSlot mslotBottom;
	mslotBottom.defaultMaterial = group->getChild( 1 )->getStateSet();
	mslotBottom.node = group->getChild( 1 );

	materialSlots[ string( sideNames[ 0 ] ) ] = mslotEdge;
	materialSlots[ string( sideNames[ 1 ] ) ] = mslotBottom;

	// define some basic values
	realSize = osg::Vec3( 10, 10, 10 );
	texsize.set( -4.0f, -4.0f );
	divisions = 16;
	hemisphere = false;

	updateGeometry();
}

// getter
string sphere::get(void) { return toString(); }

int sphere::update(UpdateMessage& message) {
	int result = bz2object::update( message );

	switch( message.type ) {
		case UpdateMessage::SET_POSITION: 	// handle a new position
			setPos( *(message.getAsPosition()) );
			break;

		case UpdateMessage::SET_POSITION_FACTOR:	// handle a translation
			setPos( getPos() + *(message.getAsPositionFactor()) );
			break;

		case UpdateMessage::SET_ROTATION:		// handle a new rotation
			setRotationZ( message.getAsRotation()->z() );
			break;

		case UpdateMessage::SET_ROTATION_FACTOR:	// handle an angular translation
			setRotationZ( getRotation().z() + message.getAsRotationFactor()->z() );
			break;

		case UpdateMessage::SET_SCALE:		// handle a new scale
			setSize( *(message.getAsScale()) );
			break;

		case UpdateMessage::SET_SCALE_FACTOR:	// handle a scaling factor
			setSize( getSize() + *(message.getAsScaleFactor()) );
			break;

		case UpdateMessage::REMOVE_MATERIAL: {
			material* mat = message.getAsMaterial();
			for ( int i = 0; i < MaterialCount; i++ ) 
				if ( ((osg::Group*)getThisNode())->getStateSet() == mat )
					((osg::Group*)getThisNode())->setStateSet( NULL );
			break;
		}

		default:	// unknown event; don't handle
			return result;
	}

	return 1;
}

// bzw methods
bool sphere::parse( std::string& line ) {
	string key = BZWParser::key( line.c_str() );
	string value = BZWParser::value( key.c_str(), line.c_str() );
	
	// check if we reached the end of the section
	if ( key == "end" )
		return false;

	// parse keys
	if ( key == "texsize" ) {
		texsize = Point2D( value.c_str() );
	}
	else if ( key == "divisions" ) {
		divisions = atof( value.c_str() );
	}
	else if ( key == "radius" ) {
		float r = atof( value.c_str() );
		realSize = Point3D( r, r, r );
	}
	else if ( key == "size" ) {
		realSize = Point3D( value.c_str() );
	}
	else if ( key == "hemisphere" ) {
		hemisphere = true;
	}
	else {
		return bz2object::parse( line );
	}

	return true;
}

void sphere::finalize() {
	updateGeometry();
	bz2object::finalize();
}

// toString
string sphere::toString(void) {
	return string("sphere\n") +
				  BZWLines( this ) +
				  "  divisions " + string(itoa(divisions)) + "\n" +
				  (flatShading == true ? "  flatshading\n" : "") +
				  (smoothbounce == true ? "  smoothbounce\n" : "") +
				  "end\n";
}

// render
int sphere::render(void) {
	return 0;
}

void sphere::setSize( osg::Vec3 newSize ) {
	realSize = newSize;
	updateGeometry();
}

osg::Vec3 sphere::getSize() {
	return realSize;
}

void sphere::setFlatshading( bool value ) {
	flatShading = value;
}

void sphere::setSmoothbounce( bool value ) {
	smoothbounce = value;
}

void sphere::setHemisphere( bool value ) {
	hemisphere = value;
}

void sphere::setDivisions( int value ) {
	divisions = value;
	updateGeometry();
}

void sphere::setTexsize( Point2D value ) {
	texsize = value;
	updateGeometry();
}


bool sphere::getFlatshading() {
	return flatShading;
}

bool sphere::getSmoothbounce() {
	return smoothbounce;
}

bool sphere::getHemisphere() {
	return hemisphere;
}

int sphere::getDivisions() {
	return divisions;
}

Point2D sphere::getTexsize() {
	return texsize;
}

void sphere::updateGeometry() {
	osg::Group* sphere = (osg::Group*)getThisNode();
	osg::Geode* outside = (osg::Geode*)sphere->getChild( 0 );
	osg::Geode* bottom = (osg::Geode*)sphere->getChild( 1 );

	if ( outside->getNumDrawables() > 0 )
		outside->removeDrawables( 0 );
	if ( bottom->getNumDrawables() > 0 )
		bottom->removeDrawables( 0 );

	int i, j, q;
	const float minSize = 1.0e-6f; // cheezy / lazy
	int factor = 2;

	// setup the multiplying factor
	if (hemisphere) {
		factor = 1;
	}

	// absolute the sizes
	osg::Vec3 sz( fabsf(getSize().x()), fabsf(getSize().y()), fabsf(getSize().z()) );

	// adjust the texture sizes
	osg::Vec2 texsz(texsize.x(), texsize.y());
	if (texsz.x() < 0.0f) {
		// unless you want to do elliptic integrals, here's
		// the Ramanujan approximation for the circumference
		// of an ellipse  (it will be rounded anyways)
		const float circ =
			(float)M_PI * ((3.0f * (sz.x() + sz.y())) -
			sqrtf ((sz.x() + (3.0f * sz.y())) * (sz.y() + (3.0f * sz.x()))));
		// make sure it's an integral number so that the edges line up
		texsz._v[0] = -floorf(circ / texsz.x());
	}
	if (texsz.y() < 0.0f) {
		texsz._v[1] = -((2.0f * sz.z()) / texsz.y());
	}

	// validity checking
	if ((divisions < 1) ||
		(texsz.x() < minSize) || (texsz.y() < minSize) ||
		(sz.x() < minSize) || (sz.y() < minSize) || (sz.z() < minSize)) {
		return;
	}

	// setup the coordinates
	osg::Vec3Array* vertices = new osg::Vec3Array();
	osg::Vec2Array* texcoords = new osg::Vec2Array();

	osg::Vec3 v;
	osg::Vec2 t;

	// the center vertices
	v.set( 0, 0, sz.z() );
	vertices->push_back(v);
	if (!hemisphere) {
		v._v[2] = -sz.z();
		vertices->push_back(v);
	}
	t.set( 0.5f, 1.0f );
	texcoords->push_back( t );
	if (!hemisphere) {
		t._v[1] = 0.0f;
		texcoords->push_back(t);
	}

	// the rest of the vertices
	for ( i = 0; i < divisions; i++ ) {
		for ( j = 0; j < (4 * (i + 1)); j++ ) {

			float h_angle = (float)((M_PI * 2.0) * j / (4 * (i + 1)));
			h_angle = h_angle;
			float v_angle = (float)((M_PI / 2.0) *
				(divisions - i - 1) / (divisions));
			osg::Vec3 unit;
			unit.set( cosf(h_angle) * cosf(v_angle),
					  sinf(h_angle) * cosf(v_angle),
					  sinf(v_angle) );
			// vertex
			v = osg::Vec3(sz.x() * unit.x(), sz.y() * unit.y(), sz.z() * unit.z());
			vertices->push_back( v );
			// texcoord
			t.set( (float)j / (float)(4 * (i + 1)) * texsz.x(),
				   (float)(divisions - i - 1) / (float)divisions );
			if ( !hemisphere ) {
				t._v[1] = 0.5f + (0.5f * t.y());
			}
			t._v[1] = t.y() * texsz.y();
			texcoords->push_back( t );

			// the bottom hemisphere
			if ( !hemisphere ) {
				if ( i != (divisions - 1) ) {
					// vertex
					v._v[2] = -v.z();
					vertices->push_back( v );
					// texcoord
					t._v[1] = texsz.y() - t.y();
					texcoords->push_back( t );
				}
			}
		}
	}

	// the closing strip of texture coordinates
	const int texStripOffset = texcoords->size();
	t.set( texsz.x() * 0.5f, // weirdness
		   texsz.y() * 1.0f );
	texcoords->push_back( t );
	if (!hemisphere) {
		t._v[1] = 0.0f;
		texcoords->push_back( t );
	}
	for (i = 0; i < divisions; i++) {
		t.set( texsz.x() * 1.0f,
			   (float)(divisions - i - 1) / (float)divisions );
		if (!hemisphere) {
			t._v[1] = 0.5f + (0.5f * t.y());
		}
		t._v[1] = texsz.y() * t.y();
		texcoords->push_back(t);
		// the bottom hemisphere
		if (!hemisphere) {
			if (i != (divisions - 1)) {
				t._v[1] = texsz.y() - t.y();
				texcoords->push_back(t);
			}
		}
	}

	// the bottom texcoords for hemispheres
	const int bottomTexOffset = texcoords->size();
	if (hemisphere) {
		const float astep = (float)((M_PI * 2.0) / (float) (divisions * 4));
		for (i = 0; i < (divisions * 4); i++) {
			float ang = astep * (float)i;
			t.set( texsz.x() * (0.5f + (0.5f * cosf(ang))),
				   texsz.y() * (0.5f + (0.5f * sinf(ang))) );
			texcoords->push_back(t);
		}
	}

	// make the mesh
	//int faceCount = (divisions * divisions) * 8;
	osg::Geometry* geometry = new osg::Geometry();
	outside->addDrawable(geometry);

	// add the faces to the mesh
	int k = (divisions - 1);
	int ringOffset;
	if (!hemisphere) {
		ringOffset = 1 + (((k*k)+k)*2);
	} else {
		ringOffset = 0;
	}

	// mapping from vertices to texcoords must be 1 to 1 so separate arrays are used
	osg::Vec3Array* realVertices = new osg::Vec3Array();
	osg::Vec2Array* realTexcoords = new osg::Vec2Array();
	geometry->setVertexArray( realVertices );
	geometry->setTexCoordArray( 0, realTexcoords );

	for (q = 0; q < 4; q++) {
		for (i = 0; i < divisions; i++) {
			for (j = 0; j < (i + 1); j++) {
				int a, b, c, d, ta, tc;
				// a, b, c form the upwards pointing triangle
				// b, c, d form the downwards pointing triangle
				// ta and tc are the texcoords for a and c
				const bool lastStrip = ((q == 3) && (j == i));
				const bool lastCircle = (i == (divisions - 1));

				// setup 'a'
				if (i > 0) {
					if (lastStrip) {
						k = (i - 1);
						a = 1 + (((k*k)+k)*2);
					} else {
						k = (i - 1);
						a = 1 + (((k*k)+k)*2) + (q*(k+1)) + j;
					}
				} else {
					a = 0;
				}

				// setup 'b'
				b = 1 + (((i*i)+i)*2) + (q*(i+1)) + j;

				// setup 'c'
				if (lastStrip) {
					c = 1 + (((i*i)+i)*2);
				} else {
					c = b + 1;
				}

				// setup 'd' for the down-pointing triangle
				k = (i + 1);
				d = 1 + (((k*k)+k)*2) + (q*(k+1)) + (j + 1);


				// top hemisphere
				a = a * factor;
				if (!lastCircle) {
					b = b * factor;
					c = c * factor;
				} else {
					b = b + ringOffset;
					c = c + ringOffset;
				}
				if (i != (divisions - 2)) {
					d = d * factor;
				} else {
					d = d + ringOffset;
				}

				// deal with the last strip of texture coordinates
				if (!lastStrip) {
					ta = a;
					tc = c;
				} else {
					ta = texStripOffset + (i * factor);
					tc = texStripOffset + ((i + 1) * factor);
				}

				realVertices->push_back( (*vertices)[a] );
				realVertices->push_back( (*vertices)[b] );
				realVertices->push_back( (*vertices)[c] );
				realTexcoords->push_back( (*texcoords)[ta] );
				realTexcoords->push_back( (*texcoords)[b] );
				realTexcoords->push_back( (*texcoords)[tc] );
				if (!lastCircle) {
					realVertices->push_back( (*vertices)[b] );
					realVertices->push_back( (*vertices)[d] );
					realVertices->push_back( (*vertices)[c] );
					realTexcoords->push_back( (*texcoords)[b] );
					realTexcoords->push_back( (*texcoords)[d] );
					realTexcoords->push_back( (*texcoords)[tc] );
				}

				// bottom hemisphere
				if (!hemisphere) {
					a = a + 1;
					ta = ta + 1;
					if (!lastCircle) {
						b = b + 1;
						c = c + 1;
						tc = tc + 1;
					}
					if (i != (divisions - 2)) {
						d = d + 1;
					}
					realVertices->push_back( (*vertices)[a] );
					realVertices->push_back( (*vertices)[c] );
					realVertices->push_back( (*vertices)[b] );
					realTexcoords->push_back( (*texcoords)[ta] );
					realTexcoords->push_back( (*texcoords)[tc] );
					realTexcoords->push_back( (*texcoords)[b] );
					if (!lastCircle) {
						realVertices->push_back( (*vertices)[b] );
						realVertices->push_back( (*vertices)[c] );
						realVertices->push_back( (*vertices)[d] );
						realTexcoords->push_back( (*texcoords)[b] );
						realTexcoords->push_back( (*texcoords)[tc] );
						realTexcoords->push_back( (*texcoords)[d] );
					}
				}
			}
		}
	}

	osg::DrawElementsUInt* faces = new osg::DrawElementsUInt( osg::DrawElements::TRIANGLES, 0 );
	for (unsigned int i = 0; i < realVertices->size(); i++)
		faces->push_back( i );
	geometry->addPrimitiveSet( faces );

	// add the bottom disc
	if (hemisphere) {
		osg::Geometry* bottomGeom = new osg::Geometry();
		bottom->addDrawable( bottomGeom );

		osg::Vec3Array* realVertices = new osg::Vec3Array();
		osg::Vec2Array* realTexcoords = new osg::Vec2Array();
		bottomGeom->setVertexArray( realVertices );
		bottomGeom->setTexCoordArray( 0, realTexcoords );

		k = (divisions - 1);
		const int offset = 1 + (((k*k)+k)*2);
		for (i = 0; i < (divisions * 4); i++) {
			const int vv = (divisions * 4) - i - 1;
			realVertices->push_back( (*vertices)[vv + offset] );
			realTexcoords->push_back( (*texcoords)[i + bottomTexOffset] );
		}

		osg::DrawElementsUInt* faces = new osg::DrawElementsUInt( osg::DrawElements::TRIANGLE_FAN, 0 );
		for (unsigned int i = 0; i < realVertices->size(); i++)
			faces->push_back( i );

		bottomGeom->addPrimitiveSet( faces );
	}
}

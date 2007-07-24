#include "../include/render/TextureRepeaterVisitor.h"

// constructor
TextureRepeaterVisitor::TextureRepeaterVisitor() :
	osg::NodeVisitor( osg::NodeVisitor::NODE_VISITOR, osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) { }

// destructor
TextureRepeaterVisitor::~TextureRepeaterVisitor() { }

// apply method for nodes
void TextureRepeaterVisitor::apply( osg::Node& node ) {
	
	// see if this is a group
	osg::Group* group = dynamic_cast< osg::Group* >( &node );
	if( group ) {
		this->apply( *group );
		return;
	}
	
	// see if this is a geode
	osg::Geode* geode = dynamic_cast< osg::Geode* >( &node );
	if( geode ) {
		this->apply( *geode );
		return;
	}
	
	
	osg::StateSet* stateSet = node.getStateSet();
	if( !stateSet ) {
		return;		// no state set ==> no textures :(
	}
		
	// get the texture
	osg::Texture* texture = (osg::Texture*)stateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE );
	
	// enable texture wrapping
	texture->setWrap( osg::Texture::WRAP_R, osg::Texture::REPEAT );
	texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
	texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );	
}

// apply method for geodes
void TextureRepeaterVisitor::apply( osg::Geode& node ) {
	
	// get the drawables from this geode
	osg::Geode::DrawableList drawables = node.getDrawableList();
	
	// iterate over the drawables
	for( osg::Geode::DrawableList::iterator i = drawables.begin(); i != drawables.end(); i++ ) {
		
		osg::StateSet* stateSet = (*i)->getStateSet();
		
		if( !stateSet ) {
			return;		// no state set ==> no textures :(
		}
			
		// get the texture
		osg::Texture* texture = (osg::Texture*)stateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE );
		
		if(!texture)
			continue;
		
		// enable texture wrapping
		texture->setWrap( osg::Texture::WRAP_R, osg::Texture::REPEAT );
		texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
		texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );	
	}
}

// apply for osg::Group
void TextureRepeaterVisitor::apply( osg::Group& group ) {
	
	// call the appropriate apply method for all children
	if( group.getNumChildren() > 0 ) {
		for( unsigned int i = 0; i < group.getNumChildren(); i++) {
			osg::Node* node = group.getChild( i );
			this->apply( *node );
		}
	}
}

#ifndef SPHERE_H_
#define SPHERE_H_

#include "bz2object.h"

class sphere : public bz2object {
public:
	// default constructor
	sphere();
	
	// constructor with data
	sphere(string& data);
	
	static DataEntry* init() { return new sphere(); }
	static DataEntry* init(string& data) { return new sphere(data); }
	
	// getter
	string get(void);
	
	// setter
	int update(string& data);
	
	// toString
	string toString(void);
	
	// render
	int render(void);
	
private:
	bool flatShading, smoothbounce;
	int divisions;
};

#endif /*SPHERE_H_*/
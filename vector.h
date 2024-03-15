#include <vector>

#ifndef _VECTOR_H
#define _VECTOR_H

class vector {
	// definitions for negation, addition, subtraction, dot product, 
	// cross product, scalar multiplication and scalar division
	friend vector operator-(vector const&);
	friend vector operator+(vector const&, vector const&);
	friend vector operator-(vector const&, vector const&);
	friend double operator%(vector const&, vector const&);
	friend vector operator*(vector const&, vector const&);
	friend vector operator*(vector const&, double);
	friend vector operator/(vector const&, double);

	// other vector functions
	friend double len(vector const&);
	friend vector normalise(vector const&);
	friend double angle(vector const&, vector const&);

public:
	//constructors
	vector() {};
	vector(double x, double y, double z) {p[0] = x; p[1] = y; p[2] = z;}
	vector(double a[3]) {p[0] = a[0]; p[1] = a[1]; p[2] = a[2];}
	~vector() {};

	//getters and setters
	double& operator[](int i) {return p[i];}
	double x() const { return p[0]; };
	double y() const { return p[1]; };
	double z() const { return p[2]; };

	void getValue(double d[3]) {d[0] = p[0]; d[1] = p[1]; d[2] = p[2];}
	void setValue(double d[3]) {p[0] = d[0]; p[1] = d[1]; p[2] = d[2];}

	double getValue(int n) const { return p[n]; }
	vector setValue(double x, double y, double z) { p[0] = x, p[1] = y, p[2] = z; return *this;}
	double setValue(int n, double x) { return p[n] = x; }

	double length() const;

	void setX(double x) { p[0] = x; };
	void setY(double y) { p[1] = y; };
	void setZ(double z) { p[2] = z; };

	//data members
	double p[3]; 
};
#endif

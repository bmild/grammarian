#ifndef	VECTOR_H
#define	VECTOR_H
#include <stdio.h>
#include <math.h>
#include <functional>	// for std::hash<float>
/*
 * Three-dimensional points, vectors and transformations
 *
 * Points and Vectors are distinguished even though they're
 * represented identically.
 * Points are anchored in space, while Vectors are free-floating.
 * You can add a Point to a Vector (yielding a Point), but you
 * can't add two Points.
 * Subtracting one point from another yields the Vector joining them.
 * Points and Vectors transform differently.
 * Given:
 *	Affine t;
 *	Point p;
 *	Vector v;
 * we have
 *	t.transform(v) == t.transform(p+v)-t.transform(p)
 *
 * Vectors support all the usual vector space operations:
 *	addition
 *	scalar multiplication
 *	dot & cross products
 *	linear transformation
 *	quotient and remainder (Condon & McIlroy http://www.iq0.com/notes/vdiv.html)
 *	lerp
 *	reflect (in a surface with given normal)
 *	rotate (around a given axis by a given angle)
 *
 * Points support
 *	addition and subtraction of vectors
 *	lerp (Should support templated multilerp)
 */
struct Vector;
inline Vector operator +(Vector p, const Vector &q);
inline Vector operator -(Vector p, const Vector &q);
inline Vector operator *(Vector p, float m);
inline Vector operator *(float m, Vector p);
inline Vector operator /(Vector p, float m);
inline Vector operator %(Vector p, const Vector &q);
inline Vector operator -(const Vector &v);
// Under the covers, Points and Vectors are represented
// identically as (x,y,z) Triples.
class Triple{
protected:
	Triple(float x, float y, float z): x(x), y(y), z(z){}
	Triple(){}
public:
	float x, y, z;
	void print(FILE *f=stdout) const{
		fprintf(f, "%7.3f %7.3f %7.3f\n", x, y, z);
	}
	float &operator[](int i) { return (&x)[i]; }
	const float &operator[](int i) const { return (&x)[i]; }
	bool operator==(const Triple &t) const { return x==t.x&&y==t.y&&z==t.z; }
	// for STL use
	// return value is
	//		hash(x)^bitrotate(hash(y), 21)^bitrotate(hash(z), 42)
	// This knows that size_t is 64 bits.
	// Use it in declarations like
	//		unordered_set<Point, Point::Hash> pointset;
	struct Hash{
		size_t operator()(const Triple &t) const{
			std::hash<float> hashfloat;
			size_t f, h;
			h=hashfloat(t.x);
			f=hashfloat(t.y);
			h^=(f<<21)|(f>>43);
			f=hashfloat(t.z);
			h^=(f<<42)|(f>>22);
			return h;
		}
	};
};
struct Point: public Triple{
	Point(float x, float y, float z): Triple(x, y, z){}
	template<typename T> Point(const T *xyz): Triple(xyz[0], xyz[1], xyz[2]){}
	Point(){}
	Point &operator =(const Point &p){
		x=p.x;
		y=p.y;
		z=p.z;
		return *this;
	}
	explicit Point(const Vector &v);
	Point neg() const{ return Point(-x, -y, -z); }
	Point &operator +=(const Vector &p);
	Point &operator -=(const Vector &p);
	Point lerp(Point a, float alpha) const{
		return Point(x+(a.x-x)*alpha, y+(a.y-y)*alpha, z+(a.z-z)*alpha);
	}
};
struct Vector: public Triple{
	Vector(float x, float y, float z): Triple(x, y, z){}
	template<typename T> Vector(const T *xyz): Triple(xyz[0], xyz[1], xyz[2]){}
	Vector(){}
	Vector &operator =(const Vector &v){
		x=v.x;
		y=v.y;
		z=v.z;
		return *this;
	}
	explicit Vector(const Point &p): Triple(p.x, p.y, p.z){}
	Vector neg() const{ return Vector(-x, -y, -z); }
	float dot(Vector p) const{ return x*p.x+y*p.y+z*p.z; }
	float len() const{ return sqrt(dot(*this)); }
	Vector unit() const{ float l=len(); return Vector(x/l, y/l, z/l); }
	Vector &operator +=(const Vector &p){
		x+=p.x;
		y+=p.y;
		z+=p.z;
		return *this;
	}
	Vector &operator -=(const Vector &p){
		x-=p.x;
		y-=p.y;
		z-=p.z;
		return *this;
	}
	Vector &operator *=(float m){
		x*=m;
		y*=m;
		z*=m;
		return *this;
	}
	Vector &operator /=(float m){
		x/=m;
		y/=m;
		z/=m;
		return *this;
	}
	/*
	 * Vector quotient (operator /) and remainder (operator %, operator %=).
	 *
	 * See Vector quotients: an aid for 3D calculations,
	 * by J. H. Condon and M. D. McIlroy, reproduced at
	 * http://www.iq0.com/notes/vdiv.html
	 *
	 * Following the analogous definition for scalars,
	 * let A and B be vectors, choose q (a scalar) and R so that
	 *	A=q*B+R
	 * and |R| is minimized.  The solution is
	 *
	 *	q = A.B/B.B
	 *	R = A - q*B
	 *
	 * We write q=A/B and R=A%B.
	 *
	 * A/B is the length of the component of A in the direction of B,
	 * measured in units of B's length, and A%B is the component of
	 * A perpendicular to B.
	 *
	 * Obviously we can do anything we want without these definitions,
	 * but many calculations are more perspicuous with them.
	 */
	float operator /(Vector b) const{ return this->dot(b)/b.dot(b); }
	Vector &operator%=(Vector b){
		*this-=b*(*this/b);
		return *this;
	}
	Vector cross(Vector b) const{
		return Vector(y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x);
	}
	Vector lerp(Vector b, float alpha) const{
		return Vector(x+(b.x-x)*alpha, y+(b.y-y)*alpha, z+(b.z-z)*alpha);
	}
	Vector refl(Vector n) const{
		float nl=n.dot(n);
		if(nl==0.) return *this;
		return *this-n*(2.*dot(n)/nl);
	}
	/*
	 * Rotate about the given axis by the given angle.
	 * Assuming axis is a unit vector:
	 *	cos(angle)*v + (1-cos(angle))*(v.axis)*axis - sin(angle)*(v x axis)
	 *
	 * If angle==0., we could just return v.
	 */
	Vector rotate(Vector axis, float angle) const{
		float c=cos(angle);
		float s=sin(angle);
		axis=axis.unit();
		return *this*c
			   +axis*((1.-c)*this->dot(axis))
			   -this->cross(axis)*s;
	}
	/*
	 * Set r to the reflection of *this in a surface with normal n.
	 * (Imagine that *this points from the surface toward some point.
	 * Then r will point at its reflection.)
	 *
	 * let
	 *	*this=alpha*n+a	(1)
	 * with a.n=0
	 * Then the reflection vector is
	 *	r=alpha*n-a
	 *
	 * Taking the dot product of both sides of (1) with n, we get
	 *	*this.n=alpha*n.n+a.n=alpha*n.n
	 *	*this.n=alpha*n.n
	 *	alpha=*this.n/n.n
	 * So, from (1)
	 *	a=*this-alpha*n
	 * and so
	 *	r=alpha*n-a
	 *	r=alpha*n-*this+alpha*n
	 *	r=2*alpha*n-*this
	 *
	 * This assumes that n is a unit vector, so n.n=1
	 * Note that if *this is a unit vector, then so is the return value.
	 */
	inline Vector reflect(const Vector &n){
		return 2.*dot(n)*n-*this;
	}
};
inline Vector operator +(Vector p, const Vector &q){
	p+=q;
	return p;
}
inline Vector operator -(Vector p, const Vector &q){
	p-=q;
	return p;
}
inline Vector operator *(Vector p, float m){
	p*=m;
	return p;
}
inline Vector operator *(float m, Vector p){
	p*=m;
	return p;
}
inline Vector operator /(Vector p, float m){
	p/=m;
	return p;
}
inline Vector operator %(Vector p, const Vector &q){
	p%=q;
	return p;
}
inline Point operator +(Point p, const Vector &q){
	p+=q;
	return p;
}
inline Point operator +(const Vector &p, Point q){
	q+=p;
	return q;
}
inline Point operator -(Point p, const Vector &q){
	p-=q;
	return p;
}
inline Vector operator -(const Point &p, const Point &q){
	return Vector(p.x-q.x, p.y-q.y, p.z-q.z);
}
inline Vector operator -(const Vector &v){
	return v.neg();
}

#endif
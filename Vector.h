#pragma once

#include <cmath>
#include <assert.h>

struct Vector
{
	Vector();
	Vector( float _x, float _y, float _z );

	void Init( void );
	void Init( float _x, float _y, float _z );

	Vector operator-( const Vector &other ) const;

	float Length( void ) const;

	float &operator[]( int i );
	float operator[]( int i ) const;

	float x, y, z;
};

struct QAngle
{
	float x, y, z;
	void Init( void );

	void Init( float _x, float _y, float _z );
};


// ===========================================================
// Inlines
// ===========================================================

inline void QAngle::Init( void )
{
	x = y = z = 0.0f;
}

inline void QAngle::Init( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
}

inline Vector::Vector()
	: x( 0 ), y( 0 ), z( 0 )
{

}

inline Vector::Vector( float _x, float _y, float _z )
	: x( _x ), y( _y ), z( _z )
{

}

inline void Vector::Init( void )
{
	x = y = z = 0.0f;
}
inline void Vector::Init( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
}

inline Vector Vector::operator-( const Vector &other ) const
{
	Vector result;
	result.x = this->x - other.x;
	result.y = this->y - other.y;
	result.z = this->z - other.z;
	return result;
}

inline float Vector::Length( void ) const
{
	return sqrtf( x * x + y * y + z * z );
}

inline float &Vector::operator[]( int i )
{
	assert( ( i >= 0 ) && ( i < 3 ) );
	return ( (float *)this )[ i ];
}

inline float Vector::operator[]( int i ) const
{
	assert( ( i >= 0 ) && ( i < 3 ) );
	return ( (float *)this )[ i ];
}
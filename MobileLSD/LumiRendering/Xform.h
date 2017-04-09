#pragma once


/*
Szymon Rusinkiewicz
Princeton University

XForm.h
Rigid-body transforms (represented internally as column-major 4x4 matrices)

Supports the following operations:
xform xf1, xf2;		// Initialized to the identity
XForm<float> xf3;	// xform is XForm<double>
xf1=xform::trans(u,v,w);// An xform that translates.
xf1=xform::rot(ang,ax); // An xform that rotates.
glMultMatrixd(xf1);	// Conversion to column-major array
xf1.read("myfile.xf");	// Read xform from file
xf1.write("myfile.xf");	// Write xform to file
xf1 * xf2		// Matrix-matrix multiplication
xf1 * inv(xf2)		// Fast inverse; assumes rigid body transform
xf1 * vec(1,2,3)	// Matrix-vector multiplication
rot_only(xf1)		// An xform that does the rotation of xf1
trans_only(xf1)		// An xform that does the translation of xf1
invert(xf1);		// Inverts xform in place
orthogonalize(xf1);	// Makes matrix orthogonal
*/

#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
using std::min;
using std::max;
using std::swap;
using std::sqrt;

#ifndef XFORM_H

#define XFORM_H
template <class T>
class XForm {
private:
	T m[16]; // Column-major (OpenGL) order

public:
	// Constructors
	XForm(const T m0 = 1, const T m1 = 0, const T m2 = 0, const T m3 = 0,
		const T m4 = 0, const T m5 = 1, const T m6 = 0, const T m7 = 0,
		const T m8 = 0, const T m9 = 0, const T m10 = 1, const T m11 = 0,
		const T m12 = 0, const T m13 = 0, const T m14 = 0, const T m15 = 1)
	{
		m[0] = m0;  m[1] = m1;  m[2] = m2;  m[3] = m3;
		m[4] = m4;  m[5] = m5;  m[6] = m6;  m[7] = m7;
		m[8] = m8;  m[9] = m9;  m[10] = m10; m[11] = m11;
		m[12] = m12; m[13] = m13; m[14] = m14; m[15] = m15;
	}
	template <class S> explicit XForm(const S &x)
	{
		for (int i = 0; i < 16; i++) m[i] = x[i];
	}

	// Default destructor, copy constructor, assignment operator

	// Array reference and conversion to array - no bounds checking 
	const T operator [] (int i) const
	{
		return m[i];
	}
	T &operator [] (int i)
	{
		return m[i];
	}
	operator const T *() const
	{
		return m;
	}
	operator const T *()
	{
		return m;
	}
	operator T *()
	{
		return m;
	}

	// Static members - really just fancy constructors
	static XForm<T> identity()
	{
		return XForm<T>();
	}
	static XForm<T> trans(const T &tx, const T &ty, const T &tz)
	{
		return XForm<T>(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, tz, 1);
	}
	template <class S> static XForm<T> trans(const S &t)
	{
		return XForm<T>::trans(t[0], t[1], t[2]);
	}
	static XForm<T> rot(const T &angle,
		const T &rx, const T &ry, const T &rz)
	{
		// Angle in radians, unlike OpenGL
		T l = sqrt(rx*rx + ry*ry + rz*rz);
		if (l == T(0))
			return XForm<T>();
		T l1 = T(1) / l, x = rx*l1, y = ry*l1, z = rz*l1;
		T s = sin(angle), c = cos(angle);
		T xs = x*s, ys = y*s, zs = z*s, c1 = T(1) - c;
		T xx = c1*x*x, yy = c1*y*y, zz = c1*z*z;
		T xy = c1*x*y, xz = c1*x*z, yz = c1*y*z;
		return XForm<T>(xx + c, xy + zs, xz - ys, 0,
			xy - zs, yy + c, yz + xs, 0,
			xz + ys, yz - xs, zz + c, 0,
			0, 0, 0, 1);
	}
	template <class S> static XForm<T> rot(const T &angle, const S &axis)
	{
		return XForm<T>::rot(angle, axis[0], axis[1], axis[2]);
	}

	// Read an XForm from a file
	static XForm<T> read(const char *filename)
	{
		XForm<T> M;
		std::ifstream f(filename);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				f >> M[i + 4 * j];
		f.close();
		if (f.good())
			return M;
		else
			return XForm<T>();
	}

	// Write an XForm to a file
	bool write(const char *filename) const
	{
		std::ofstream f(filename);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				f << m[i + 4 * j];
				if (j == 3)
					f << std::endl;
				else
					f << " ";
			}
		}
		f.close();
		return f.good();
	}
};

typedef XForm<double> xform;


// Matrix multiplication
template <class T>
static inline XForm<T> operator * (const XForm<T> &xf1, const XForm<T> &xf2)
{
	return XForm<T>(
		xf1[0] * xf2[0] + xf1[4] * xf2[1] + xf1[8] * xf2[2] + xf1[12] * xf2[3],
		xf1[1] * xf2[0] + xf1[5] * xf2[1] + xf1[9] * xf2[2] + xf1[13] * xf2[3],
		xf1[2] * xf2[0] + xf1[6] * xf2[1] + xf1[10] * xf2[2] + xf1[14] * xf2[3],
		xf1[3] * xf2[0] + xf1[7] * xf2[1] + xf1[11] * xf2[2] + xf1[15] * xf2[3],
		xf1[0] * xf2[4] + xf1[4] * xf2[5] + xf1[8] * xf2[6] + xf1[12] * xf2[7],
		xf1[1] * xf2[4] + xf1[5] * xf2[5] + xf1[9] * xf2[6] + xf1[13] * xf2[7],
		xf1[2] * xf2[4] + xf1[6] * xf2[5] + xf1[10] * xf2[6] + xf1[14] * xf2[7],
		xf1[3] * xf2[4] + xf1[7] * xf2[5] + xf1[11] * xf2[6] + xf1[15] * xf2[7],
		xf1[0] * xf2[8] + xf1[4] * xf2[9] + xf1[8] * xf2[10] + xf1[12] * xf2[11],
		xf1[1] * xf2[8] + xf1[5] * xf2[9] + xf1[9] * xf2[10] + xf1[13] * xf2[11],
		xf1[2] * xf2[8] + xf1[6] * xf2[9] + xf1[10] * xf2[10] + xf1[14] * xf2[11],
		xf1[3] * xf2[8] + xf1[7] * xf2[9] + xf1[11] * xf2[10] + xf1[15] * xf2[11],
		xf1[0] * xf2[12] + xf1[4] * xf2[13] + xf1[8] * xf2[14] + xf1[12] * xf2[15],
		xf1[1] * xf2[12] + xf1[5] * xf2[13] + xf1[9] * xf2[14] + xf1[13] * xf2[15],
		xf1[2] * xf2[12] + xf1[6] * xf2[13] + xf1[10] * xf2[14] + xf1[14] * xf2[15],
		xf1[3] * xf2[12] + xf1[7] * xf2[13] + xf1[11] * xf2[14] + xf1[15] * xf2[15]
		);
}


// Component-wise equality and inequality (#include the usual caveats
// about comparing floats for equality...)
template <class T>
static inline bool operator == (const XForm<T> &xf1, const XForm<T> &xf2)
{
	for (int i = 0; i < 16; i++)
		if (xf1[i] != xf2[i])
			return false;
	return true;
}

template <class T>
static inline bool operator != (const XForm<T> &xf1, const XForm<T> &xf2)
{
	for (int i = 0; i < 16; i++)
		if (xf1[i] != xf2[i])
			return true;
	return false;
}


// Fast Inverse
// XXX - Danger, Will Robinson! Danger!  Assumes rigid-body transformation
template <class T>
static inline XForm<T> inv(const XForm<T> &xf)
{
	return XForm<T>(xf[0], xf[4], xf[8], xf[3],
		xf[1], xf[5], xf[9], xf[7],
		xf[2], xf[6], xf[10], xf[11],
		-(xf[0] * xf[12] + xf[1] * xf[13] + xf[2] * xf[14]),
		-(xf[4] * xf[12] + xf[5] * xf[13] + xf[6] * xf[14]),
		-(xf[8] * xf[12] + xf[9] * xf[13] + xf[10] * xf[14]),
		xf[15]);
}

template <class T>
static inline void invert(XForm<T> &xf)
{
	// NOTE: Assumes rigid-body transformation
	swap(xf[1], xf[4]);
	swap(xf[2], xf[8]);
	swap(xf[6], xf[9]);

	T tx = -xf[12], ty = -xf[13], tz = -xf[14];
	xf[12] = tx*xf[0] + ty*xf[4] + tz*xf[8];
	xf[13] = tx*xf[1] + ty*xf[5] + tz*xf[9];
	xf[14] = tx*xf[2] + ty*xf[6] + tz*xf[10];
}

template <class T>
static inline XForm<T> rot_only(const XForm<T> &xf)
{
	return XForm<T>(xf[0], xf[1], xf[2], 0,
		xf[4], xf[5], xf[6], 0,
		xf[8], xf[9], xf[10], 0,
		0, 0, 0, 1);
}

template <class T>
static inline XForm<T> trans_only(const XForm<T> &xf)
{
	return XForm<T>(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		xf[12], xf[13], xf[14], 1);
}

template <class T>
static inline void orthogonalize(XForm<T> &xf)
{
	if (xf[15] == T(0))	// Yuck.  Doesn't make sense...
		xf[15] = T(1);

	T q0 = xf[0] + xf[5] + xf[10] + xf[15];
	T q1 = xf[6] - xf[9];
	T q2 = xf[8] - xf[2];
	T q3 = xf[1] - xf[4];
	T l = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);

	XForm<T> M = XForm<T>::rot(T(2)*acos(q0 / l), q1, q2, q3);
	M[12] = xf[12] / xf[15];
	M[13] = xf[13] / xf[15];
	M[14] = xf[14] / xf[15];

	xf = M;
}


// Matrix-vector multiplication
template <class S, class T>
static inline const S operator * (const XForm<T> &xf, const S &v)
{
	// Assumes bottom row of xf is [0,0,0,1]
	return S(xf[0] * v[0] + xf[4] * v[1] + xf[8] * v[2] + xf[12],
		xf[1] * v[0] + xf[5] * v[1] + xf[9] * v[2] + xf[13],
		xf[2] * v[0] + xf[6] * v[1] + xf[10] * v[2] + xf[14]);
}

#endif

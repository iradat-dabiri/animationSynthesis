#include <cmath>
#include <cstdio>
#include "transform.h"
#include "types.h"

void matrix_transpose(double a[4][4], double b[4][4]) {
	int i, j;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			b[i][j] = a[j][i];
}

void matrix_print(char* str, double a[4][4]) {
	int i;

	printf("matrix %s:\n", str);
	for (i = 0; i < 4; i++)
		printf(" %8.3f %8.3f %8.3f %8.3f\n",
			a[i][0], a[i][1], a[i][2], a[i][3]);
}

// transform point (x, y, z) by matrix m with last row 0 0 0 1
void matrix_transform_affine(double m[4][4], double x, double y, double z, double pt[3]) {
	pt[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
	pt[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
	pt[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
}

void v3_cross(double a[3], double b[3], double c[3]) {
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

double v3_dot(double a[3], double b[3]) {
	return(a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

double v3_magnitude(double a[3]) {
	return(sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]));
}

void rotationXYZ(double* v, double a, double b, double c) {
	double Rx[4][4], Ry[4][4], Rz[4][4];

	rotationZ(Rz, c);
	rotationY(Ry, b);
	rotationX(Rx, a);

	matrix_transform_affine(Rz, v[0], v[1], v[2], v);
	matrix_transform_affine(Ry, v[0], v[1], v[2], v);
	matrix_transform_affine(Rx, v[0], v[1], v[2], v);
}

void rotationX(double r[][4], double a) {
	a = a * M_PI / 180.;
	r[0][0] = 1;       r[0][1] = 0;       r[0][2] = 0;       r[0][3] = 0;
	r[1][0] = 0;       r[1][1] = cos(a);  r[1][2] = -sin(a); r[1][3] = 0;
	r[2][0] = 0;       r[2][1] = sin(a);  r[2][2] = cos(a);  r[2][3] = 0;
	r[3][0] = 0;       r[3][1] = 0;       r[3][2] = 0;       r[3][3] = 1;
}

void rotationY(double r[][4], double b) {
	b = b * M_PI / 180.;
	r[0][0] = cos(b);  r[0][1] = 0;       r[0][2] = sin(b); r[0][3] = 0;
	r[1][0] = 0;       r[1][1] = 1;       r[1][2] = 0;      r[1][3] = 0;
	r[2][0] = -sin(b); r[2][1] = 0;       r[2][2] = cos(b); r[2][3] = 0;
	r[3][0] = 0;       r[3][1] = 0;       r[3][2] = 0;      r[3][3] = 1;
}

void rotationZ(double r[][4], double c) {
	c = c * M_PI / 180.;
	r[0][0] = cos(c); r[0][1] = -sin(c); r[0][2] = 0; r[0][3] = 0;
	r[1][0] = sin(c); r[1][1] = cos(c);  r[1][2] = 0; r[1][3] = 0;
	r[2][0] = 0;      r[2][1] = 0;       r[2][2] = 1; r[2][3] = 0;
	r[3][0] = 0;      r[3][1] = 0;       r[3][2] = 0; r[3][3] = 1;
}

void matrix_mult(double a[][4], double b[][4], double c[][4]) {
	int i, j, k;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			c[i][j] = 0;
			for (k = 0; k < 4; k++)
				c[i][j] += a[i][k] * b[k][j];
		}
}

double get_angle(double* v1, double* v2, double* axis) {
	double dot_prod = v3_dot(v1, v2);
	double r_axis_len = v3_magnitude(axis);

	return atan2(r_axis_len, dot_prod);
}

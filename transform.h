#ifndef _TRANSFORM_H
#define _TRANSFORM_H

class Matrix{};

void matrix_transpose(double a[4][4], double b[4][4]);
void matrix_print(char* str, double a[4][4]);
void matrix_transform_affine(double m[4][4], double x, double y, double z, double pt[3]);
void matrix_mult(double a[][4], double b[][4], double c[][4]);

void v3_cross(double a[3], double b[3], double c[3]);
double v3_magnitude(double a[3]);
double v3_dot(double a[3], double b[3]);

void rotationXYZ(double* v, double a, double b, double c);
void rotationX(double r[][4], double a);
void rotationY(double r[][4], double b);
void rotationZ(double r[][4], double c);

double get_angle(double* v1, double* v2, double* axis);

#endif

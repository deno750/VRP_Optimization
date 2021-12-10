#ifndef DISTUTIL_H
#define DISTUTIL_H

#include "utility.h"

#define PI 3.14159265358979323846264
#define EARTH_RAD 6378.388

/**
 * Rounds to the nearest integer value.
 * For negative numbers it may create some problems.
 * 
 * Some example behaviors:
 * 0.4 -> 0.0
 * 0.51 -> 1.0
 * -0.3 -> 0.0
 * -0.6 -> 0.0
 * -1.2 -> 0.0
 * -1.6 -> 1.0
 *  
 * @param x The number to convert to the nearest integer
 * @returns the nearest integer value
 */
static double nint(double x);

/**
 * Rounds the number such as round function.
 * 
 * Use this instead of nint when you work with negative numbers.
 * 
 * Some examples:
 * 0.4 -> 0.0
 * 0.51 -> 1.0
 * -0.3 -> 0.0
 * -0.6 -> -1.0
 *
 * @param x The number to round
 * @returns the rounded value
 */
static double e_round(double x);

/**
 * Calculates the euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 *
 * @param p1 Point 1
 * @param p2 Point 2
 * @param integer 1 if the retured distance should be integer, 0 otherwise
 * @returns the eucledian 2d distance
 */
double calc_euc2d(node p1, node p2, int integer);

/**
 * Calculates the pseudo euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 *
 * @param p1 Point 1
 * @param p2 Point 2
 * @param integer 1 if the retured distance should be integer, 0 otherwise
 * @returns the pseudo eucledian 2d distance
 */
double calc_pseudo_euc(node p1, node p2, int integer);

/**
 * Calculates the manhattan 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 *
 * @param p1 Point 1
 * @param p2 Point 2
 * @param integer 1 if the retured distance should be integer, 0 otherwise
 * @returns the manhattan 2d distance
 */
double calc_man2d(node p1, node p2, int integer);

/**
 * Calculates the maximum 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 *
 * @param p1 Point 1
 * @param p2 Point 2
 * @param integer 1 if the retured distance should be integer, 0 otherwise
 * @returns the max 2d distance
 */
double calc_max2d(node p1, node p2, int integer);

/**
 * Calculates the ceiling of euclidean 2d distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 *
 * @param p1 Point 1
 * @param p2 Point 2
 * @returns the ceil 2d distance
 */
double calc_ceil2d(node p1, node p2);  //Returns always an integer value


/**
 * Calculates the latitude and longitude of a given point.
 * 
 * @param p   Point
 * @param lat Latitude pointer 
 * @param lon Longitude pointer
 */
static void calc_lat_lon(node p, double *lat, double *lon);

/**
 * Calculates the geographical distance between two points.
 * 
 * Integer param is passed when an integer distance have to be
 * computed.
 *
 * @param p1 Point 1
 * @param p2 Point 2
 * @param integer 1 if the retured distance should be integer, 0 otherwise
 * @returns the geo distance
 */
double calc_geo(node p1, node p2, int integer);

/**
 * Calculating the distance based on the instance's weight_type
 *
 * @param i The node i index
 * @param j The node j index
 * @param inst The instance pointer of the problem
 * @returns the distance between node i and node j accordingly with the instance
 */ 
double calc_dist(int i, int j, instance *inst);

#endif
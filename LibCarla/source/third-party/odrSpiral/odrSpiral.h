/* ===================================================
 *  file:       odrSpiral.c
 * ---------------------------------------------------
 *  purpose:	free sample for computing spirals
 *              in OpenDRIVE applications 
 * ---------------------------------------------------
 *  first edit:	09.03.2010 by M. Dupuis @ VIRES GmbH
 *  last mod.:  09.03.2010 by M. Dupuis @ VIRES GmbH
 * ===================================================
 
    Copyright 2010 VIRES Simulationstechnologie GmbH

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */
 
/**
* compute the actual "standard" spiral, starting with curvature 0
* @param s      run-length along spiral
* @param cDot   first derivative of curvature [1/m2]
* @param x      resulting x-coordinate in spirals local co-ordinate system [m]
* @param y      resulting y-coordinate in spirals local co-ordinate system [m]
* @param t      tangent direction at s [rad]
*/

extern void odrSpiral( double s, double cDot, double *x, double *y, double *t );

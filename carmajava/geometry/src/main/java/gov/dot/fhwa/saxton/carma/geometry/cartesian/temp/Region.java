/*
 * Copyright (C) 2017 Michael McConnell.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package gov.dot.fhwa.saxton.carma.geometry.cartesian.temp;// Change

import Java.lang.Math;
/**
 * A representation of a point in N-dimensional space.
 */
// A region does not have a point ordering so the description of point order is left up to the user
 
// Can use the Quick hull algorithm to calculate the convex hull in n-dimensions
// Quickhull has average complexity is considered to be Θ(n * log(n)), whereas in the worst case it takes O(n2)
// Chan's algorithm can be used for 2/3d and works in O(nlog(h)) time where n=num points, h=num points in final hull


public class Region {
  private List<Point> vertices;

  public Region(List<Point> vertices){
    this.vertices = vertices;
  }

  public int getNumDimensions(){
    return vertices.at(0).getNumDimensions();
  }
}

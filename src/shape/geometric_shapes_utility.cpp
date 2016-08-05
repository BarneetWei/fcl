/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#include "fcl/math/geometry.h"
#include "fcl/shape/geometric_shapes_utility.h"
#include "fcl/BVH/BV_fitter.h"

namespace fcl
{

namespace details
{

std::vector<Vector3d> getBoundVertices(const Box& box, const Transform3d& tf)
{
  std::vector<Vector3d> result(8);
  FCL_REAL a = box.side[0] / 2;
  FCL_REAL b = box.side[1] / 2;
  FCL_REAL c = box.side[2] / 2;
  result[0] = tf * Vector3d(a, b, c);
  result[1] = tf * Vector3d(a, b, -c);
  result[2] = tf * Vector3d(a, -b, c);
  result[3] = tf * Vector3d(a, -b, -c);
  result[4] = tf * Vector3d(-a, b, c);
  result[5] = tf * Vector3d(-a, b, -c);
  result[6] = tf * Vector3d(-a, -b, c);
  result[7] = tf * Vector3d(-a, -b, -c);
  
  return result;
}

// we use icosahedron to bound the sphere
std::vector<Vector3d> getBoundVertices(const Sphere& sphere, const Transform3d& tf)
{
  std::vector<Vector3d> result(12);
  const FCL_REAL m = (1 + sqrt(5.0)) / 2.0;
  FCL_REAL edge_size = sphere.radius * 6 / (sqrt(27.0) + sqrt(15.0));

  FCL_REAL a = edge_size;
  FCL_REAL b = m * edge_size;
  result[0] = tf * Vector3d(0, a, b);
  result[1] = tf * Vector3d(0, -a, b);
  result[2] = tf * Vector3d(0, a, -b);
  result[3] = tf * Vector3d(0, -a, -b);
  result[4] = tf * Vector3d(a, b, 0);
  result[5] = tf * Vector3d(-a, b, 0);
  result[6] = tf * Vector3d(a, -b, 0);
  result[7] = tf * Vector3d(-a, -b, 0);
  result[8] = tf * Vector3d(b, 0, a);
  result[9] = tf * Vector3d(b, 0, -a);
  result[10] = tf * Vector3d(-b, 0, a);
  result[11] = tf * Vector3d(-b, 0, -a);

  return result;
}

std::vector<Vector3d> getBoundVertices(const Ellipsoid& ellipsoid, const Transform3d& tf)
{
  // we use scaled icosahedron to bound the ellipsoid

  std::vector<Vector3d> result(12);

  const FCL_REAL phi = (1.0 + std::sqrt(5.0)) / 2.0;  // golden ratio

  const FCL_REAL a = std::sqrt(3.0) / (phi * phi);
  const FCL_REAL b = phi * a;

  const FCL_REAL& A = ellipsoid.radii[0];
  const FCL_REAL& B = ellipsoid.radii[1];
  const FCL_REAL& C = ellipsoid.radii[2];

  const FCL_REAL Aa = A * a;
  const FCL_REAL Ab = A * b;
  const FCL_REAL Ba = B * a;
  const FCL_REAL Bb = B * b;
  const FCL_REAL Ca = C * a;
  const FCL_REAL Cb = C * b;

  result[0] = tf * Vector3d(0, Ba, Cb);
  result[1] = tf * Vector3d(0, -Ba, Cb);
  result[2] = tf * Vector3d(0, Ba, -Cb);
  result[3] = tf * Vector3d(0, -Ba, -Cb);
  result[4] = tf * Vector3d(Aa, Bb, 0);
  result[5] = tf * Vector3d(-Aa, Bb, 0);
  result[6] = tf * Vector3d(Aa, -Bb, 0);
  result[7] = tf * Vector3d(-Aa, -Bb, 0);
  result[8] = tf * Vector3d(Ab, 0, Ca);
  result[9] = tf * Vector3d(Ab, 0, -Ca);
  result[10] = tf * Vector3d(-Ab, 0, Ca);
  result[11] = tf * Vector3d(-Ab, 0, -Ca);

  return result;
}

std::vector<Vector3d> getBoundVertices(const Capsule& capsule, const Transform3d& tf)
{
  std::vector<Vector3d> result(36);
  const FCL_REAL m = (1 + sqrt(5.0)) / 2.0;

  FCL_REAL hl = capsule.lz * 0.5;
  FCL_REAL edge_size = capsule.radius * 6 / (sqrt(27.0) + sqrt(15.0));
  FCL_REAL a = edge_size;
  FCL_REAL b = m * edge_size;
  FCL_REAL r2 = capsule.radius * 2 / sqrt(3.0);


  result[0] = tf * Vector3d(0, a, b + hl);
  result[1] = tf * Vector3d(0, -a, b + hl);
  result[2] = tf * Vector3d(0, a, -b + hl);
  result[3] = tf * Vector3d(0, -a, -b + hl);
  result[4] = tf * Vector3d(a, b, hl);
  result[5] = tf * Vector3d(-a, b, hl);
  result[6] = tf * Vector3d(a, -b, hl);
  result[7] = tf * Vector3d(-a, -b, hl);
  result[8] = tf * Vector3d(b, 0, a + hl);
  result[9] = tf * Vector3d(b, 0, -a + hl);
  result[10] = tf * Vector3d(-b, 0, a + hl);
  result[11] = tf * Vector3d(-b, 0, -a + hl);

  result[12] = tf * Vector3d(0, a, b - hl);
  result[13] = tf * Vector3d(0, -a, b - hl);
  result[14] = tf * Vector3d(0, a, -b - hl);
  result[15] = tf * Vector3d(0, -a, -b - hl);
  result[16] = tf * Vector3d(a, b, -hl);
  result[17] = tf * Vector3d(-a, b, -hl);
  result[18] = tf * Vector3d(a, -b, -hl);
  result[19] = tf * Vector3d(-a, -b, -hl);
  result[20] = tf * Vector3d(b, 0, a - hl);
  result[21] = tf * Vector3d(b, 0, -a - hl);
  result[22] = tf * Vector3d(-b, 0, a - hl);
  result[23] = tf * Vector3d(-b, 0, -a - hl);

  FCL_REAL c = 0.5 * r2;
  FCL_REAL d = capsule.radius;
  result[24] = tf * Vector3d(r2, 0, hl);
  result[25] = tf * Vector3d(c, d, hl);
  result[26] = tf * Vector3d(-c, d, hl);
  result[27] = tf * Vector3d(-r2, 0, hl);
  result[28] = tf * Vector3d(-c, -d, hl);
  result[29] = tf * Vector3d(c, -d, hl);

  result[30] = tf * Vector3d(r2, 0, -hl);
  result[31] = tf * Vector3d(c, d, -hl);
  result[32] = tf * Vector3d(-c, d, -hl);
  result[33] = tf * Vector3d(-r2, 0, -hl);
  result[34] = tf * Vector3d(-c, -d, -hl);
  result[35] = tf * Vector3d(c, -d, -hl);

  return result;
}


std::vector<Vector3d> getBoundVertices(const Cone& cone, const Transform3d& tf)
{
  std::vector<Vector3d> result(7);
  
  FCL_REAL hl = cone.lz * 0.5;
  FCL_REAL r2 = cone.radius * 2 / sqrt(3.0);
  FCL_REAL a = 0.5 * r2;
  FCL_REAL b = cone.radius;

  result[0] = tf * Vector3d(r2, 0, -hl);
  result[1] = tf * Vector3d(a, b, -hl);
  result[2] = tf * Vector3d(-a, b, -hl);
  result[3] = tf * Vector3d(-r2, 0, -hl);
  result[4] = tf * Vector3d(-a, -b, -hl);
  result[5] = tf * Vector3d(a, -b, -hl);

  result[6] = tf * Vector3d(0, 0, hl);
                          
  return result;
}

std::vector<Vector3d> getBoundVertices(const Cylinder& cylinder, const Transform3d& tf)
{
  std::vector<Vector3d> result(12);

  FCL_REAL hl = cylinder.lz * 0.5;
  FCL_REAL r2 = cylinder.radius * 2 / sqrt(3.0);
  FCL_REAL a = 0.5 * r2;
  FCL_REAL b = cylinder.radius;

  result[0] = tf * Vector3d(r2, 0, -hl);
  result[1] = tf * Vector3d(a, b, -hl);
  result[2] = tf * Vector3d(-a, b, -hl);
  result[3] = tf * Vector3d(-r2, 0, -hl);
  result[4] = tf * Vector3d(-a, -b, -hl);
  result[5] = tf * Vector3d(a, -b, -hl);

  result[6] = tf * Vector3d(r2, 0, hl);
  result[7] = tf * Vector3d(a, b, hl);
  result[8] = tf * Vector3d(-a, b, hl);
  result[9] = tf * Vector3d(-r2, 0, hl);
  result[10] = tf * Vector3d(-a, -b, hl);
  result[11] = tf * Vector3d(a, -b, hl);

  return result;
}

std::vector<Vector3d> getBoundVertices(const Convex& convex, const Transform3d& tf)
{
  std::vector<Vector3d> result(convex.num_points);
  for(int i = 0; i < convex.num_points; ++i)
  {
    result[i] = tf * convex.points[i];
  }

  return result;
}

std::vector<Vector3d> getBoundVertices(const TriangleP& triangle, const Transform3d& tf)
{
  std::vector<Vector3d> result(3);
  result[0] = tf * triangle.a;
  result[1] = tf * triangle.b;
  result[2] = tf * triangle.c;

  return result;
}

} // end detail

Halfspace transform(const Halfspace& a, const Transform3d& tf)
{
  /// suppose the initial halfspace is n * x <= d
  /// after transform (R, T), x --> x' = R x + T
  /// and the new half space becomes n' * x' <= d'
  /// where n' = R * n
  ///   and d' = d + n' * T

  Vector3d n = tf.linear() * a.n;
  FCL_REAL d = a.d + n.dot(tf.translation());

  return Halfspace(n, d);
}


Plane transform(const Plane& a, const Transform3d& tf)
{
  /// suppose the initial halfspace is n * x <= d
  /// after transform (R, T), x --> x' = R x + T
  /// and the new half space becomes n' * x' <= d'
  /// where n' = R * n
  ///   and d' = d + n' * T

  Vector3d n = tf.linear() * a.n;
  FCL_REAL d = a.d + n.dot(tf.translation());

  return Plane(n, d);
}



template<>
void computeBV<AABB, Box>(const Box& s, const Transform3d& tf, AABB& bv)
{
  const Matrix3d& R = tf.linear();
  const Vector3d& T = tf.translation();

  FCL_REAL x_range = 0.5 * (fabs(R(0, 0) * s.side[0]) + fabs(R(0, 1) * s.side[1]) + fabs(R(0, 2) * s.side[2]));
  FCL_REAL y_range = 0.5 * (fabs(R(1, 0) * s.side[0]) + fabs(R(1, 1) * s.side[1]) + fabs(R(1, 2) * s.side[2]));
  FCL_REAL z_range = 0.5 * (fabs(R(2, 0) * s.side[0]) + fabs(R(2, 1) * s.side[1]) + fabs(R(2, 2) * s.side[2]));

  Vector3d v_delta(x_range, y_range, z_range);
  bv.max_ = T + v_delta;
  bv.min_ = T - v_delta;
}

template<>
void computeBV<AABB, Sphere>(const Sphere& s, const Transform3d& tf, AABB& bv)
{
  const Vector3d& T = tf.translation();

  Vector3d v_delta = Vector3d::Constant(s.radius);
  bv.max_ = T + v_delta;
  bv.min_ = T - v_delta;
}

template<>
void computeBV<AABB, Ellipsoid>(const Ellipsoid& s, const Transform3d& tf, AABB& bv)
{
  const Matrix3d& R = tf.linear();
  const Vector3d& T = tf.translation();

  FCL_REAL x_range = (fabs(R(0, 0) * s.radii[0]) + fabs(R(0, 1) * s.radii[1]) + fabs(R(0, 2) * s.radii[2]));
  FCL_REAL y_range = (fabs(R(1, 0) * s.radii[0]) + fabs(R(1, 1) * s.radii[1]) + fabs(R(1, 2) * s.radii[2]));
  FCL_REAL z_range = (fabs(R(2, 0) * s.radii[0]) + fabs(R(2, 1) * s.radii[1]) + fabs(R(2, 2) * s.radii[2]));

  Vector3d v_delta(x_range, y_range, z_range);
  bv.max_ = T + v_delta;
  bv.min_ = T - v_delta;
}

template<>
void computeBV<AABB, Capsule>(const Capsule& s, const Transform3d& tf, AABB& bv)
{
  const Matrix3d& R = tf.linear();
  const Vector3d& T = tf.translation();

  FCL_REAL x_range = 0.5 * fabs(R(0, 2) * s.lz) + s.radius;
  FCL_REAL y_range = 0.5 * fabs(R(1, 2) * s.lz) + s.radius;
  FCL_REAL z_range = 0.5 * fabs(R(2, 2) * s.lz) + s.radius;

  Vector3d v_delta(x_range, y_range, z_range);
  bv.max_ = T + v_delta;
  bv.min_ = T - v_delta;
}

template<>
void computeBV<AABB, Cone>(const Cone& s, const Transform3d& tf, AABB& bv)
{
  const Matrix3d& R = tf.linear();
  const Vector3d& T = tf.translation();

  FCL_REAL x_range = fabs(R(0, 0) * s.radius) + fabs(R(0, 1) * s.radius) + 0.5 * fabs(R(0, 2) * s.lz);
  FCL_REAL y_range = fabs(R(1, 0) * s.radius) + fabs(R(1, 1) * s.radius) + 0.5 * fabs(R(1, 2) * s.lz);
  FCL_REAL z_range = fabs(R(2, 0) * s.radius) + fabs(R(2, 1) * s.radius) + 0.5 * fabs(R(2, 2) * s.lz);

  Vector3d v_delta(x_range, y_range, z_range);
  bv.max_ = T + v_delta;
  bv.min_ = T - v_delta;
}

template<>
void computeBV<AABB, Cylinder>(const Cylinder& s, const Transform3d& tf, AABB& bv)
{
  const Matrix3d& R = tf.linear();
  const Vector3d& T = tf.translation();

  FCL_REAL x_range = fabs(R(0, 0) * s.radius) + fabs(R(0, 1) * s.radius) + 0.5 * fabs(R(0, 2) * s.lz);
  FCL_REAL y_range = fabs(R(1, 0) * s.radius) + fabs(R(1, 1) * s.radius) + 0.5 * fabs(R(1, 2) * s.lz);
  FCL_REAL z_range = fabs(R(2, 0) * s.radius) + fabs(R(2, 1) * s.radius) + 0.5 * fabs(R(2, 2) * s.lz);

  Vector3d v_delta(x_range, y_range, z_range);
  bv.max_ = T + v_delta;
  bv.min_ = T - v_delta;
}

template<>
void computeBV<AABB, Convex>(const Convex& s, const Transform3d& tf, AABB& bv)
{
  const Matrix3d& R = tf.linear();
  const Vector3d& T = tf.translation();

  AABB bv_;
  for(int i = 0; i < s.num_points; ++i)
  {
    Vector3d new_p = R * s.points[i] + T;
    bv_ += new_p;
  }

  bv = bv_;
}

template<>
void computeBV<AABB, TriangleP>(const TriangleP& s, const Transform3d& tf, AABB& bv)
{
  bv = AABB(tf * s.a, tf * s.b, tf * s.c);
}


template<>
void computeBV<AABB, Halfspace>(const Halfspace& s, const Transform3d& tf, AABB& bv)
{
  Halfspace new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  AABB bv_;
  bv_.min_ = Vector3d::Constant(-std::numeric_limits<FCL_REAL>::max());
  bv_.max_ = Vector3d::Constant(std::numeric_limits<FCL_REAL>::max());
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    // normal aligned with x axis
    if(n[0] < 0) bv_.min_[0] = -d;
    else if(n[0] > 0) bv_.max_[0] = d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    // normal aligned with y axis
    if(n[1] < 0) bv_.min_[1] = -d;
    else if(n[1] > 0) bv_.max_[1] = d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    // normal aligned with z axis
    if(n[2] < 0) bv_.min_[2] = -d;
    else if(n[2] > 0) bv_.max_[2] = d;
  }

  bv = bv_;  
}

template<>
void computeBV<AABB, Plane>(const Plane& s, const Transform3d& tf, AABB& bv)
{
  Plane new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;  

  AABB bv_;
  bv_.min_ = Vector3d::Constant(-std::numeric_limits<FCL_REAL>::max());
  bv_.max_ = Vector3d::Constant(std::numeric_limits<FCL_REAL>::max());
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    // normal aligned with x axis
    if(n[0] < 0) { bv_.min_[0] = bv_.max_[0] = -d; }
    else if(n[0] > 0) { bv_.min_[0] = bv_.max_[0] = d; }
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    // normal aligned with y axis
    if(n[1] < 0) { bv_.min_[1] = bv_.max_[1] = -d; }
    else if(n[1] > 0) { bv_.min_[1] = bv_.max_[1] = d; }
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    // normal aligned with z axis
    if(n[2] < 0) { bv_.min_[2] = bv_.max_[2] = -d; }
    else if(n[2] > 0) { bv_.min_[2] = bv_.max_[2] = d; }
  }

  bv = bv_;
}


template<>
void computeBV<OBB, Box>(const Box& s, const Transform3d& tf, OBB& bv)
{
  bv.To = tf.translation();
  bv.axis = tf.linear();
  bv.extent = s.side * (FCL_REAL)0.5;
}

template<>
void computeBV<OBB, Sphere>(const Sphere& s, const Transform3d& tf, OBB& bv)
{
  bv.To = tf.translation();
  bv.axis.setIdentity();
  bv.extent.setConstant(s.radius);
}

template<>
void computeBV<OBB, Ellipsoid>(const Ellipsoid& s, const Transform3d& tf, OBB& bv)
{
  bv.To = tf.translation();
  bv.axis = tf.linear();
  bv.extent = s.radii;
}

template<>
void computeBV<OBB, Capsule>(const Capsule& s, const Transform3d& tf, OBB& bv)
{
  bv.To = tf.translation();
  bv.axis = tf.linear();
  bv.extent << s.radius, s.radius, s.lz / 2 + s.radius;
}

template<>
void computeBV<OBB, Cone>(const Cone& s, const Transform3d& tf, OBB& bv)
{
  bv.To = tf.translation();
  bv.axis = tf.linear();
  bv.extent << s.radius, s.radius, s.lz / 2;
}

template<>
void computeBV<OBB, Cylinder>(const Cylinder& s, const Transform3d& tf, OBB& bv)
{
  bv.To = tf.translation();
  bv.axis = tf.linear();
  bv.extent << s.radius, s.radius, s.lz / 2;
}

template<>
void computeBV<OBB, Convex>(const Convex& s, const Transform3d& tf, OBB& bv)
{
  fit(s.points, s.num_points, bv);

  bv.axis = tf.linear();
  bv.To = tf * bv.To;
}

template<>
void computeBV<OBB, Halfspace>(const Halfspace& s, const Transform3d& tf, OBB& bv)
{
  /// Half space can only have very rough OBB
  bv.axis.setIdentity();
  bv.To.setZero();
  bv.extent.setConstant(std::numeric_limits<FCL_REAL>::max());
}

template<>
void computeBV<RSS, Halfspace>(const Halfspace& s, const Transform3d& tf, RSS& bv)
{
  /// Half space can only have very rough RSS
  bv.axis.setIdentity();
  bv.Tr.setZero();
  bv.l[0] = bv.l[1] = bv.r = std::numeric_limits<FCL_REAL>::max();
}

template<>
void computeBV<OBBRSS, Halfspace>(const Halfspace& s, const Transform3d& tf, OBBRSS& bv)
{
  computeBV<OBB, Halfspace>(s, tf, bv.obb);
  computeBV<RSS, Halfspace>(s, tf, bv.rss);
}

template<>
void computeBV<kIOS, Halfspace>(const Halfspace& s, const Transform3d& tf, kIOS& bv)
{
  bv.num_spheres = 1;
  computeBV<OBB, Halfspace>(s, tf, bv.obb);
  bv.spheres[0].o.setZero();
  bv.spheres[0].r = std::numeric_limits<FCL_REAL>::max();
}

template<>
void computeBV<KDOP<16>, Halfspace>(const Halfspace& s, const Transform3d& tf, KDOP<16>& bv)
{
  Halfspace new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  const std::size_t D = 8;
  for(std::size_t i = 0; i < D; ++i)
    bv.dist(i) = -std::numeric_limits<FCL_REAL>::max();
  for(std::size_t i = D; i < 2 * D; ++i)
    bv.dist(i) = std::numeric_limits<FCL_REAL>::max();
  
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D) = d;
    else bv.dist(0) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(D + 1) = d;
    else bv.dist(1) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    if(n[2] > 0) bv.dist(D + 2) = d;
    else bv.dist(2) = -d;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] == n[1])
  {
    if(n[0] > 0) bv.dist(D + 3) = n[0] * d * 2;
    else bv.dist(3) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] == n[2])
  {
    if(n[1] > 0) bv.dist(D + 4) = n[0] * d * 2;
    else bv.dist(4) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == n[2])
  {
    if(n[1] > 0) bv.dist(D + 5) = n[1] * d * 2;
    else bv.dist(5) = n[1] * d * 2;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 6) = n[0] * d * 2;
    else bv.dist(6) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 7) = n[0] * d * 2;
    else bv.dist(7) = n[0] * d * 2;
  }
}

template<>
void computeBV<KDOP<18>, Halfspace>(const Halfspace& s, const Transform3d& tf, KDOP<18>& bv)
{
  Halfspace new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  const std::size_t D = 9;

  for(std::size_t i = 0; i < D; ++i)
    bv.dist(i) = -std::numeric_limits<FCL_REAL>::max();
  for(std::size_t i = D; i < 2 * D; ++i)
    bv.dist(i) = std::numeric_limits<FCL_REAL>::max();
  
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D) = d;
    else bv.dist(0) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(D + 1) = d;
    else bv.dist(1) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    if(n[2] > 0) bv.dist(D + 2) = d;
    else bv.dist(2) = -d;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] == n[1])
  {
    if(n[0] > 0) bv.dist(D + 3) = n[0] * d * 2;
    else bv.dist(3) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] == n[2])
  {
    if(n[1] > 0) bv.dist(D + 4) = n[0] * d * 2;
    else bv.dist(4) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == n[2])
  {
    if(n[1] > 0) bv.dist(D + 5) = n[1] * d * 2;
    else bv.dist(5) = n[1] * d * 2;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 6) = n[0] * d * 2;
    else bv.dist(6) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 7) = n[0] * d * 2;
    else bv.dist(7) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] + n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(D + 8) = n[1] * d * 2;
    else bv.dist(8) = n[1] * d * 2;
  }
}

template<>
void computeBV<KDOP<24>, Halfspace>(const Halfspace& s, const Transform3d& tf, KDOP<24>& bv)
{
  Halfspace new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  const std::size_t D = 12;

  for(std::size_t i = 0; i < D; ++i)
    bv.dist(i) = -std::numeric_limits<FCL_REAL>::max();
  for(std::size_t i = D; i < 2 * D; ++i)
    bv.dist(i) = std::numeric_limits<FCL_REAL>::max();
  
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D) = d;
    else bv.dist(0) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(D + 1) = d;
    else bv.dist(1) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    if(n[2] > 0) bv.dist(D + 2) = d;
    else bv.dist(2) = -d;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] == n[1])
  {
    if(n[0] > 0) bv.dist(D + 3) = n[0] * d * 2;
    else bv.dist(3) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] == n[2])
  {
    if(n[1] > 0) bv.dist(D + 4) = n[0] * d * 2;
    else bv.dist(4) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == n[2])
  {
    if(n[1] > 0) bv.dist(D + 5) = n[1] * d * 2;
    else bv.dist(5) = n[1] * d * 2;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 6) = n[0] * d * 2;
    else bv.dist(6) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 7) = n[0] * d * 2;
    else bv.dist(7) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] + n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(D + 8) = n[1] * d * 2;
    else bv.dist(8) = n[1] * d * 2;
  }
  else if(n[0] + n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 9) = n[0] * d * 3;
    else bv.dist(9) = n[0] * d * 3;
  }
  else if(n[0] + n[1] == (FCL_REAL)0.0 && n[1] + n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(D + 10) = n[0] * d * 3;
    else bv.dist(10) = n[0] * d * 3;
  }
  else if(n[0] + n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(D + 11) = n[1] * d * 3;
    else bv.dist(11) = n[1] * d * 3;
  }
}



template<>
void computeBV<OBB, Plane>(const Plane& s, const Transform3d& tf, OBB& bv)
{
  Vector3d n = tf.linear() * s.n;
  bv.axis.col(0) = n;
  generateCoordinateSystem(bv.axis);

  bv.extent << 0, std::numeric_limits<FCL_REAL>::max(), std::numeric_limits<FCL_REAL>::max();

  Vector3d p = s.n * s.d; 
  bv.To = tf * p; /// n'd' = R * n * (d + (R * n) * T) = R * (n * d) + T
}

template<>
void computeBV<RSS, Plane>(const Plane& s, const Transform3d& tf, RSS& bv)
{
  Vector3d n = tf.linear() * s.n;

  bv.axis.col(0) = n;
  generateCoordinateSystem(bv.axis);

  bv.l[0] = std::numeric_limits<FCL_REAL>::max();
  bv.l[1] = std::numeric_limits<FCL_REAL>::max();

  bv.r = 0;
  
  Vector3d p = s.n * s.d;
  bv.Tr = tf * p;
}

template<>
void computeBV<OBBRSS, Plane>(const Plane& s, const Transform3d& tf, OBBRSS& bv)
{
  computeBV<OBB, Plane>(s, tf, bv.obb);
  computeBV<RSS, Plane>(s, tf, bv.rss);
}

template<>
void computeBV<kIOS, Plane>(const Plane& s, const Transform3d& tf, kIOS& bv)
{
  bv.num_spheres = 1;
  computeBV<OBB, Plane>(s, tf, bv.obb);
  bv.spheres[0].o.setZero();
  bv.spheres[0].r = std::numeric_limits<FCL_REAL>::max();
}

template<>
void computeBV<KDOP<16>, Plane>(const Plane& s, const Transform3d& tf, KDOP<16>& bv)
{
  Plane new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  const std::size_t D = 8;

  for(std::size_t i = 0; i < D; ++i)
    bv.dist(i) = -std::numeric_limits<FCL_REAL>::max();
  for(std::size_t i = D; i < 2 * D; ++i)
    bv.dist(i) = std::numeric_limits<FCL_REAL>::max();
  
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(0) = bv.dist(D) = d;
    else bv.dist(0) = bv.dist(D) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(1) = bv.dist(D + 1) = d;
    else bv.dist(1) = bv.dist(D + 1) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    if(n[2] > 0) bv.dist(2) = bv.dist(D + 2) = d;
    else bv.dist(2) = bv.dist(D + 2) = -d;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] == n[1])
  {
    bv.dist(3) = bv.dist(D + 3) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] == n[2])
  {
    bv.dist(4) = bv.dist(D + 4) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == n[2])
  {
    bv.dist(6) = bv.dist(D + 5) = n[1] * d * 2;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    bv.dist(6) = bv.dist(D + 6) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(7) = bv.dist(D + 7) = n[0] * d * 2;
  }
}

template<>
void computeBV<KDOP<18>, Plane>(const Plane& s, const Transform3d& tf, KDOP<18>& bv)
{
  Plane new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  const std::size_t D = 9;

  for(std::size_t i = 0; i < D; ++i)
    bv.dist(i) = -std::numeric_limits<FCL_REAL>::max();
  for(std::size_t i = D; i < 2 * D; ++i)
    bv.dist(i) = std::numeric_limits<FCL_REAL>::max();
  
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(0) = bv.dist(D) = d;
    else bv.dist(0) = bv.dist(D) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(1) = bv.dist(D + 1) = d;
    else bv.dist(1) = bv.dist(D + 1) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    if(n[2] > 0) bv.dist(2) = bv.dist(D + 2) = d;
    else bv.dist(2) = bv.dist(D + 2) = -d;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] == n[1])
  {
    bv.dist(3) = bv.dist(D + 3) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] == n[2])
  {
    bv.dist(4) = bv.dist(D + 4) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == n[2])
  {
    bv.dist(5) = bv.dist(D + 5) = n[1] * d * 2;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    bv.dist(6) = bv.dist(D + 6) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(7) = bv.dist(D + 7) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(8) = bv.dist(D + 8) = n[1] * d * 2;
  }
}

template<>
void computeBV<KDOP<24>, Plane>(const Plane& s, const Transform3d& tf, KDOP<24>& bv)
{
  Plane new_s = transform(s, tf);
  const Vector3d& n = new_s.n;
  const FCL_REAL& d = new_s.d;

  const std::size_t D = 12;

  for(std::size_t i = 0; i < D; ++i)
    bv.dist(i) = -std::numeric_limits<FCL_REAL>::max();
  for(std::size_t i = D; i < 2 * D; ++i)
    bv.dist(i) = std::numeric_limits<FCL_REAL>::max();
  
  if(n[1] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[0] > 0) bv.dist(0) = bv.dist(D) = d;
    else bv.dist(0) = bv.dist(D) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[2] == (FCL_REAL)0.0)
  {
    if(n[1] > 0) bv.dist(1) = bv.dist(D + 1) = d;
    else bv.dist(1) = bv.dist(D + 1) = -d;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == (FCL_REAL)0.0)
  {
    if(n[2] > 0) bv.dist(2) = bv.dist(D + 2) = d;
    else bv.dist(2) = bv.dist(D + 2) = -d;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] == n[1])
  {
    bv.dist(3) = bv.dist(D + 3) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] == n[2])
  {
    bv.dist(4) = bv.dist(D + 4) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] == n[2])
  {
    bv.dist(5) = bv.dist(D + 5) = n[1] * d * 2;
  }
  else if(n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    bv.dist(6) = bv.dist(D + 6) = n[0] * d * 2;
  }
  else if(n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(7) = bv.dist(D + 7) = n[0] * d * 2;
  }
  else if(n[0] == (FCL_REAL)0.0 && n[1] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(8) = bv.dist(D + 8) = n[1] * d * 2;
  }
  else if(n[0] + n[2] == (FCL_REAL)0.0 && n[0] + n[1] == (FCL_REAL)0.0)
  {
    bv.dist(9) = bv.dist(D + 9) = n[0] * d * 3;
  }
  else if(n[0] + n[1] == (FCL_REAL)0.0 && n[1] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(10) = bv.dist(D + 10) = n[0] * d * 3;
  }
  else if(n[0] + n[1] == (FCL_REAL)0.0 && n[0] + n[2] == (FCL_REAL)0.0)
  {
    bv.dist(11) = bv.dist(D + 11) = n[1] * d * 3;
  }
}


void constructBox(const AABB& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.max_ - bv.min_);
  tf.linear().setIdentity();
  tf.translation() = bv.center();
}

void constructBox(const OBB& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.extent * 2);
  tf.linear() = bv.axis;
  tf.translation() = bv.To;
}

void constructBox(const OBBRSS& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.obb.extent * 2);
  tf.linear() = bv.obb.axis;
  tf.translation() = bv.obb.To;
}

void constructBox(const kIOS& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.obb.extent * 2);
  tf.linear() = bv.obb.axis;
  tf.translation() = bv.obb.To;
}

void constructBox(const RSS& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf.linear() = bv.axis;
  tf.translation() = bv.Tr;
}

void constructBox(const KDOP<16>& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf.linear().setIdentity();
  tf.translation() = bv.center();
}

void constructBox(const KDOP<18>& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf.linear().setIdentity();
  tf.translation() = bv.center();
}

void constructBox(const KDOP<24>& bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf.linear().setIdentity();
  tf.translation() = bv.center();
}



void constructBox(const AABB& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.max_ - bv.min_);
  tf = tf_bv * Eigen::Translation3d(bv.center());
}

void constructBox(const OBB& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.extent * 2);
  tf.linear() = bv.axis;
  tf.translation() = bv.To;
}

void constructBox(const OBBRSS& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.obb.extent * 2);
  tf.linear() = bv.obb.axis;
  tf.translation() = bv.obb.To;
  tf = tf_bv * tf;
}

void constructBox(const kIOS& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.obb.extent * 2);
  tf.linear() = bv.obb.axis;
  tf.translation() = bv.obb.To;
  tf = tf_bv * tf;
}

void constructBox(const RSS& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf.linear() = bv.axis;
  tf.translation() = bv.Tr;
  tf = tf_bv * tf;
}

void constructBox(const KDOP<16>& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf = tf_bv * Eigen::Translation3d(bv.center());
}

void constructBox(const KDOP<18>& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf = tf_bv * Eigen::Translation3d(bv.center());
}

void constructBox(const KDOP<24>& bv, const Transform3d& tf_bv, Box& box, Transform3d& tf)
{
  box = Box(bv.width(), bv.height(), bv.depth());
  tf = tf_bv * Eigen::Translation3d(bv.center());
}



}
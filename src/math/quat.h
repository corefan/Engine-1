#pragma once

#include "math/dll.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat44.h"

namespace Math
{
	/**
	 * Quaternion.
	 */
	class MATH_DLL Quat final
	{
	public:
		f32 x, y, z, w;

	public:
		// ctor
		Quat();
		Quat(f32* Val);
		Quat(f32 lX, f32 lY, f32 lZ, f32 lW);

		// Arithmetic
		Quat operator*(const Quat& rhs) const;
		Quat operator~() const { return Quat(-x, -y, -z, w); }


		// Additional stuff
		void MakeIdentity();
		f32 Magnitude();
		f32 MagnitudeSquared() const { return Dot(*this); }
		f32 Dot(const Quat& Rhs) const { return (x * Rhs.x) + (y * Rhs.y) + (z * Rhs.z) + (w * Rhs.w); }
		void Normalise();
		void Inverse();

		// Interpolation
		static Quat Slerp(const Quat& a, const Quat& b, f32 t);

		Vec3 RotateVector(const Vec3&) const;

		void FromAxis(const Vec3& X, const Vec3& Y, const Vec3& Z);

		//
		void FromMatrix4d(const Mat44& Mat) { FromAxis(Mat.Row0().xyz(), Mat.Row1().xyz(), Mat.Row2().xyz()); }
		void AsMatrix4d(Mat44& Matrix) const;

		//
		void AxisAngle(const Vec3& Axis, f32 Angle);

		//
		void FromEuler(f32 Yaw, f32 Pitch, f32 Roll);
		Vec3 AsEuler() const;

		//
		void CalcFromXYZ();
	};

	// ctor
	inline Quat::Quat()
	{ //
		MakeIdentity();
	}

	inline Quat::Quat(f32* Val)
	    : x(Val[0])
	    , y(Val[1])
	    , z(Val[2])
	    , w(Val[3])
	{
	}

	inline Quat::Quat(f32 lX, f32 lY, f32 lZ, f32 lW)
	    : x(lX)
	    , y(lY)
	    , z(lZ)
	    , w(lW)
	{
	}
} // namespace Math
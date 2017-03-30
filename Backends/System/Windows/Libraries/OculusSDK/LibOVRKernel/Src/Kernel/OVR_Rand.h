/************************************************************************************

Filename    :   OVR_Rand.h
Content     :   Random number generator
Created     :   Aug 28, 2014
Author      :   Chris Taylor

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License"); 
you may not use the Oculus VR Rift SDK except in compliance with the License, 
which is provided at the time of installation or download, or which 
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3 

Unless required by applicable law or agreed to in writing, the Oculus VR SDK 
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_Rand_h
#define OVR_Rand_h

#include "OVR_Types.h"
#include <math.h>

namespace OVR {


/*
 This is designed to generate up to 2^^32 numbers per seed.

 Its period is about 2^^126 and passes all BigCrush tests.
 It is the fastest simple generator that passes all tests.

 It has a few advantages over the stdlib RNG, including that
 all bits of the output are equally good in quality.
 Furthermore, the input seeds are hashed to avoid linear
 relationships between the input seeds and the low bits of
 the first few outputs.
*/
class RandomNumberGenerator
{
protected:
    uint64_t Rx;
    uint64_t Ry;
    double NextRandN;
    bool Seeded;
    bool HaveRandN;

public:
    RandomNumberGenerator() :
        Seeded(false),
        HaveRandN(false)
        // Other members left uninitialized
    {
    }

    bool IsSeeded() const
    {
        return Seeded;
    }

    // Seed with a random state
    void SeedRandom();

    // Start with a specific seed
    void Seed(uint32_t x, uint32_t y);

    // Returns an unsigned uint32_t uniformly distributed in interval [0..2^32-1]
    OVR_FORCE_INLINE uint32_t Next()
    {
        // If it is not Seeded yet,
        if (!IsSeeded())
        {
            SeedRandom();
        }

        // Sum of two long-period MWC generators
        Rx = (uint64_t)0xfffd21a7 * (uint32_t)Rx + (uint32_t)(Rx >> 32);
        Ry = (uint64_t)0xfffd1361 * (uint32_t)Ry + (uint32_t)(Ry >> 32);
        return (((uint32_t)Rx << 7) | ((uint32_t)Rx >> (32 - 7))) + (uint32_t)Ry; // ROL(x, 7) + y
    }

    // The following functions are inspired by the Matlab functions rand(), randi(), and randn()

    // Uniform distribution over open interval (0..2^32)
    // (i.e. closed interval [1..2^32-1], not including 0)
    OVR_FORCE_INLINE uint32_t NextNonZero()
    {
        uint32_t n;
        do
        {
            n = Next();
        } while (n == 0);
        return n;
    }

    // Double uniformly distributed over open interval (0..1)
    OVR_FORCE_INLINE double Rand()
    {
        return NextNonZero() * (1.0 / 4294967296.0);    // 2^32
    }

    // Double uniformly distributed over open interval (dmin..dmax)
    OVR_FORCE_INLINE double Rand(double dmin, double dmax)
    {
        return dmin + (dmax - dmin) * (1.0 / 4294967296.0) * NextNonZero(); // 2^32
    }

    // Integer uniformly distributed over closed interval [0..imax-1]
    // (NOTE: Matalb randi(imax) returns 1..imax)
    OVR_FORCE_INLINE int RandI(int imax)
    {
        return (int)(Next() % imax);
    }

    // Integer uniformly distributed over closed interval [imin..imax-1]
    // (NOTE: Matlab randi() returns imin..imax)
    OVR_FORCE_INLINE int RandI(int imin, int imax)
    {
        return imin + (int)(Next() % (imax - imin));
    }

    // Coordinate (x,y) uniformly distributed inside unit circle.
    // Returns magnitude squared of (x,y)
    OVR_FORCE_INLINE double RandInUnitCircle(double& x, double& y)
    {
        double r2;
        do
        {
            x = Rand(-1.0, 1.0);
            y = Rand(-1.0, 1.0);
            r2 = x*x + y*y;
        } while (r2 >= 1.0);
        return r2;
    }

    // Standard normal (gaussian) distribution: mean 0.0, stdev 1.0
    double RandN()
    {
        if (HaveRandN)
        {
            HaveRandN = false;
            return NextRandN;
        }
        else
        {
            double x, y;
            double r2 = RandInUnitCircle(x, y);
            // Box-Muller transform
            double f = sqrt(-2 * log(r2) / r2);

            // Return first, save second for next call
            NextRandN = y * f;
            HaveRandN = true;

            return x * f;
        }
    }

    // Uniform coordinate (c,s) ON unit circle.
    // This function computes cos(theta), sin(theta)
    // of rotation uniform in (0..2*pi).
    // [ c s] is a random 2D rotation matrix.
    // [-s c]
    // Reference: Numerical Recipes in C++, chap. 21
    OVR_FORCE_INLINE void RandOnUnitCircle(double& c, double& s)
    {
        double r2 = RandInUnitCircle(c, s);
        double normalize = 1.0 / sqrt(r2);
        c *= normalize;
        s *= normalize;
    }

    // Uniform coordinate (x,y,z,w) on surface of 4D hypersphere.
    // This is a quaternion rotation uniformly distributed across all rotations
    // Reference: Numerical Recipes in C++, chap. 21
    OVR_FORCE_INLINE void RandOnUnitSphere4(double& x, double& y, double& z, double& w)
    {
        double r2xy = RandInUnitCircle(x, y);

        double u, v;
        double r2uv = RandInUnitCircle(u, v);

        double f = sqrt((1.0 - r2xy) / r2uv);
        z = u * f;
        w = v * f;
    }
};

} // namespace OVR

#endif // OVR_Rand_h

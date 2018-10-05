#pragma once
/*
    Project includes
*/

/*
    Lib includes
*/

/*
    STD/C++ includes
*/
#include <cmath>

///////////////////////////////////////////////////////////////////////////////

namespace math
{
    namespace vector
    {
        template <typename T> union Vector3
        {
            struct { T x, y, z; };
            T v[3];
        };

        ////////////////////////////////////////////////////////////

        template <typename T> inline bool           operator ==    (const Vector3<T> &a, const Vector3<T> &b) { return (a.x == b.x) && (a.y == b.y) && (a.z == b.z); }
        template <typename T> inline bool           operator !=    (const Vector3<T> &a, const Vector3<T> &b) { return !operator==(a, b); }

        template <typename T> inline Vector3<T>     operator +     (const Vector3<T> &a) { return a; }
        template <typename T> inline Vector3<T>     operator -     (const Vector3<T> &a) { Vector3<T> r = { -a.x, -a.y, -a.z }; return r; }

        template <typename T> inline Vector3<T>     operator +     (const Vector3<T> &a, const Vector3<T> &b) { Vector3<T> r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
        template <typename T> inline Vector3<T>     operator -     (const Vector3<T> &a, const Vector3<T> &b) { Vector3<T> r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }

        template <typename T> inline Vector3<T>     operator *     (const Vector3<T> &a, T scalar) { Vector3<T> r = { a.x * scalar, a.y * scalar, a.z * scalar }; return r; }
        template <typename T> inline Vector3<T>     operator *     (T scalar, const Vector3<T> &a) { return a * scalar; }

        template <typename T> inline Vector3<T>     operator /     (const Vector3<T> &a, T scalar) { Vector3<T> r = { a.x / scalar, a.y / scalar, a.z / scalar }; return r; }

        template <typename T> inline Vector3<T>     operator *     (const Vector3<T> &a, const Vector3<T> &b) { Vector3<T> r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
        template <typename T> inline Vector3<T>     operator /     (const Vector3<T> &a, const Vector3<T> &b) { Vector3<T> r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }

        template <typename T> inline Vector3<T>     &operator +=   (Vector3<T> &a, const Vector3<T> &b) { return (a = a + b); }
        template <typename T> inline Vector3<T>     &operator -=   (Vector3<T> &a, const Vector3<T> &b) { return (a = a - b); }

        template <typename T> inline Vector3<T>     &operator *=   (Vector3<T> &a, const Vector3<T> &b) { return (a = a * b); }
        template <typename T> inline Vector3<T>     &operator /=   (Vector3<T> &a, const Vector3<T> &b) { return (a = a / b); }

        template <typename T> inline Vector3<T>     &operator *=   (Vector3<T> &a, T scalar) { return (a = a * scalar); }
        template <typename T> inline Vector3<T>     &operator /=   (Vector3<T> &a, T scalar) { return (a = a / scalar); }

        ////////////////////////////////////////////////////////////

        template <typename T> inline Vector3<T> normalize(const Vector3<T> &a)
        {
            // NOTE(Andrei): No division by zero check
            T inversLength = 1.0 / length(a);

            Vector3<T> result = { a.x * inversLength, a.y * inversLength, a.z * inversLength };
            return result;
        }

        template <typename T> inline void normalize(Vector3<T> &a)
        {
            // NOTE(Andrei): No division by zero check
            T inversLength = 1.0 / length(a);
            a = a * inversLength;
        }

        ////////////////////////////////////////////////////////////

        template <typename T> inline T length(const Vector3<T> &a)
        {
            T result = std::sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
            return result;
        }

        template <typename T> inline T distance(const Vector3<T> &a, const Vector3<T> &b)
        {
            T result = std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z));
            return result;
        }
    }
}

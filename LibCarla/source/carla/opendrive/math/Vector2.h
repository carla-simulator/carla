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
        template <typename T> union Vector2
        {
            struct { T x, y; };
            T v[2];
        };

        ////////////////////////////////////////////////////////////

        template <typename T> inline bool           operator ==    (const Vector2<T> &a, const Vector2<T> &b) { return (a.x == b.x) && (a.y == b.y); }
        template <typename T> inline bool           operator !=    (const Vector2<T> &a, const Vector2<T> &b) { return !operator==(a, b); }

        template <typename T> inline Vector2<T>     operator +     (const Vector2<T> &a) { return a; }
        template <typename T> inline Vector2<T>     operator -     (const Vector2<T> &a) { Vector2<T> r = { -a.x, -a.y }; return r; }

        template <typename T> inline Vector2<T>     operator +     (const Vector2<T> &a, const Vector2<T> &b) { Vector2<T> r = { a.x + b.x, a.y + b.y }; return r; }
        template <typename T> inline Vector2<T>     operator -     (const Vector2<T> &a, const Vector2<T> &b) { Vector2<T> r = { a.x - b.x, a.y - b.y }; return r; }

        template <typename T> inline Vector2<T>     operator *     (const Vector2<T> &a, T scalar) { Vector2<T> r = { a.x * scalar, a.y * scalar }; return r; }
        template <typename T> inline Vector2<T>     operator *     (T scalar, const Vector2<T> &a) { return a * scalar; }

        template <typename T> inline Vector2<T>     operator /     (const Vector2<T> &a, T scalar) { Vector2<T> r = { a.x / scalar, a.y / scalar }; return r; }

        template <typename T> inline Vector2<T>     operator *     (const Vector2<T> &a, const Vector2<T> &b) { Vector2<T> r = { a.x * b.x, a.y * b.y }; return r; }
        template <typename T> inline Vector2<T>     operator /     (const Vector2<T> &a, const Vector2<T> &b) { Vector2<T> r = { a.x / b.x, a.y / b.y }; return r; }

        template <typename T> inline Vector2<T>     &operator +=   (Vector2<T> &a, const Vector2<T> &b) { return (a = a + b); }
        template <typename T> inline Vector2<T>     &operator -=   (Vector2<T> &a, const Vector2<T> &b) { return (a = a - b); }

        template <typename T> inline Vector2<T>     &operator *=   (Vector2<T> &a, const Vector2<T> &b) { return (a = a * b); }
        template <typename T> inline Vector2<T>     &operator /=   (Vector2<T> &a, const Vector2<T> &b) { return (a = a / b); }

        template <typename T> inline Vector2<T>     &operator *=   (Vector2<T> &a, T scalar) { return (a = a * scalar); }
        template <typename T> inline Vector2<T>     &operator /=   (Vector2<T> &a, T scalar) { return (a = a / scalar); }

        ////////////////////////////////////////////////////////////

        template <typename T> inline Vector2<T> normalVector(const Vector2<T> &a)
        {
            Vector2<T> result = { a.y, -a.x };
            return result;
        }

        ////////////////////////////////////////////////////////////

        template <typename T> inline Vector2<T> normalize(const Vector2<T> &a)
        {
            // NOTE(Andrei): No division by zero check
            T inversLength = 1.0 / length(a);

            Vector2<T> result = { a.x * inversLength, a.y * inversLength };
            return result;
        }

        template <typename T> inline void normalize(Vector2<T> &a)
        {
            // NOTE(Andrei): No division by zero check
            T inversLength = 1.0 / length(a);
            a = a * inversLength;
        }

        ////////////////////////////////////////////////////////////

        template <typename T> inline T length(const Vector2<T> &a)
        {
            T result = std::sqrt((a.x * a.x) + (a.y * a.y));
            return result;
        }

        template <typename T> inline T distance(const Vector2<T> &a, const Vector2<T> &b)
        {
            T result = std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
            return result;
        }
    }
}

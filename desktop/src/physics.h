#pragma once
#include <cmath>

// ---- 基础向量 ----
struct Vec3 {
    double x = 0.0, y = 0.0, z = 0.0;
    Vec3() = default;
    Vec3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(double s) const { return { x * s, y * s, z * s }; }
    double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    double length() const { return std::sqrt(dot(*this)); }
    Vec3 normalized() const { double l = length(); return l > 1e-12 ? (*this) * (1.0 / l) : Vec3{}; }
};

struct State {
    Vec3 pos;   // km
    Vec3 vel;   // km/s
};

// ---- 常数（km, s）----
constexpr double PI = 3.14159265358979323846;
constexpr double MU_EARTH    = 398600.4418;      // km^3/s^2
constexpr double MU_MOON     = 4902.8;           // km^3/s^2
constexpr double R_EARTH     = 6371.0;           // km
constexpr double R_MOON      = 1737.4;           // km
constexpr double MOON_DIST   = 384400.0;         // km
constexpr double MOON_PERIOD = 27.321661 * 86400.0; // s
constexpr double MOON_OMEGA  = 2.0 * PI / MOON_PERIOD; // rad/s

// ---- 函数 ----
Vec3  moonPosKm(double t);
Vec3  accel(const Vec3& pos, double t);
State rk4Step(const State& s, double t, double dt);
Vec3  velocityFromDir(double speed, double azDeg, double elevDeg);
State elementsToState(double a, double e, double incDeg, double raanDeg, double argpDeg, double nuDeg);
double circularSpeed(double r);

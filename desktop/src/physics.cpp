#include "physics.h"

// 状态对时间的导数（dpos = 速度，dvel = 加速度）
struct Deriv { Vec3 dpos, dvel; };

Vec3 moonPosKm(double t) {
    double a = MOON_OMEGA * t;
    return { MOON_DIST * std::cos(a), MOON_DIST * std::sin(a), 0.0 };
}

Vec3 accel(const Vec3& p, double t) {
    Vec3 mp = moonPosKm(t);
    Vec3 de = p;          // 地球固定在原点
    Vec3 dm = p - mp;     // 相对月球
    double de2 = de.dot(de), dm2 = dm.dot(dm);
    double de_ = std::sqrt(de2), dm_ = std::sqrt(dm2);
    double ae = MU_EARTH / (de2 * de_);
    double am = MU_MOON / (dm2 * dm_);
    return de * (-ae) + dm * (-am);
}

State rk4Step(const State& s, double t, double dt) {
    auto deriv = [](const State& st, double tt) -> Deriv {
        return { st.vel, accel(st.pos, tt) };
    };
    Deriv k1 = deriv(s, t);
    State s2{ s.pos + k1.dpos * (dt / 2), s.vel + k1.dvel * (dt / 2) };
    Deriv k2 = deriv(s2, t + dt / 2);
    State s3{ s.pos + k2.dpos * (dt / 2), s.vel + k2.dvel * (dt / 2) };
    Deriv k3 = deriv(s3, t + dt / 2);
    State s4{ s.pos + k3.dpos * dt, s.vel + k3.dvel * dt };
    Deriv k4 = deriv(s4, t + dt);
    return {
        s.pos + (k1.dpos + k2.dpos * 2 + k3.dpos * 2 + k4.dpos) * (dt / 6),
        s.vel + (k1.dvel + k2.dvel * 2 + k3.dvel * 2 + k4.dvel) * (dt / 6)
    };
}

Vec3 velocityFromDir(double speed, double azDeg, double elevDeg) {
    double az = azDeg * PI / 180.0, el = elevDeg * PI / 180.0;
    return {
        speed * std::cos(el) * std::cos(az),
        speed * std::cos(el) * std::sin(az),
        speed * std::sin(el)
    };
}

static Vec3 rot3(double th, const Vec3& v) {
    double c = std::cos(th), s = std::sin(th);
    return { c * v.x - s * v.y, s * v.x + c * v.y, v.z };
}
static Vec3 rot1(double th, const Vec3& v) {
    double c = std::cos(th), s = std::sin(th);
    return { v.x, c * v.y - s * v.z, s * v.y + c * v.z };
}

State elementsToState(double a, double e, double incDeg, double raanDeg, double argpDeg, double nuDeg) {
    double inc = incDeg * PI / 180.0, raan = raanDeg * PI / 180.0;
    double argp = argpDeg * PI / 180.0, nu = nuDeg * PI / 180.0;
    double p = a * (1.0 - e * e);
    double r = p / (1.0 + e * std::cos(nu));
    Vec3 rp{ r * std::cos(nu), r * std::sin(nu), 0.0 };
    Vec3 vp{
        -std::sqrt(MU_EARTH / p) * std::sin(nu),
         std::sqrt(MU_EARTH / p) * (e + std::cos(nu)),
        0.0
    };
    Vec3 pos = rot3(raan, rot1(inc, rot3(argp, rp)));
    Vec3 vel = rot3(raan, rot1(inc, rot3(argp, vp)));
    return { pos, vel };
}

double circularSpeed(double r) { return std::sqrt(MU_EARTH / r); }

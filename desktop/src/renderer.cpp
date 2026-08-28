#include "renderer.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QVector4D>
#include <random>
#include <algorithm>
#include <cmath>

static constexpr float SC = 0.001f;        // km -> 场景单位（1 单位 = 1000 km）
static constexpr float ARROW_SCALE = 0.65f;

static const char* kLitVS = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMat;
out vec3 vNormal;
out vec2 vUV;
void main(){
    vNormal = uNormalMat * aNormal;
    vUV = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char* kLitFS = R"(
#version 330 core
in vec3 vNormal;
in vec2 vUV;
out vec4 fragColor;
uniform sampler2D uTex;
uniform vec3 uLightDir;
void main(){
    vec3 n = normalize(vNormal);
    float diff = max(dot(n, uLightDir), 0.0);
    vec3 base = texture(uTex, vUV).rgb;
    vec3 c = base * (0.25 + 0.85 * diff);
    fragColor = vec4(c, 1.0);
}
)";

static const char* kColorVS = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main(){ gl_Position = uMVP * vec4(aPos, 1.0); }
)";

static const char* kColorFS = R"(
#version 330 core
out vec4 fragColor;
uniform vec4 uColor;
void main(){ fragColor = uColor; }
)";

static const char* kTrailVS = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
out vec3 vColor;
uniform mat4 uMVP;
void main(){ vColor = aColor; gl_Position = uMVP * vec4(aPos, 1.0); }
)";

static const char* kTrailFS = R"(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main(){ fragColor = vec4(vColor, 1.0); }
)";

Renderer::Renderer(QWidget* parent) : QOpenGLWidget(parent) {
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(400, 300);
}

Renderer::~Renderer() {
    makeCurrent();
    if (m_earthTex) glDeleteTextures(1, &m_earthTex);
    if (m_moonTex)  glDeleteTextures(1, &m_moonTex);
    if (m_whiteTex) glDeleteTextures(1, &m_whiteTex);
    m_sphereVao.destroy(); m_sphereVbo.destroy(); m_sphereIbo.destroy();
    m_trailVao.destroy(); m_trailVbo.destroy(); m_trailColorVbo.destroy();
    m_barVao.destroy(); m_barVbo.destroy();
    m_arrowVao.destroy(); m_arrowVbo.destroy();
    m_ringVao.destroy(); m_ringVbo.destroy();
    m_gridVao.destroy(); m_gridVbo.destroy();
    m_starVao.destroy(); m_starVbo.destroy();
    delete m_lit; delete m_color; delete m_trail;
    doneCurrent();
}

void Renderer::setScene(const SceneData& d) { m_scene = d; update(); }

void Renderer::resetView() {
    m_target = QVector3D(150, 0, 0);
    m_distance = 700.0f;
    m_yaw = -25.0f; m_pitch = 30.0f;
    update();
}

void Renderer::initShader(QOpenGLShaderProgram* p, const char* vs, const char* fs) {
    p->addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    p->addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
    p->link();
}

void Renderer::initializeGL() {
    initializeOpenGLFunctions();

    m_lit = new QOpenGLShaderProgram(this);
    initShader(m_lit, kLitVS, kLitFS);
    m_color = new QOpenGLShaderProgram(this);
    initShader(m_color, kColorVS, kColorFS);
    m_trail = new QOpenGLShaderProgram(this);
    initShader(m_trail, kTrailVS, kTrailFS);

    initSphere();
    initTrail();
    initBars();
    initArrow();
    initRingGrid();
    initStars();

    m_earthTex = makeTexture(makeEarthImage());
    m_moonTex  = makeTexture(makeMoonImage());
    QImage white(4, 4, QImage::Format_RGB888);
    white.fill(QColor(255, 255, 255));
    m_whiteTex = makeTexture(white);
}

void Renderer::initSphere() {
    const int stacks = 40, sectors = 60;
    std::vector<float> verts;
    std::vector<unsigned> idx;
    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / stacks;
        float phi = v * (float)PI;
        for (int j = 0; j <= sectors; ++j) {
            float u = (float)j / sectors;
            float theta = u * 2.0f * (float)PI;
            float nx = std::sin(phi) * std::cos(theta);
            float ny = std::cos(phi);
            float nz = std::sin(phi) * std::sin(theta);
            verts.insert(verts.end(), { nx, ny, nz, nx, ny, nz, u, v });
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            unsigned a = i * (sectors + 1) + j;
            unsigned b = a + sectors + 1;
            idx.insert(idx.end(), { a, b, a + 1, a + 1, b, b + 1 });
        }
    }
    m_sphereIndexCount = (int)idx.size();
    m_sphereVao.create(); m_sphereVao.bind();
    m_sphereVbo.create(); m_sphereVbo.bind();
    m_sphereVbo.allocate(verts.data(), (int)(verts.size() * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    m_sphereIbo.create(); m_sphereIbo.bind();
    m_sphereIbo.allocate(idx.data(), (int)(idx.size() * sizeof(unsigned)));
    m_sphereVao.release();
}

void Renderer::initTrail() {
    m_trailVao.create(); m_trailVao.bind();
    m_trailVbo.create(); m_trailVbo.bind();
    m_trailVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_trailColorVbo.create(); m_trailColorVbo.bind();
    m_trailColorVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_trailVao.release();
}

void Renderer::initBars() {
    m_barVao.create(); m_barVao.bind();
    m_barVbo.create(); m_barVbo.bind();
    m_barVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_barVao.release();
}

void Renderer::initArrow() {
    m_arrowVao.create(); m_arrowVao.bind();
    m_arrowVbo.create(); m_arrowVbo.bind();
    m_arrowVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_arrowVao.release();
}

void Renderer::initRingGrid() {
    // 月球轨道圆（x-y 平面）
    std::vector<float> ring;
    const int seg = 256;
    float R = (float)(MOON_DIST * SC);
    for (int i = 0; i < seg; ++i) {
        float a = (float)(2.0 * PI * i / seg);
        ring.push_back(R * std::cos(a)); ring.push_back(R * std::sin(a)); ring.push_back(0.0f);
    }
    m_ringCount = seg;
    m_ringVao.create(); m_ringVao.bind();
    m_ringVbo.create(); m_ringVbo.bind();
    m_ringVbo.allocate(ring.data(), (int)(ring.size() * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_ringVao.release();

    // 参考网格（同心圆 + 径向线，GL_LINES）
    std::vector<float> grid;
    auto addLine = [&](float x1, float y1, float x2, float y2) {
        grid.insert(grid.end(), { x1, y1, 0.0f, x2, y2, 0.0f });
    };
    const int cseg = 96;
    const float radii[] = { 50, 100, 150, 200, 250, 300, 350, 400 };
    for (float r : radii) {
        for (int i = 0; i < cseg; ++i) {
            float a0 = (float)(2.0 * PI * i / cseg), a1 = (float)(2.0 * PI * (i + 1) / cseg);
            addLine(r * std::cos(a0), r * std::sin(a0), r * std::cos(a1), r * std::sin(a1));
        }
    }
    for (int i = 0; i < 12; ++i) {
        float a = (float)(2.0 * PI * i / 12);
        addLine(0, 0, 420.0f * std::cos(a), 420.0f * std::sin(a));
    }
    m_gridCount = (int)(grid.size() / 3);
    m_gridVao.create(); m_gridVao.bind();
    m_gridVbo.create(); m_gridVbo.bind();
    m_gridVbo.allocate(grid.data(), (int)(grid.size() * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_gridVao.release();
}

void Renderer::initStars() {
    const int n = 2000;
    std::mt19937 rng(99);
    std::vector<float> pts;
    pts.reserve(n * 3);
    for (int i = 0; i < n; ++i) {
        float th = (rng() % 628318) / 100000.0f;
        float ph = std::acos(2.0f * (rng() % 100000) / 100000.0f - 1.0f);
        float r = 1400.0f + (rng() % 600);
        pts.push_back(r * std::sin(ph) * std::cos(th));
        pts.push_back(r * std::cos(ph));
        pts.push_back(r * std::sin(ph) * std::sin(th));
    }
    m_starCount = n;
    m_starVao.create(); m_starVao.bind();
    m_starVbo.create(); m_starVbo.bind();
    m_starVbo.allocate(pts.data(), (int)(pts.size() * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_starVao.release();
}

GLuint Renderer::makeTexture(const QImage& img) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    QImage glImg = img.convertToFormat(QImage::Format_RGBA8888);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImg.width(), glImg.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, glImg.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

QImage Renderer::makeEarthImage() {
    QImage img(512, 256, QImage::Format_RGB888);
    img.fill(QColor(28, 68, 148));
    QPainter p(&img);
    std::mt19937 rng(7);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 30; ++i) {
        int x = rng() % 512, y = rng() % 256;
        int rx = 18 + rng() % 65, ry = 10 + rng() % 38;
        p.setBrush(QColor(58, 140, 72));
        p.drawEllipse(QPoint(x, y), rx, ry);
    }
    p.fillRect(0, 0, 512, 14, QColor(232, 240, 250));
    p.fillRect(0, 242, 512, 14, QColor(232, 240, 250));
    p.end();
    return img;
}

QImage Renderer::makeMoonImage() {
    QImage img(512, 256, QImage::Format_RGB888);
    img.fill(QColor(181, 181, 181));
    QPainter p(&img);
    std::mt19937 rng(42);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 260; ++i) {
        int x = rng() % 512, y = rng() % 256, r = 2 + rng() % 9;
        p.setBrush((rng() % 2) ? QColor(140, 140, 140) : QColor(206, 206, 206));
        p.drawEllipse(QPoint(x, y), r, r);
    }
    p.end();
    return img;
}

QMatrix4x4 Renderer::viewMatrix() const {
    float ry = m_yaw * (float)PI / 180.0f;
    float rp = m_pitch * (float)PI / 180.0f;
    QVector3D eye(m_target.x() + m_distance * std::cos(rp) * std::cos(ry),
                  m_target.y() + m_distance * std::sin(rp),
                  m_target.z() + m_distance * std::cos(rp) * std::sin(ry));
    QMatrix4x4 v;
    v.lookAt(eye, m_target, QVector3D(0, 1, 0));
    return v;
}

void Renderer::drawSphere(GLuint tex, const QVector3D& center, float radius,
                          const QMatrix4x4& proj, const QMatrix4x4& view, const QVector3D& light) {
    m_lit->bind();
    QMatrix4x4 model;
    model.translate(center);
    model.scale(radius);
    m_lit->setUniformValue("uMVP", proj * view * model);
    m_lit->setUniformValue("uModel", model);
    m_lit->setUniformValue("uNormalMat", model.normalMatrix());
    m_lit->setUniformValue("uLightDir", light);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    m_lit->setUniformValue("uTex", 0);
    m_sphereVao.bind();
    glDrawElements(GL_TRIANGLES, m_sphereIndexCount, GL_UNSIGNED_INT, nullptr);
    m_sphereVao.release();
    m_lit->release();
}

void Renderer::resizeGL(int w, int h) { Q_UNUSED(w); Q_UNUSED(h); }

void Renderer::paintGL() {
    glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
    glClearColor(0.015f, 0.025f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = width() > 0 ? (float)width() / height() : 1.0f;
    QMatrix4x4 proj;
    proj.perspective(55.0f, aspect, 0.1f, 20000.0f);
    QMatrix4x4 view = viewMatrix();
    QVector3D light = QVector3D(0.6f, 0.6f, 0.4f).normalized();

    // 星空（先画，无深度）
    glDisable(GL_DEPTH_TEST);
    m_color->bind();
    m_color->setUniformValue("uMVP", proj * view);
    m_color->setUniformValue("uColor", QVector4D(1, 1, 1, 0.85f));
    m_starVao.bind();
    glDrawArrays(GL_POINTS, 0, m_starCount);
    m_starVao.release();
    m_color->release();
    glEnable(GL_DEPTH_TEST);

    // 参考网格 + 月球轨道圆
    m_color->bind();
    m_color->setUniformValue("uMVP", proj * view);
    m_color->setUniformValue("uColor", QVector4D(0.25f, 0.32f, 0.44f, 1.0f));
    m_gridVao.bind();
    glDrawArrays(GL_LINES, 0, m_gridCount);
    m_gridVao.release();
    m_color->setUniformValue("uColor", QVector4D(0.45f, 0.52f, 0.62f, 1.0f));
    m_ringVao.bind();
    glDrawArrays(GL_LINE_LOOP, 0, m_ringCount);
    m_ringVao.release();
    m_color->release();

    // 天体
    drawSphere(m_earthTex, QVector3D(0, 0, 0), (float)(R_EARTH * SC), proj, view, light);
    QVector3D moonPos((float)(m_scene.moonKm.x * SC), (float)(m_scene.moonKm.y * SC), (float)(m_scene.moonKm.z * SC));
    drawSphere(m_moonTex, moonPos, (float)(R_MOON * SC), proj, view, light);
    QVector3D scPos((float)(m_scene.scPosKm.x * SC), (float)(m_scene.scPosKm.y * SC), (float)(m_scene.scPosKm.z * SC));
    drawSphere(m_whiteTex, scPos, 0.35f, proj, view, light);

    // 轨迹（顶点颜色）
    int n = (int)m_scene.trailKm.size();
    if (n >= 2 && (int)m_scene.trailColors.size() == n * 3) {
        std::vector<float> pts(n * 3);
        for (int i = 0; i < n; ++i) {
            pts[i*3+0] = (float)(m_scene.trailKm[i].x * SC);
            pts[i*3+1] = (float)(m_scene.trailKm[i].y * SC);
            pts[i*3+2] = (float)(m_scene.trailKm[i].z * SC);
        }
        m_trailVao.bind();
        m_trailVbo.bind();
        m_trailVbo.allocate(pts.data(), (int)(pts.size() * sizeof(float)));
        m_trailColorVbo.bind();
        m_trailColorVbo.allocate(m_scene.trailColors.data(), (int)(m_scene.trailColors.size() * sizeof(float)));
        m_trail->bind();
        m_trail->setUniformValue("uMVP", proj * view);
        glDrawArrays(GL_LINE_STRIP, 0, n);
        m_trail->release();
        m_trailVao.release();
    }

    // 速度竖线
    if (m_scene.barVerts.size() >= 6) {
        m_barVao.bind();
        m_barVbo.bind();
        m_barVbo.allocate(m_scene.barVerts.data(), (int)(m_scene.barVerts.size() * sizeof(float)));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_color->bind();
        m_color->setUniformValue("uMVP", proj * view);
        m_color->setUniformValue("uColor", QVector4D(0.62f, 0.77f, 0.92f, 0.55f));
        glDrawArrays(GL_LINES, 0, (int)(m_scene.barVerts.size() / 3));
        m_color->release();
        glDisable(GL_BLEND);
        m_barVao.release();
    }

    // 速度箭头
    if (m_scene.showArrow) {
        Vec3 v = m_scene.scVelKmS;
        double spd = v.length();
        if (spd > 1e-9) {
            Vec3 dir = v * (1.0 / spd);
            Vec3 tip = m_scene.scPosKm + dir * (spd * ARROW_SCALE);
            float arrow[6] = {
                (float)(m_scene.scPosKm.x * SC), (float)(m_scene.scPosKm.y * SC), (float)(m_scene.scPosKm.z * SC),
                (float)(tip.x * SC), (float)(tip.y * SC), (float)(tip.z * SC)
            };
            m_arrowVao.bind();
            m_arrowVbo.bind();
            m_arrowVbo.allocate(arrow, sizeof(arrow));
            m_color->bind();
            m_color->setUniformValue("uMVP", proj * view);
            m_color->setUniformValue("uColor", QVector4D(0.0f, 0.9f, 1.0f, 1.0f));
            glDrawArrays(GL_LINES, 0, 2);
            m_color->release();
            m_arrowVao.release();
        }
    }
}

void Renderer::mousePressEvent(QMouseEvent* e) { m_lastPos = e->pos(); }

void Renderer::mouseMoveEvent(QMouseEvent* e) {
    QPoint d = e->pos() - m_lastPos;
    m_lastPos = e->pos();
    if (e->buttons() & Qt::LeftButton) {
        m_yaw += d.x() * 0.3f;
        m_pitch += d.y() * 0.3f;
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        update();
    } else if (e->buttons() & Qt::RightButton) {
        float ry = m_yaw * (float)PI / 180.0f, rp = m_pitch * (float)PI / 180.0f;
        QVector3D eye(m_target.x() + m_distance * std::cos(rp) * std::cos(ry),
                      m_target.y() + m_distance * std::sin(rp),
                      m_target.z() + m_distance * std::cos(rp) * std::sin(ry));
        QVector3D forward = (m_target - eye).normalized();
        QVector3D right = QVector3D::crossProduct(forward, QVector3D(0, 1, 0)).normalized();
        QVector3D up = QVector3D::crossProduct(right, forward).normalized();
        float s = m_distance * 0.0012f;
        m_target += -right * (d.x() * s) + up * (d.y() * s);
        update();
    }
}

void Renderer::wheelEvent(QWheelEvent* e) {
    float delta = e->angleDelta().y() / 120.0f;
    m_distance *= std::pow(0.9f, delta);
    m_distance = std::clamp(m_distance, 2.0f, 4000.0f);
    update();
}

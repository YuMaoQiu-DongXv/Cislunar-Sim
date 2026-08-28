#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QPoint>
#include <vector>
#include "physics.h"

struct SceneData {
    double time = 0.0;
    Vec3 scPosKm{};          // 航天器位置 (km)
    Vec3 scVelKmS{};         // 航天器速度 (km/s)
    Vec3 moonKm{};           // 月球位置 (km)
    std::vector<Vec3> trailKm;      // 轨迹点（已按窗口过滤，km）
    std::vector<float> trailColors; // 每点颜色 RGB（3 * trailKm.size()）
    std::vector<float> barVerts;    // 速度竖线顶点（每根 2 顶点 * 3 分量，场景单位）
    std::vector<Vec3> keplerKm;     // 开普勒轨道预览点（km）
    bool showArrow = true;
};

class Renderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit Renderer(QWidget* parent = nullptr);
    ~Renderer() override;
    void setScene(const SceneData& d);
    void resetView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    // 着色器
    QOpenGLShaderProgram* m_lit = nullptr;   // 球体（贴图+光照）
    QOpenGLShaderProgram* m_color = nullptr; // 纯色（线/点/箭头/竖线）
    QOpenGLShaderProgram* m_trail = nullptr; // 轨迹（顶点颜色）

    // 球体几何（单位球）
    QOpenGLBuffer m_sphereVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_sphereIbo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_sphereVao;
    int m_sphereIndexCount = 0;

    GLuint m_earthTex = 0, m_moonTex = 0, m_whiteTex = 0;

    // 轨迹（位置 + 顶点颜色）
    QOpenGLBuffer m_trailVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_trailColorVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_trailVao;

    // 速度竖线
    QOpenGLBuffer m_barVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_barVao;

    // 速度箭头
    QOpenGLBuffer m_arrowVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_arrowVao;

    // 开普勒轨道预览
    QOpenGLBuffer m_keplerVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_keplerVao;

    // 月球轨道圆 + 参考网格
    QOpenGLBuffer m_ringVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_ringVao;
    int m_ringCount = 0;
    QOpenGLBuffer m_gridVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_gridVao;
    int m_gridCount = 0;

    // 星空
    QOpenGLBuffer m_starVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_starVao;
    int m_starCount = 0;

    SceneData m_scene;

    QVector3D m_target{150, 0, 0};
    float m_distance = 700.0f;
    float m_yaw = -25.0f, m_pitch = 30.0f;
    QPoint m_lastPos;

    void initSphere();
    void initTrail();
    void initBars();
    void initArrow();
    void initKepler();
    void initRingGrid();
    void initStars();
    void initShader(QOpenGLShaderProgram* p, const char* vs, const char* fs);
    GLuint makeTexture(const QImage& img);
    QImage makeEarthImage();
    QImage makeMoonImage();
    QMatrix4x4 viewMatrix() const;
    void drawSphere(GLuint tex, const QVector3D& center, float radius,
                    const QMatrix4x4& proj, const QMatrix4x4& view, const QVector3D& light);
};

#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QVector3D>
#include <vector>
#include "physics.h"

class Renderer;
class QSlider;
class QLabel;
class QTabWidget;
class QVBoxLayout;
class QPushButton;
class QLineEdit;
class QListWidget;
class QDoubleSpinBox;
class QCheckBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onParamChanged();
    void onStart();
    void onPause();
    void onReset();
    void tick();
    void onSavePreset();
    void onLoadPreset();
    void onDeletePreset();
    void onTrailModeChanged(bool all);
    void onSpeedVizChanged(const QString& mode);

private:
    enum Mode { PosVel, Elements };
    struct ParamBinding { QSlider* slider; QDoubleSpinBox* spin; double* value; double min, max; };

    Renderer* m_renderer = nullptr;
    QTimer m_timer;
    qint64 m_lastTick = 0;

    Mode m_mode = PosVel;
    double m_x=6771, m_y=0, m_z=0, m_speed=7.67, m_az=90, m_elev=0;
    double m_a=6771, m_e=0, m_inc=0, m_raan=0, m_argp=0, m_nu=0;

    State m_state;
    double m_simTime = 0;
    double m_simSpeed = 60;
    bool m_running = false;
    bool m_trailAll = false;         // false=最近30天, true=始终显示
    bool m_keplerPreview = true;     // 开普勒轨道预览（仅地球引力）
    QString m_speedViz = "color";    // color / bars / both / off
    std::vector<Vec3> m_trail;
    std::vector<double> m_trailTimes;
    std::vector<double> m_trailSpeeds;

    QTabWidget* m_tabs = nullptr;
    QLabel *m_lTime=nullptr, *m_lSpeed=nullptr, *m_lAltE=nullptr, *m_lAltM=nullptr,
           *m_lAccE=nullptr, *m_lAccM=nullptr, *m_lStatus=nullptr;
    QSlider* m_speedSlider = nullptr;
    QLabel* m_speedLabel = nullptr;
    QPushButton* m_pauseBtn = nullptr;
    QLineEdit* m_presetName = nullptr;
    QListWidget* m_presetList = nullptr;
    QPushButton *m_btnTrail30d=nullptr, *m_btnTrailAll=nullptr;
    QPushButton *m_btnVizColor=nullptr, *m_btnVizBars=nullptr, *m_btnVizBoth=nullptr, *m_btnVizOff=nullptr;
    QCheckBox* m_keplerCheck = nullptr;
    std::vector<ParamBinding> m_paramBindings;

    State computeState0() const;
    void addParam(QVBoxLayout* layout, const QString& label, double min, double max, double* value, const QString& unit = QString());
    void advance(double dtReal);
    bool checkEvents();
    void pushScene();
    void updateTelemetry();
    void refreshPresetList();
    void syncParamWidgets();
    void applyPreset(const QString& name);
    static QString fmtTime(double t);
    static QString fmtAccel(double a);
    static QVector3D speedToColor(double speed);
    void styleButton(QPushButton* b, bool active);
};

#include "mainwindow.h"
#include "renderer.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QTimer>
#include <QDateTime>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <algorithm>
#include <cmath>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Cislunar-Sim · 地月系轨道 3D 模拟器"));
    resize(1300, 820);

    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_renderer = new Renderer(central);
    root->addWidget(m_renderer, 1);

    auto* panel = new QWidget(central);
    panel->setFixedWidth(430);
    panel->setStyleSheet(
        "QWidget{background:#101724;color:#e7edf7;font-size:12px;}"
        "QGroupBox{border:1px solid #26324a;border-radius:8px;margin-top:10px;padding-top:8px;}"
        "QGroupBox::title{color:#4da3ff;}"
        "QTabWidget::pane{border:1px solid #26324a;}"
        "QTabBar::tab{background:#182032;color:#8fa2bf;padding:6px 12px;}"
        "QTabBar::tab:selected{background:#2563eb;color:#fff;}"
        "QPushButton{background:#2563eb;color:#fff;border:none;border-radius:6px;padding:8px;font-weight:600;}"
        "QPushButton:hover{background:#3b82f6;}"
        "QLabel{color:#e7edf7;}"
        "QLineEdit,QDoubleSpinBox{background:#0b1322;border:1px solid #26324a;border-radius:4px;color:#e7edf7;padding:4px;}"
        "QListWidget{background:#0b1322;border:1px solid #26324a;border-radius:6px;color:#e7edf7;}"
        "QListWidget::item:selected{background:#2563eb;}"
        "QSlider::handle:horizontal{background:#4da3ff;border-radius:6px;width:14px;margin:-4px 0;}"
        "QSlider::groove:horizontal{background:#1a2438;height:4px;border-radius:2px;}");
    root->addWidget(panel);
    setCentralWidget(central);

    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(12, 12, 12, 12);

    // ---- 初始轨道设置 ----
    auto* initGroup = new QGroupBox(QStringLiteral("初始轨道设置"), panel);
    auto* initLayout = new QVBoxLayout(initGroup);
    m_tabs = new QTabWidget(initGroup);
    auto* posvelTab = new QWidget();
    auto* posvelLayout = new QVBoxLayout(posvelTab);
    addParam(posvelLayout, QStringLiteral("X 位置"), -500000, 500000, &m_x, QStringLiteral("km"));
    addParam(posvelLayout, QStringLiteral("Y 位置"), -500000, 500000, &m_y, QStringLiteral("km"));
    addParam(posvelLayout, QStringLiteral("Z 位置"), -500000, 500000, &m_z, QStringLiteral("km"));
    addParam(posvelLayout, QStringLiteral("速度大小"), 0, 15, &m_speed, QStringLiteral("km/s"));
    addParam(posvelLayout, QStringLiteral("方位角"), 0, 360, &m_az, QStringLiteral("°"));
    addParam(posvelLayout, QStringLiteral("仰角"), -90, 90, &m_elev, QStringLiteral("°"));
    posvelLayout->addStretch(1);
    auto* elemTab = new QWidget();
    auto* elemLayout = new QVBoxLayout(elemTab);
    addParam(elemLayout, QStringLiteral("半长轴 a"), 6400, 600000, &m_a, QStringLiteral("km"));
    addParam(elemLayout, QStringLiteral("偏心率 e"), 0, 0.95, &m_e);
    addParam(elemLayout, QStringLiteral("倾角 i"), 0, 180, &m_inc, QStringLiteral("°"));
    addParam(elemLayout, QStringLiteral("升交点赤经 Ω"), 0, 360, &m_raan, QStringLiteral("°"));
    addParam(elemLayout, QStringLiteral("近地点幅角 ω"), 0, 360, &m_argp, QStringLiteral("°"));
    addParam(elemLayout, QStringLiteral("真近点角 ν"), 0, 360, &m_nu, QStringLiteral("°"));
    elemLayout->addStretch(1);
    m_tabs->addTab(posvelTab, QStringLiteral("位置 + 速度"));
    m_tabs->addTab(elemTab, QStringLiteral("轨道根数"));
    initLayout->addWidget(m_tabs);
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int idx){
        m_mode = (idx == 0) ? PosVel : Elements;
        onParamChanged();
    });
    panelLayout->addWidget(initGroup);

    // ---- 轨迹显示 ----
    auto* trailGroup = new QGroupBox(QStringLiteral("轨迹显示"), panel);
    auto* trailLayout = new QHBoxLayout(trailGroup);
    m_btnTrail30d = new QPushButton(QStringLiteral("最近 30 天"));
    m_btnTrailAll = new QPushButton(QStringLiteral("始终显示"));
    trailLayout->addWidget(m_btnTrail30d);
    trailLayout->addWidget(m_btnTrailAll);
    connect(m_btnTrail30d, &QPushButton::clicked, this, [this]{ onTrailModeChanged(false); });
    connect(m_btnTrailAll, &QPushButton::clicked, this, [this]{ onTrailModeChanged(true); });
    styleButton(m_btnTrail30d, true);
    styleButton(m_btnTrailAll, false);
    panelLayout->addWidget(trailGroup);

    // ---- 速度可视化 ----
    auto* vizGroup = new QGroupBox(QStringLiteral("速度可视化"), panel);
    auto* vizLayout = new QHBoxLayout(vizGroup);
    m_btnVizColor = new QPushButton(QStringLiteral("颜色"));
    m_btnVizBars = new QPushButton(QStringLiteral("竖线"));
    m_btnVizBoth = new QPushButton(QStringLiteral("两者"));
    m_btnVizOff = new QPushButton(QStringLiteral("关闭"));
    vizLayout->addWidget(m_btnVizColor);
    vizLayout->addWidget(m_btnVizBars);
    vizLayout->addWidget(m_btnVizBoth);
    vizLayout->addWidget(m_btnVizOff);
    connect(m_btnVizColor, &QPushButton::clicked, this, [this]{ onSpeedVizChanged("color"); });
    connect(m_btnVizBars, &QPushButton::clicked, this, [this]{ onSpeedVizChanged("bars"); });
    connect(m_btnVizBoth, &QPushButton::clicked, this, [this]{ onSpeedVizChanged("both"); });
    connect(m_btnVizOff, &QPushButton::clicked, this, [this]{ onSpeedVizChanged("off"); });
    styleButton(m_btnVizColor, true);
    styleButton(m_btnVizBars, false);
    styleButton(m_btnVizBoth, false);
    styleButton(m_btnVizOff, false);
    panelLayout->addWidget(vizGroup);

    // ---- 预设方案 ----
    auto* presetGroup = new QGroupBox(QStringLiteral("预设方案"), panel);
    auto* presetLayout = new QVBoxLayout(presetGroup);
    auto* presetRow = new QHBoxLayout();
    m_presetName = new QLineEdit();
    m_presetName->setPlaceholderText(QStringLiteral("预设名称（留空自动命名）"));
    auto* saveBtn = new QPushButton(QStringLiteral("保存"));
    presetRow->addWidget(m_presetName, 1);
    presetRow->addWidget(saveBtn);
    presetLayout->addLayout(presetRow);
    m_presetList = new QListWidget();
    m_presetList->setFixedHeight(110);
    presetLayout->addWidget(m_presetList);
    auto* presetBtnRow = new QHBoxLayout();
    auto* loadBtn = new QPushButton(QStringLiteral("载入"));
    auto* delBtn = new QPushButton(QStringLiteral("删除"));
    loadBtn->setStyleSheet("background:#334155;");
    delBtn->setStyleSheet("background:#7f1d1d;");
    presetBtnRow->addWidget(loadBtn);
    presetBtnRow->addWidget(delBtn);
    presetLayout->addLayout(presetBtnRow);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSavePreset);
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoadPreset);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::onDeletePreset);
    connect(m_presetList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* it){ applyPreset(it->text()); });
    panelLayout->addWidget(presetGroup);

    // ---- 实时数据（整行，避免重叠）----
    auto* dataGroup = new QGroupBox(QStringLiteral("实时数据"), panel);
    auto* dataLayout = new QVBoxLayout(dataGroup);
    dataLayout->setSpacing(4);
    auto addData = [&](const QString& k, QLabel** out){
        auto* row = new QHBoxLayout();
        auto* kl = new QLabel(k); kl->setStyleSheet("color:#8fa2bf;");
        kl->setFixedWidth(110);
        auto* vl = new QLabel(QStringLiteral("—")); vl->setStyleSheet("font-weight:600;");
        vl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        row->addWidget(kl);
        row->addWidget(vl, 1);
        dataLayout->addLayout(row);
        *out = vl;
    };
    addData(QStringLiteral("模拟时间"), &m_lTime);
    addData(QStringLiteral("速度"), &m_lSpeed);
    addData(QStringLiteral("距地面高度"), &m_lAltE);
    addData(QStringLiteral("距月面高度"), &m_lAltM);
    addData(QStringLiteral("地球引力加速度"), &m_lAccE);
    addData(QStringLiteral("月球引力加速度"), &m_lAccM);
    addData(QStringLiteral("状态"), &m_lStatus);
    m_lStatus->setText(QStringLiteral("预览中"));
    panelLayout->addWidget(dataGroup);

    // ---- 控制 ----
    auto* ctrlGroup = new QGroupBox(QStringLiteral("控制"), panel);
    auto* ctrlLayout = new QVBoxLayout(ctrlGroup);
    auto* btnRow = new QHBoxLayout();
    auto* startBtn = new QPushButton(QStringLiteral("开始模拟"));
    m_pauseBtn = new QPushButton(QStringLiteral("暂停"));
    auto* resetBtn = new QPushButton(QStringLiteral("重置"));
    resetBtn->setStyleSheet("background:#334155;");
    btnRow->addWidget(startBtn);
    btnRow->addWidget(m_pauseBtn);
    btnRow->addWidget(resetBtn);
    ctrlLayout->addLayout(btnRow);
    m_speedLabel = new QLabel();
    m_speedLabel->setStyleSheet("color:#8fa2bf;");
    ctrlLayout->addWidget(m_speedLabel);
    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(0, 1000);
    ctrlLayout->addWidget(m_speedSlider);
    panelLayout->addWidget(ctrlGroup);

    panelLayout->addStretch(1);

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPause);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);

    const double LOG_MAX = std::log10(86400.0);
    auto updateSpeedUI = [&](){
        m_speedLabel->setText(QStringLiteral("模拟速度：每现实秒推进 ") + fmtTime(m_simSpeed));
    };
    m_speedSlider->setValue((int)std::round(std::log10(m_simSpeed) / LOG_MAX * 1000.0));
    connect(m_speedSlider, &QSlider::valueChanged, this, [=](int v){
        m_simSpeed = std::pow(10.0, (double)v / 1000.0 * LOG_MAX);
        updateSpeedUI();
    });
    updateSpeedUI();

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::tick);
    m_timer.start(16);

    m_state = computeState0();
    refreshPresetList();
    pushScene();
    updateTelemetry();
}

void MainWindow::addParam(QVBoxLayout* layout, const QString& label, double min, double max,
                          double* value, const QString& unit) {
    auto* row = new QWidget();
    auto* h = new QHBoxLayout(row);
    h->setContentsMargins(0, 0, 0, 0);
    auto* lab = new QLabel(label);
    lab->setFixedWidth(88);
    lab->setStyleSheet("color:#8fa2bf;");
    auto* slider = new QSlider(Qt::Horizontal);
    auto* spin = new QDoubleSpinBox();
    spin->setRange(min, max);
    spin->setDecimals(3);
    spin->setValue(*value);
    spin->setSingleStep((max - min) / 1000.0);
    spin->setFixedWidth(108);
    if (!unit.isEmpty()) spin->setSuffix(QStringLiteral(" ") + unit);
    slider->setRange(0, 1000);
    slider->setValue((int)std::round((*value - min) / (max - min) * 1000.0));
    h->addWidget(lab);
    h->addWidget(slider, 1);
    h->addWidget(spin);
    layout->addWidget(row);

    m_paramBindings.push_back({ slider, spin, value, min, max });

    connect(slider, &QSlider::valueChanged, this, [=](int v){
        *value = min + (max - min) * v / 1000.0;
        spin->blockSignals(true);
        spin->setValue(*value);
        spin->blockSignals(false);
        onParamChanged();
    });
    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double v){
        *value = v;
        slider->blockSignals(true);
        slider->setValue((int)std::round((v - min) / (max - min) * 1000.0));
        slider->blockSignals(false);
        onParamChanged();
    });
}

void MainWindow::syncParamWidgets() {
    for (auto& b : m_paramBindings) {
        b.spin->blockSignals(true);
        b.slider->blockSignals(true);
        b.spin->setValue(*b.value);
        b.slider->setValue((int)std::round((*b.value - b.min) / (b.max - b.min) * 1000.0));
        b.spin->blockSignals(false);
        b.slider->blockSignals(false);
    }
}

void MainWindow::styleButton(QPushButton* b, bool active) {
    if (active) b->setStyleSheet("background:#2563eb;color:#fff;border:none;border-radius:6px;padding:7px;font-weight:600;");
    else b->setStyleSheet("background:#1a2438;color:#8fa2bf;border:1px solid #26324a;border-radius:6px;padding:7px;");
}

State MainWindow::computeState0() const {
    if (m_mode == PosVel) {
        Vec3 vel = velocityFromDir(m_speed, m_az, m_elev);
        return { { m_x, m_y, m_z }, vel };
    }
    return elementsToState(m_a, m_e, m_inc, m_raan, m_argp, m_nu);
}

void MainWindow::onParamChanged() {
    if (!m_running) {
        m_state = computeState0();
        m_simTime = 0;
        m_trail.clear();
        m_trailTimes.clear();
        m_trailSpeeds.clear();
        pushScene();
        updateTelemetry();
    }
}

void MainWindow::onStart() {
    m_state = computeState0();
    m_simTime = 0;
    m_trail.clear();
    m_trailTimes.clear();
    m_trailSpeeds.clear();
    m_running = true;
    m_pauseBtn->setText(QStringLiteral("暂停"));
    pushScene();
    updateTelemetry();
}

void MainWindow::onPause() {
    if (m_running) { m_running = false; m_pauseBtn->setText(QStringLiteral("继续")); }
    else { m_running = true; m_pauseBtn->setText(QStringLiteral("暂停")); }
    updateTelemetry();
}

void MainWindow::onReset() {
    m_running = false;
    m_state = computeState0();
    m_simTime = 0;
    m_trail.clear();
    m_trailTimes.clear();
    m_trailSpeeds.clear();
    m_pauseBtn->setText(QStringLiteral("暂停"));
    pushScene();
    updateTelemetry();
}

void MainWindow::onTrailModeChanged(bool all) {
    m_trailAll = all;
    styleButton(m_btnTrail30d, !all);
    styleButton(m_btnTrailAll, all);
    pushScene();
}

void MainWindow::onSpeedVizChanged(const QString& mode) {
    m_speedViz = mode;
    styleButton(m_btnVizColor, mode == "color");
    styleButton(m_btnVizBars, mode == "bars");
    styleButton(m_btnVizBoth, mode == "both");
    styleButton(m_btnVizOff, mode == "off");
    pushScene();
}

void MainWindow::advance(double dtReal) {
    double target = m_simSpeed * dtReal;
    const double maxStep = 5.0;
    int n = std::max(1, (int)std::ceil(target / maxStep));
    double dt = target / n;
    const size_t MAXPTS = 200000;
    for (int i = 0; i < n; ++i) {
        if (!m_running) break;
        m_state = rk4Step(m_state, m_simTime, dt);
        m_simTime += dt;
        bool shouldAdd = (m_trail.size() < MAXPTS) || !m_trailAll;
        if (shouldAdd && (m_trail.empty() || (m_state.pos - m_trail.back()).length() > 250.0)) {
            m_trail.push_back(m_state.pos);
            m_trailTimes.push_back(m_simTime);
            m_trailSpeeds.push_back(m_state.vel.length());
        }
        if (m_trail.size() > MAXPTS) {
            size_t drop = m_trail.size() / 2;
            m_trail.erase(m_trail.begin(), m_trail.begin() + drop);
            m_trailTimes.erase(m_trailTimes.begin(), m_trailTimes.begin() + drop);
            m_trailSpeeds.erase(m_trailSpeeds.begin(), m_trailSpeeds.begin() + drop);
        }
        if (checkEvents()) break;
    }
}

bool MainWindow::checkEvents() {
    double de = m_state.pos.length();
    Vec3 mp = moonPosKm(m_simTime);
    double dm = (m_state.pos - mp).length();
    if (de <= R_EARTH) { m_running = false; m_lStatus->setText(QStringLiteral("已坠毁地球")); return true; }
    if (dm <= R_MOON)  { m_running = false; m_lStatus->setText(QStringLiteral("已坠毁月球")); return true; }
    if (de > 1.2e6)    { m_running = false; m_lStatus->setText(QStringLiteral("已脱离地月系")); return true; }
    return false;
}

void MainWindow::pushScene() {
    SceneData d;
    d.time = m_simTime;
    d.scPosKm = m_state.pos;
    d.scVelKmS = m_state.vel;
    d.moonKm = moonPosKm(m_simTime);
    d.showArrow = true;

    // 轨迹窗口过滤（最近 30 天 / 全部）
    int start = 0;
    if (!m_trailAll && !m_trailTimes.empty()) {
        double cutoff = m_simTime - 30.0 * 86400.0;
        int lo = 0, hi = (int)m_trailTimes.size();
        while (lo < hi) { int mid = (lo + hi) / 2; if (m_trailTimes[mid] < cutoff) lo = mid + 1; else hi = mid; }
        start = lo;
    }
    int total = (int)m_trail.size();
    int n = total - start;
    d.trailKm.assign(m_trail.begin() + start, m_trail.end());
    d.trailColors.reserve((size_t)n * 3);
    bool useColor = (m_speedViz == "color" || m_speedViz == "both");
    for (int i = start; i < total; ++i) {
        QVector3D c = useColor ? speedToColor(m_trailSpeeds[i]) : QVector3D(1.0f, 0.7f, 0.0f);
        d.trailColors.push_back(c.x()); d.trailColors.push_back(c.y()); d.trailColors.push_back(c.z());
    }

    // 速度竖线
    bool showBars = (m_speedViz == "bars" || m_speedViz == "both");
    if (showBars && n > 0) {
        const int barStep = 12;
        const float barScale = 1.0f;
        const double SC = 0.001;
        int first = ((start + barStep - 1) / barStep) * barStep;
        for (int i = first; i < total; i += barStep) {
            float px = (float)(m_trail[i].x * SC);
            float py = (float)(m_trail[i].y * SC);
            float pz = (float)(m_trail[i].z * SC);
            float h = (float)(m_trailSpeeds[i] * barScale);
            d.barVerts.push_back(px); d.barVerts.push_back(py); d.barVerts.push_back(pz - h);
            d.barVerts.push_back(px); d.barVerts.push_back(py); d.barVerts.push_back(pz + h);
        }
    }
    m_renderer->setScene(d);
}

void MainWindow::updateTelemetry() {
    double speed = m_state.vel.length();
    double de = m_state.pos.length();
    Vec3 mp = moonPosKm(m_simTime);
    double dm = (m_state.pos - mp).length();
    m_lTime->setText(fmtTime(m_simTime));
    m_lSpeed->setText(QString::number(speed, 'f', 3) + QStringLiteral(" km/s"));
    m_lAltE->setText(QString::number(de - R_EARTH, 'f', 1) + QStringLiteral(" km"));
    m_lAltM->setText(QString::number(dm - R_MOON, 'f', 1) + QStringLiteral(" km"));
    m_lAccE->setText(fmtAccel(MU_EARTH / (de * de) * 1000.0));
    m_lAccM->setText(fmtAccel(MU_MOON / (dm * dm) * 1000.0));
    if (m_running) m_lStatus->setText(QStringLiteral("运行中"));
    else if (m_simTime > 0) m_lStatus->setText(QStringLiteral("已暂停"));
    else m_lStatus->setText(QStringLiteral("预览中"));
}

void MainWindow::tick() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastTick == 0) m_lastTick = now;
    double dtReal = std::min((now - m_lastTick) / 1000.0, 0.1);
    m_lastTick = now;
    if (m_running) advance(dtReal);
    pushScene();
    updateTelemetry();
}

// ---- 预设（QSettings + JSON）----
static QJsonArray loadPresetsArray() {
    QSettings s(QStringLiteral("CislunarSim"), QStringLiteral("CislunarSim"));
    QJsonDocument doc = QJsonDocument::fromJson(s.value(QStringLiteral("presets")).toByteArray());
    return doc.isArray() ? doc.array() : QJsonArray();
}
static void savePresetsArray(const QJsonArray& arr) {
    QSettings s(QStringLiteral("CislunarSim"), QStringLiteral("CislunarSim"));
    s.setValue(QStringLiteral("presets"), QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

void MainWindow::refreshPresetList() {
    m_presetList->clear();
    QJsonArray arr = loadPresetsArray();
    for (const auto& v : arr) {
        m_presetList->addItem(v.toObject()[QStringLiteral("name")].toString());
    }
}

void MainWindow::onSavePreset() {
    QString name = m_presetName->text().trimmed();
    if (name.isEmpty()) name = QStringLiteral("预设 ") + QDateTime::currentDateTime().toString(QStringLiteral("MM-dd hh:mm"));
    QJsonArray arr = loadPresetsArray();
    QJsonObject o;
    o[QStringLiteral("name")] = name;
    o[QStringLiteral("mode")] = (m_mode == PosVel) ? QStringLiteral("posvel") : QStringLiteral("elements");
    o[QStringLiteral("speed")] = m_simSpeed;
    QVariantList params;
    params << m_x << m_y << m_z << m_speed << m_az << m_elev << m_a << m_e << m_inc << m_raan << m_argp << m_nu;
    o[QStringLiteral("params")] = QJsonArray::fromVariantList(params);
    arr.append(o);
    savePresetsArray(arr);
    m_presetName->clear();
    refreshPresetList();
}

void MainWindow::applyPreset(const QString& name) {
    QJsonArray arr = loadPresetsArray();
    for (const auto& v : arr) {
        QJsonObject o = v.toObject();
        if (o[QStringLiteral("name")].toString() != name) continue;
        m_mode = (o[QStringLiteral("mode")].toString() == QStringLiteral("elements")) ? Elements : PosVel;
        QJsonArray p = o[QStringLiteral("params")].toArray();
        if (p.size() >= 12) {
            m_x = p[0].toDouble(); m_y = p[1].toDouble(); m_z = p[2].toDouble();
            m_speed = p[3].toDouble(); m_az = p[4].toDouble(); m_elev = p[5].toDouble();
            m_a = p[6].toDouble(); m_e = p[7].toDouble(); m_inc = p[8].toDouble();
            m_raan = p[9].toDouble(); m_argp = p[10].toDouble(); m_nu = p[11].toDouble();
        }
        m_simSpeed = o[QStringLiteral("speed")].toDouble(60.0);
        m_tabs->setCurrentIndex(m_mode == PosVel ? 0 : 1);
        syncParamWidgets();
        const double LOG_MAX = std::log10(86400.0);
        m_speedSlider->setValue((int)std::round(std::log10(m_simSpeed) / LOG_MAX * 1000.0));
        m_speedLabel->setText(QStringLiteral("模拟速度：每现实秒推进 ") + fmtTime(m_simSpeed));
        onParamChanged();
        break;
    }
}

void MainWindow::onLoadPreset() {
    auto* it = m_presetList->currentItem();
    if (it) applyPreset(it->text());
}

void MainWindow::onDeletePreset() {
    int row = m_presetList->currentRow();
    if (row < 0) return;
    QString name = m_presetList->item(row)->text();
    QJsonArray arr = loadPresetsArray();
    QJsonArray out;
    for (const auto& v : arr) {
        if (v.toObject()[QStringLiteral("name")].toString() != name) out.append(v);
    }
    savePresetsArray(out);
    refreshPresetList();
}

QString MainWindow::fmtTime(double t) {
    if (t < 0) t = 0;
    if (t < 60) return QString::number(t, 'f', 1) + QStringLiteral(" 秒");
    if (t < 3600) {
        int m = (int)(t / 60);
        return QString::number(m) + QStringLiteral(" 分 ") + QString::number((int)t % 60) + QStringLiteral(" 秒");
    }
    if (t < 86400) {
        int h = (int)(t / 3600);
        return QString::number(h) + QStringLiteral(" 时 ") + QString::number(((int)t % 3600) / 60) + QStringLiteral(" 分");
    }
    int d = (int)(t / 86400);
    return QString::number(d) + QStringLiteral(" 天 ") + QString::number(((int)t % 86400) / 3600) + QStringLiteral(" 时");
}

QString MainWindow::fmtAccel(double a) {
    if (a < 0.01) return QString::number(a, 'e', 2) + QStringLiteral(" m/s²");
    return QString::number(a, 'f', 4) + QStringLiteral(" m/s²");
}

QVector3D MainWindow::speedToColor(double speed) {
    double t = std::clamp(speed / 12.0, 0.0, 1.0);
    double h = t * 240.0 / 360.0;
    double s = 1.0, l = 0.55;
    auto hue2rgb = [](double p, double q, double tt) {
        if (tt < 0) tt += 1; if (tt > 1) tt -= 1;
        if (tt < 1 / 6.0) return p + (q - p) * 6 * tt;
        if (tt < 1 / 2.0) return q;
        if (tt < 2 / 3.0) return p + (q - p) * (2 / 3.0 - tt) * 6;
        return p;
    };
    double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    double p = 2 * l - q;
    double r = hue2rgb(p, q, h + 1 / 3.0);
    double g = hue2rgb(p, q, h);
    double b = hue2rgb(p, q, h - 1 / 3.0);
    return QVector3D((float)r, (float)g, (float)b);
}

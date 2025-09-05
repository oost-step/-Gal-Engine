#include "MainWindow.h"
#include "ResourceManager.h"
#include "StartWindow.h"
#include <QFileDialog>
#include <QStatusBar>
#include <QMessageBox>
#include <QDir>
#include <QAction>
#include <QWidget>
#include <QMenuBar>
#include <QKeyEvent>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("GalEngine Qt6");
    //resize(1280, 720);
    setFixedSize(1280, 720);
    setStyleSheet("QMainWindow { background: white; }");

    m_bg = new QLabel(this);
    m_bg->setScaledContents(true);
    m_bg->setGeometry(rect());

    m_layer = new ImageLayer(this);
    m_layer->setGeometry(rect());

    m_layerT = new ImageLayerTop(this);
    m_layerT->setGeometry(rect());

    m_dialogue = new DialogueBox(this);
    m_choices = new ChoiceOverlay(this);

    m_engine = new ScriptEngine(this);
    m_audio = new AudioManager(this);

    // ---------- Return °´Å¥ ----------
    m_returnBtn = new QPushButton("Return", this);
    m_returnBtn->setFixedSize(80, 30);
    m_returnBtn->move(width() - m_returnBtn->width() - 10, 10); // ÓÒÉÏ½ÇÎ»ÖÃ
    m_returnBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(70, 70, 70, 180);
            color: white;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(100, 100, 200, 220);
            color: white;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:pressed {
            background-color: rgba(50, 50, 150, 255);
            color: white;
            font-size: 18px;
            font-weight: bold;
        }
    )");

    connect(m_returnBtn, &QPushButton::clicked, this, &MainWindow::onReturnClicked);
    m_returnBtn->raise();

    connect(m_engine, &ScriptEngine::backgroundChanged, this, &MainWindow::onBackgroundChanged);
    connect(m_engine, &ScriptEngine::spriteChanged, this, &MainWindow::onSpriteChanged);
    connect(m_engine, &ScriptEngine::spriteChangedTop, this, &MainWindow::onSpriteChangedTop);
    connect(m_engine, &ScriptEngine::spriteCleared, this, &MainWindow::onSpriteCleared);
    connect(m_engine, &ScriptEngine::spriteClearedTop, this, &MainWindow::onSpriteClearedTop);
    connect(m_engine, &ScriptEngine::textReady, this, &MainWindow::onTextReady);
    connect(m_engine, &ScriptEngine::choiceRequested, this, &MainWindow::onChoiceRequested);
    connect(m_engine, &ScriptEngine::preloadRequested, this, &MainWindow::onPreloadRequested);
    connect(m_engine, &ScriptEngine::autosavePoint, this, &MainWindow::onAutosavePoint);

    connect(m_engine, &ScriptEngine::stopBgm, m_audio, &AudioManager::stopBgm);
    connect(m_engine, &ScriptEngine::playBgm, m_audio, &AudioManager::playBgm);
    connect(m_engine, &ScriptEngine::playSe, m_audio, &AudioManager::playSe);

    connect(m_dialogue, &DialogueBox::clicked, m_engine, &ScriptEngine::advance);
    connect(m_choices, &ChoiceOverlay::choiceSelected, m_engine, &ScriptEngine::onChoiceSelected);

    layoutUi();

    // Load script
    QString json = QDir::current().filePath("script.json");
    if (!QFile::exists(json)) {
        json = QFileDialog::getOpenFileName(this, "Open Script JSON", QDir::currentPath(), "JSON (*.json)");
    }
    if (!json.isEmpty() && m_engine->loadFromJsonFile(json)) {
        statusBar()->showMessage("Loaded script: " + json);
        m_engine->start();
    }
    else {
        QMessageBox::warning(this, "GalEngine", "Failed to load JSON script.");
    }

    // menu actions
    auto* fileMenu = menuBar()->addMenu("&File");
    auto* saveAct = fileMenu->addAction("Save Game");
    auto* loadAct = fileMenu->addAction("Load Game");
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveGame);
    connect(loadAct, &QAction::triggered, this, &MainWindow::loadGameFromDialog);
}

void MainWindow::resizeEvent(QResizeEvent* ev) {
    QMainWindow::resizeEvent(ev);
    layoutUi();

    if (m_returnBtn) {
        m_returnBtn->move(width() - m_returnBtn->width() - 10, 10);
    }
}

void MainWindow::layoutUi() {
    const QRect r = rect();
    m_bg->setGeometry(r);
    m_layer->setGeometry(r);
    const int h = height();
    const int w = width();
    const int dlgH = h / 3;
    m_dialogue->setGeometry(0, h - dlgH, w, dlgH);
    m_choices->setGeometry(r);
}

void MainWindow::keyPressEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Space || ev->key() == Qt::Key_Return) m_engine->advance();
    else QMainWindow::keyPressEvent(ev);
}

void MainWindow::onBackgroundChanged(const QString& path) {
    auto& rm = ResourceManager::instance();
    QPixmap px;
    if (rm.hasPixmap(path)) {
        px = rm.getPixmap(path);
    }
    else {
        px = QPixmap(path);
    }
    if (px.isNull()) return;

    m_bgPixmap = px;
    m_bg->setPixmap(m_bgPixmap);

    if (auto* oldEff = qobject_cast<QGraphicsOpacityEffect*>(m_bg->graphicsEffect())) {
        m_bg->setGraphicsEffect(nullptr);
        //oldEff->deleteLater();
    }

    const int fadeDurationMs = 400;

    auto* effect = new QGraphicsOpacityEffect(m_bg);
    effect->setOpacity(0.0);
    m_bg->setGraphicsEffect(effect);

    auto* anim = new QPropertyAnimation(effect, "opacity", m_bg);
    anim->setDuration(fadeDurationMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);

    QObject::connect(anim, &QPropertyAnimation::finished, m_bg, [this, effect]() {
        m_bg->setGraphicsEffect(nullptr); // ÒÆ³ýÐ§¹û
        //effect->deleteLater();
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onSpriteChanged(const QString& slot, const QString& path) {
    auto& rm = ResourceManager::instance();
    if (rm.hasPixmap(path)) {
        m_layer->setSprite(slot, rm.getPixmap(path));
        return;
    }
    QPixmap px(path);
    if (!px.isNull()) m_layer->setSprite(slot, px);
}

void MainWindow::onSpriteChangedTop(const QString& slot, const QString& path) {
    auto& rm = ResourceManager::instance();
    if (rm.hasPixmap(path)) {
        m_layerT->setSpriteTop(slot, rm.getPixmap(path));
        return;
    }
    QPixmap px(path);
    if (!px.isNull()) m_layerT->setSpriteTop(slot, px);
}

void MainWindow::onSpriteCleared(const QString& slot) {
    m_layer->clearSprite(slot);
}

void MainWindow::onSpriteClearedTop(const QString& slot) {
    m_layerT->clearSpriteTop(slot);
}

void MainWindow::onTextReady(const QString& speaker, const QString& text) {
    m_dialogue->setSpeaker(speaker);
    m_dialogue->setText(text);
}

void MainWindow::onChoiceRequested(const QString& prompt, const QStringList& options) {
    m_choices->setChoices(prompt, options);
}

void MainWindow::onPreloadRequested(const QStringList& images, const QStringList& audios) {
    auto& rm = ResourceManager::instance();
    rm.preloadImages(images);
    for (const auto& a : audios) rm.registerAudio(a);
    if (!images.isEmpty() || !audios.isEmpty()) {
        statusBar()->showMessage(QString("Preloaded %1 images, %2 audios").arg(images.count()).arg(audios.count()), 3000);
    }
}

void MainWindow::onAutosavePoint(const QString& name) {
    QString fn = QDir::current().filePath(QString("autosave_%1.json").arg(name));
    m_engine->saveSnapshotToFile(fn);
    statusBar()->showMessage("Autosaved: " + fn, 3000);
}

void MainWindow::saveGame() {
    QString fn = QFileDialog::getSaveFileName(this, "Save Game", QDir::currentPath(), "Save (*.json)");
    if (fn.isEmpty()) return;
    m_engine->saveSnapshotToFile(fn);
    statusBar()->showMessage("Saved: " + fn, 3000);
}

void MainWindow::loadGameFromDialog() {
    QString fn = QFileDialog::getOpenFileName(this, "Load Game", QDir::currentPath(), "Save (*.json)");
    if (fn.isEmpty()) return;
    m_engine->loadSnapshotFromFile(fn);
    statusBar()->showMessage("Loaded: " + fn, 3000);
}

void MainWindow::loadGame() {
    // Try default autosave
    QString fn = QDir::current().filePath("autosave_last.json");
    if (QFile::exists(fn)) {
        m_engine->loadSnapshotFromFile(fn);
        statusBar()->showMessage("Loaded: " + fn, 3000);
    }
    else {
        // select
        loadGameFromDialog();
    }
}

void MainWindow::onReturnClicked() {
    m_engine->stopBgm(); // Í£Ö¹±³¾°ÒôÀÖ
    if (!m_startWindow) {
        m_startWindow = new StartWindow();
    }
    m_startWindow->show();
    this->close();
}
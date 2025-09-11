#include "MainWindow.h"
#include "ResourceManager.h"
#include "StartWindow.h"
#include "SaveLoadWindow.h"
#include "SettingWindow.h"
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
#include <QTextEdit>
#include <QBoxLayout>
#include <QSequentialAnimationGroup>
#include <QToolBar>
#include <QApplication>
#include <QToolButton>
#include <QLineEdit>
#include <QTextEdit>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("GalEngine");
    setFixedSize(1280, 720);
    setStyleSheet("QMainWindow { background: white; }");

    m_ctrlLongPressTimer = new QTimer(this);
    m_ctrlLongPressTimer->setInterval(500); // 300ms 判定为长按
    m_ctrlLongPressTimer->setSingleShot(false); // 重复触发
    connect(m_ctrlLongPressTimer, &QTimer::timeout, this, [this]() {
        handleAdvanceOrSkip(); // 连续触发
    });

    qApp->installEventFilter(this);

    bottomToolBar = new QToolBar(this);
    addToolBar(Qt::BottomToolBarArea, bottomToolBar);

    bottomToolBar->raise();

    bottomToolBar->setMovable(false);      
    bottomToolBar->setFloatable(false);   

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bottomToolBar->addWidget(spacer);

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

    m_autoWaitCount = 0;
    m_skipAllMode = false;

    m_modeTimer = new QTimer(this);
    m_autoDelayTimer = new QTimer(this); // 用于自动模式的延迟计时
    m_autoDelayTimer->setSingleShot(true); // 设置为单次触发

    connect(m_modeTimer, &QTimer::timeout, this, [this]() {
        if (g_skipMode) {
            // 快进模式：立即跳过当前文本并推进
            if (m_dialogue->isTyping()) {
                m_dialogue->skipTyping();
            }
            m_engine->advance();
        }
    });

    // 连接文本动画完成信号到自动模式处理
    connect(m_dialogue, &DialogueBox::textAnimationComplete, this, [this]() {
        if (g_autoMode && !g_skipMode) {
            // 文本动画完成后，启动延迟计时器
            m_autoDelayTimer->start(1000); // 1秒后自动前进
        }
    });

    // 连接自动延迟计时器
    connect(m_autoDelayTimer, &QTimer::timeout, this, [this]() {
        if (g_autoMode && !g_skipMode) {
            m_engine->advance();
        }
    });

    m_modeTimer->start(100); // 每100ms检查一次模式


    connect(m_engine, &ScriptEngine::backgroundChanged, this, &MainWindow::onBackgroundChanged);
    connect(m_engine, &ScriptEngine::spriteChanged, this, &MainWindow::onSpriteChanged);
    connect(m_engine, &ScriptEngine::spriteChangedTop, this, &MainWindow::onSpriteChangedTop);
    connect(m_engine, &ScriptEngine::spriteCleared, this, &MainWindow::onSpriteCleared);
    connect(m_engine, &ScriptEngine::spriteClearedTop, this, &MainWindow::onSpriteClearedTop);
    connect(m_engine, &ScriptEngine::textReady, this, &MainWindow::onTextReady);
    connect(m_engine, &ScriptEngine::choiceRequested, this, &MainWindow::onChoiceRequested);
    connect(m_engine, &ScriptEngine::preloadRequested, this, &MainWindow::onPreloadRequested);
    connect(m_engine, &ScriptEngine::autosavePoint, this, &MainWindow::onAutosavePoint);
    connect(m_engine, &ScriptEngine::shakeWindow, this, &MainWindow::onShakeWindow);
    connect(m_engine, &ScriptEngine::close, this, &MainWindow::onClose);

    connect(m_engine, &ScriptEngine::onBackGame, this, &MainWindow::onReturnClicked);

    connect(m_engine, &ScriptEngine::stopBgm, m_audio, &AudioManager::stopBgm);
    connect(m_engine, &ScriptEngine::playBgm, m_audio, &AudioManager::playBgm);
    connect(m_engine, &ScriptEngine::playSe, m_audio, &AudioManager::playSe);

    connect(m_choices, &ChoiceOverlay::choiceSelected, m_engine, &ScriptEngine::onChoiceSelected);

    connect(this, &MainWindow::playSound, m_audio, &AudioManager::playSe);

    layoutUi();


    QString jsonPath;
    if (ResourceManager::USE_PACKED_RESOURCES) {
        jsonPath = "assets/script.json";
    }
    else {
        jsonPath = QDir::current().filePath("script.json");
    }

    QString script = ResourceManager::instance().loadTextFile(jsonPath);

    if (script.isNull()) {
        jsonPath = QFileDialog::getOpenFileName(this, "Open Script JSON", QDir::currentPath(), "JSON (*.json)");
        if (!jsonPath.isEmpty() && m_engine->loadFromJsonFile(jsonPath)) {
            m_engine->start();
        }
        else {
            QMessageBox::warning(this, "GalEngine", "Failed to load JSON script.");
        }
    }
    else {
        if (m_engine->loadFromJsonFile(jsonPath)) {
            m_engine->start();
        }
        else {
            QMessageBox::warning(this, "GalEngine", "Failed to load JSON script.");
        }
    }


    auto* saveAct = new QAction("Save", this);  
    connect(saveAct, &QAction::triggered, this, [this]() {
        emit playSound("resources/save.mp3");
        SaveLoadWindow dlg(m_engine, SaveLoadWindow::SaveMode, this);
        connect(&dlg, &SaveLoadWindow::saveToSlot, this, &MainWindow::saveToSlot);
        connect(&dlg, &SaveLoadWindow::loadFromSlot, this, &MainWindow::loadFromSlot);
        dlg.exec();
    });
    bottomToolBar->addAction(saveAct);


    auto* loadAct = new QAction("Load");  
    connect(loadAct, &QAction::triggered, this, [this]() {
        emit playSound("resources/load.mp3");
        SaveLoadWindow dlg(m_engine, SaveLoadWindow::LoadMode, this);
        connect(&dlg, &SaveLoadWindow::saveToSlot, this, &MainWindow::saveToSlot);
        connect(&dlg, &SaveLoadWindow::loadFromSlot, this, &MainWindow::loadFromSlot);
        dlg.exec();
    });
    bottomToolBar->addAction(loadAct);

    auto* settingAct = new QAction("Setting", this);
    connect(settingAct, &QAction::triggered, this, [this]() {
        m_tempSnapshot = m_engine->snapshot();

        // 传 caller=this，确保 SettingWindow 能识别来源
        auto* setting = new SettingWindow(nullptr, true, this);

        connect(setting, &SettingWindow::closedFromMainWindow, this, [this]() {
            if (!m_tempSnapshot.isEmpty()) {
                m_engine->restore(m_tempSnapshot);
            }
            this->show();
        });

        this->hide();
        setting->show();
    });
    bottomToolBar->addAction(settingAct);

    auto* viewAct = new QAction("View");
    connect(viewAct, &QAction::triggered, this, &MainWindow::showHistory);
    bottomToolBar->addAction(viewAct);

    auto* returnAct = new QAction("Return");
    connect(returnAct, &QAction::triggered, this, &MainWindow::onReturnClicked);
    bottomToolBar->addAction(returnAct);

    bottomToolBar->setIconSize(QSize(32, 32));

    bottomToolBar->setFixedHeight(32);

    bottomToolBar->setStyleSheet(R"(
    QToolBar {
        background-color: transparent; /* 工具栏背景透明 */
        border: none;                 /* 移除边框 */
        spacing: 3px;                 /* 根据需要调整工具项间距 */
    }
    QToolBar::separator {
        background: transparent;      /* 分隔符也透明 */
    }
    QToolBar QToolButton, 
    QToolBar QAction {
        color: white;               /* 白色字体 */
        font-size: 12px;            /* 字号大小 */
        font-family: "Microsoft YaHei"; /* 可选：设置字体 */
        padding: 6px 6px;
        background: transparent;
        border: 1px solid transparent;
        border-radius: 4px;
    }
    QToolBar QToolButton:hover, 
    QToolBar QAction:hover {
        background-color: #3E3E40;  /* 悬停效果 */
        border: 1px solid #555555;
    }
    QToolBar QToolButton:pressed, 
    QToolBar QAction:pressed {
        background-color: #007ACC;  /* 按下效果 */
    }
    QToolBar QToolButton:menu-indicator {
        image: none;               /* 隐藏菜单指示器 */
    }
    )");
}

MainWindow::~MainWindow() {
    qApp->removeEventFilter(this);
}

void MainWindow::resizeEvent(QResizeEvent* ev) {
    QMainWindow::resizeEvent(ev);
    if (bottomToolBar) {
        bottomToolBar->raise();
    }
    layoutUi();
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
    if (ev->key() == Qt::Key_Space || ev->key() == Qt::Key_Return) {
        if (!ev->isAutoRepeat()) {
            handleAdvanceOrSkip();
        }
    }
    else if (ev->key() == Qt::Key_Control) {
        if (!ev->isAutoRepeat() && !m_ctrlLongPressTimer->isActive()) {
            m_ctrlLongPressTimer->start();

            // 按下Ctrl键时启用跳过所有模式
            enableSkipAllMode(true);
        }
    }
    else {
        QMainWindow::keyPressEvent(ev);
    }
}

// 修改keyReleaseEvent以支持释放Ctrl键时退出跳过所有模式
void MainWindow::keyReleaseEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Control) {
        m_ctrlLongPressTimer->stop();

        // 释放Ctrl键时退出跳过所有模式
        enableSkipAllMode(false);
    }
    else {
        QMainWindow::keyReleaseEvent(ev);
    }
}

void MainWindow::onBackgroundChanged(const QString& path) {
    auto& rm = ResourceManager::instance();
    QPixmap px = rm.getPixmap(path);

    if (px.isNull()) return;

    m_bgPixmap = px;
    m_bg->setPixmap(m_bgPixmap);

    if (auto* oldEff = qobject_cast<QGraphicsOpacityEffect*>(m_bg->graphicsEffect())) {
        m_bg->setGraphicsEffect(nullptr);
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
        m_bg->setGraphicsEffect(nullptr);
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onSpriteChanged(const QString& slot, const QString& path) {
    auto& rm = ResourceManager::instance();
    QPixmap px = rm.getPixmap(path);
    if (!px.isNull()) m_layer->setSprite(slot, px);
}

void MainWindow::onSpriteChangedTop(const QString& slot, const QString& path) {
    auto& rm = ResourceManager::instance();
    QPixmap px = rm.getPixmap(path);
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

    // 追加到历史记录
    QString line = speaker.isEmpty() ? text : QString("%1: %2").arg(speaker, text);
    m_history.append(line);

    // 限制历史记录长度，避免过大
    const int maxHistory = 200;
    if (m_history.size() > maxHistory) {
        m_history.removeFirst();
    }
    m_currentText = text;
}

void MainWindow::onChoiceRequested(const QString& prompt, const QStringList& options) {
    // 遇到选择枝时退出跳过所有模式
    enableSkipAllMode(false);
    m_choices->setChoices(prompt, options);
}

void MainWindow::onPreloadRequested(const QStringList& images, const QStringList& audios) {
    auto& rm = ResourceManager::instance();
    rm.preloadImages(images);
    for (const auto& a : audios) rm.registerAudio(a);
    if (!images.isEmpty() || !audios.isEmpty()) {
        //statusBar()->showMessage(QString("Preloaded %1 images, %2 audios").arg(images.count()).arg(audios.count()), 3000);
    }
}

void MainWindow::onAutosavePoint(const QString& name) {
    QString fn = QDir::current().filePath(QString("autosave_%1.json").arg(name));
    m_engine->saveSnapshotToFile(fn);
    //statusBar()->showMessage("Autosaved: " + fn, 3000);
}

void MainWindow::saveGame() {
    QString fn = QFileDialog::getSaveFileName(this, "Save Game", QDir::currentPath(), "Save (*.json)");
    if (fn.isEmpty()) return;
    m_engine->saveSnapshotToFile(fn);
    //statusBar()->showMessage("Saved: " + fn, 3000);
}

void MainWindow::loadGameFromDialog() {
    QString fn = QFileDialog::getOpenFileName(this, "Load Game", QDir::currentPath(), "Save (*.json)");
    if (fn.isEmpty()) return;
    m_engine->loadSnapshotFromFile(fn);
    //statusBar()->showMessage("Loaded: " + fn, 3000);
}

void MainWindow::loadGame() {
    // Try default autosave
    QString fn = QDir::current().filePath("autosave_last.json");
    if (QFile::exists(fn)) {
        m_history.clear();
        m_engine->loadSnapshotFromFile(fn);
        //statusBar()->showMessage("Loaded: " + fn, 3000);
    }
    else {
        // select
        m_history.clear();
        loadGameFromDialog();
    }
}

void MainWindow::onReturnClicked()
{
    // 停止所有定时器和模式
    m_modeTimer->stop();
    m_autoDelayTimer->stop();
    m_ctrlLongPressTimer->stop();
    enableSkipAllMode(false);

    m_engine->stopBgm();
    if (!m_startWindow) {
        m_startWindow = new StartWindow();
    }
    m_startWindow->show();
    this->close();
}

void MainWindow::showHistory() {
    QDialog dlg(this);
    dlg.setWindowTitle("History");
    dlg.resize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QTextEdit* textEdit = new QTextEdit(&dlg);
    textEdit->setReadOnly(true);
    textEdit->setText(m_history.join("\n\n"));
    layout->addWidget(textEdit);

    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::saveToSlot(int slotIndex) {
    if (!m_engine) return;

    QDir().mkpath("saves");

    // 1. 截取当前窗口为缩略图
    QPixmap screenshot = this->grab();
    QString screenshotPath = QString("saves/save_slot_%1.png").arg(slotIndex);
    screenshot = screenshot.scaled(320, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    screenshot.save(screenshotPath, "PNG");

    // 2. 获取存档描述：取对话框当前文本前 30 个字符
    QString desc;
    if (m_dialogue) {
        desc = m_currentText;
        if (desc.length() > 30) desc = desc.left(30) + "...";
    }

    // 3. 调用 ScriptEngine 保存
    QString jsonFile = QString("saves/save_slot_%1.json").arg(slotIndex);
    m_engine->saveSnapshotWithMeta(jsonFile, screenshotPath, desc);

    // 4. 提示用户
    //statusBar()->showMessage(QString("Saved to slot %1").arg(slotIndex), 3000);
}

void MainWindow::loadFromSlot(int slotIndex) {
    if (!m_engine) return;

    QString jsonFile = QString("saves/save_slot_%1.json").arg(slotIndex);
    if (!QFile::exists(jsonFile)) {
        //statusBar()->showMessage(QString("Slot %1 is empty").arg(slotIndex), 3000);
        return;
    }

    m_history.clear();
    m_engine->loadSnapshotFromFile(jsonFile);

    //statusBar()->showMessage(QString("Loaded from slot %1").arg(slotIndex), 3000);
}

void MainWindow::startWindowContinue()
{
    SaveLoadWindow dlg(m_engine, SaveLoadWindow::LoadMode, this);
    connect(&dlg, &SaveLoadWindow::saveToSlot, this, &MainWindow::saveToSlot);
    connect(&dlg, &SaveLoadWindow::loadFromSlot, this, &MainWindow::loadFromSlot);
    dlg.exec();
}

void MainWindow::onShakeWindow(int amplitude, int duration, int shakeCount)
{
    if (amplitude <= 0) {
        amplitude = 50; // 设置合理的默认值
    }
    if (shakeCount <= 0) {
        shakeCount = 10;
    }
    if (duration <= 0) {
        duration = 1500;
    }

    // 检查并清理旧的动画指针 - 使用 data() 获取原始指针
    if (!m_shakeAnimation.isNull()) {
        // 断开连接 - 使用 m_shakeAnimation.data() 获取原始指针
        disconnect(m_shakeAnimation.data(), nullptr, this, nullptr);
        if (m_shakeAnimation->state() == QAbstractAnimation::Running) {
            m_shakeAnimation->stop();
        }
        m_shakeAnimation.clear(); // 清除 QPointer
    }

    // 保存每个子控件的初始位置
    QList<QWidget*> widgets = this->findChildren<QWidget*>();
    QMap<QWidget*, QPoint> initialPosMap;
    for (QWidget* w : widgets) {
        if (w->isWindow()) continue; // 跳过子窗口
        initialPosMap[w] = w->pos();
    }

    // 创建新的动画
    QVariantAnimation* anim = new QVariantAnimation(this);
    m_shakeAnimation = anim; // 正确赋值给 QPointer

    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::Linear);

    connect(anim, &QVariantAnimation::valueChanged, this, [=](const QVariant& value) {
        double progress = value.toDouble();
        double angle = progress * shakeCount * 2 * M_PI;
        double decay = 1.0 - progress;

        double dx = amplitude * decay * std::sin(angle);
        double dy = amplitude * decay * std::cos(angle * 0.8);

        QPoint offset((int)dx, (int)dy);

        for (auto it = initialPosMap.begin(); it != initialPosMap.end(); ++it) {
            QWidget* w = it.key();
            QPoint base = it.value();
            w->move(base + offset);
        }
    });

    connect(anim, &QVariantAnimation::finished, this, [=]() {
        // 归位
        for (auto it = initialPosMap.begin(); it != initialPosMap.end(); ++it) {
            it.key()->move(it.value());
        }
        // 动画完成后自动清除指针
        if (m_shakeAnimation == anim) {
            m_shakeAnimation.clear();
        }
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}


void MainWindow::onClose()
{
    this->close();
}

void MainWindow::handleAdvance()
{
    if (!m_dialogue || !m_engine) return;
    if (!iswaiting){
        if (!m_dialogue->isTyping()) {
            qDebug() << "skip animation！";
            m_dialogue->skipTyping();
        }
        else if (m_engine) {
            qDebug() << "skip text！";
            m_engine->advance();
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent* ev) {
    QWidget* child = childAt(ev->pos());

    // 如果点击的是工具栏按钮，不拦截
    if (child && qobject_cast<QToolButton*>(child)) {
        QMainWindow::mousePressEvent(ev);
        return;
    }

}

void MainWindow::handleAdvanceOrSkip() {
    if (!m_dialogue) return;

    if (m_skipAllMode) {
        // 在跳过所有模式下，直接推进而不检查文本状态
        m_engine->advance();
    }
    else if (!g_autoMode && !g_skipMode) {
        // 默认模式 → 手动推进
        if (!iswaiting) {
            if (!m_dialogue->isTyping()) {
                qDebug() << "skip animation！";
                m_dialogue->skipTyping();
            }
            else if (m_engine) {
                qDebug() << "skip text！";
                m_engine->advance();
            }
        }
    }
    // 如果是auto/skip模式，就交给m_modeTimer自动驱动
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
    if (ev->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(ev);
        if (me->button() != Qt::LeftButton) return false;

        QWidget* w = QApplication::widgetAt(me->globalPosition().toPoint());

        // 工具栏按钮 → 不拦截
        if (w && (qobject_cast<QToolButton*>(w) || qobject_cast<QAbstractButton*>(w)))
            return false;

        if (w) {
            // 检查点击的窗口是否是模态对话框
            QWidget* topLevel = w->window();
            if (topLevel && topLevel != this && topLevel->isModal()) {
                return false; // 不拦截模态对话框内的点击
            }

            // 检查点击的窗口是否是子窗口（如History对话框）
            if (topLevel && topLevel != this && topLevel->isWindow()) {
                return false; // 不拦截子窗口内的点击
            }
        }

        // 其它区域（包括对话框、背景）
        if (m_dialogue) {
            if (!iswaiting) {
                if (!m_dialogue->isTyping()) {
                    m_dialogue->skipTyping();
                }
                else if (m_engine) {
                    m_engine->advance();
                }
            }
        }

        return true; // return true，避免事件再传到子控件导致重复
    }

    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::enableSkipAllMode(bool enable) {
    m_skipAllMode = enable;

    if (m_skipAllMode) {
        // 进入跳过所有模式时，立即跳过当前文本
        if (m_dialogue->isTyping()) {
            m_dialogue->skipTyping();
        }

        // 设置更快的定时器间隔以实现快速跳过
        m_modeTimer->setInterval(5);
    }
    else {
        // 退出跳过所有模式时，恢复正常定时器间隔
        m_modeTimer->setInterval(100);
    }
}

void MainWindow::hideEvent(QHideEvent* event)
{
    QMainWindow::hideEvent(event);

    // 停止所有定时器
    m_modeTimer->stop();
    m_autoDelayTimer->stop();
    m_ctrlLongPressTimer->stop();

    // 退出跳过所有模式
    enableSkipAllMode(false);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    // 如果处于自动或快进模式，重新启动定时器
    if (g_autoMode || g_skipMode) {
        m_modeTimer->start(100);
    }
}
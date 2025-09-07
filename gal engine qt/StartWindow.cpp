// StartWindow.cpp
#include "StartWindow.h"
#include "SettingWindow.h"
#include "GalleryWindow.h"
#include "MainWindow.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include <QVBoxLayout>
#include <QFile>
#include <QApplication>
#include <QPushButton>
#include <QString>
#include <QPainter>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDebug>

// 配置变量
const QString BACKGROUND_IMAGE_PATH = "resources/background.jpg"; // 底图文件路径
const QString LOGO_IMAGE_PATH = "resources/logo.png"; // logo文件路径
const double LOGO_SCALE_FACTOR = 0.3; // logo缩放因子
const int BUTTON_WIDTH = 240; // 按钮宽度
const int BUTTON_HEIGHT = 60; // 按钮高度
const int RIGHT_MARGIN = 20; // 右侧边距
const int BOTTOM_MARGIN = 0; // 底部边距

// 音频文件路径
const QString BGM_PATH = "resources/little explorer.mp3";
const QString START_SOUND_PATH = "resources/start.mp3";
const QString CONTINUE_SOUND_PATH = "resources/Continue.mp3";
const QString GALLERY_SOUND_PATH = "resources/gallery.mp3";
const QString SETTING_SOUND_PATH = "resources/setting.mp3";
const QString EXIT_SOUND_PATH = "resources/exit.mp3";

StartWindow::StartWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("GalEngine - Start");
    setFixedSize(1280, 720);

    // 创建音频管理器
    m_audioManager = new AudioManager(this);

    // 预加载图片资源
    ResourceManager::instance().preloadImage(BACKGROUND_IMAGE_PATH);
    ResourceManager::instance().preloadImage(LOGO_IMAGE_PATH);

    // 注册音频资源
    ResourceManager::instance().registerAudio(BGM_PATH);
    ResourceManager::instance().registerAudio(START_SOUND_PATH);
    ResourceManager::instance().registerAudio(CONTINUE_SOUND_PATH);
    ResourceManager::instance().registerAudio(GALLERY_SOUND_PATH);
    ResourceManager::instance().registerAudio(SETTING_SOUND_PATH);
    ResourceManager::instance().registerAudio(EXIT_SOUND_PATH);

    // 播放背景音乐
    if (ResourceManager::instance().hasAudio(BGM_PATH)) {
        m_audioManager->playBgm(BGM_PATH);
    }
    else {
        qDebug() << "BGM file not found:" << BGM_PATH;
    }

    // 设置样式表用于按钮悬停效果
    setStyleSheet(R"(
        QPushButton {
            background-color: rgba(70, 70, 70, 180);
            color: white;
            border: 2px solid #555;
            border-radius: 10px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(100, 100, 200, 220);
            border: 2px solid #888;
            color: white;
            border: 2px solid #555;
            border-radius: 20px;
            font-size: 22px;
            font-weight: bold;
        }
        QPushButton:pressed {
            background-color: rgba(50, 50, 150, 255);
            border: 2px solid #666;
            color: white;
            border: 2px solid #555;
            border-radius: 20px;
            font-size: 22px;
            font-weight: bold;
        }
    )");

    // 使用水平布局将按钮放置在右侧
    auto* mainLayout = new QHBoxLayout(this);

    // 左侧占位空间
    auto* leftSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainLayout->addItem(leftSpacer);

    // 右侧按钮区域
    auto* rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->setSpacing(20);

    startBtn = new QPushButton(QString::fromLocal8Bit("开始"), this);
    continueBtn = new QPushButton(QString::fromLocal8Bit("继续"), this);
    galleryBtn = new QPushButton(QString::fromLocal8Bit("Gallery"), this);
    settingBtn = new QPushButton(QString::fromLocal8Bit("设置"), this);
    exitBtn = new QPushButton(QString::fromLocal8Bit("结束"), this);

    // 设置按钮大小
    startBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    continueBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    galleryBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    settingBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    exitBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);

    // 添加到右侧布局
    rightLayout->addWidget(startBtn);
    rightLayout->addWidget(continueBtn);
    rightLayout->addWidget(galleryBtn);
    rightLayout->addWidget(settingBtn);
    rightLayout->addWidget(exitBtn);

    // 在按钮上方添加弹性空间，使按钮垂直居中
    rightLayout->insertStretch(0, 1);
    rightLayout->addStretch(1);

    // 将右侧布局添加到主布局
    mainLayout->addLayout(rightLayout);
    mainLayout->setAlignment(rightLayout, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->setContentsMargins(0, 0, RIGHT_MARGIN, 0);

    connect(startBtn, &QPushButton::clicked, this, &StartWindow::onStartGame);
    connect(continueBtn, &QPushButton::clicked, this, &StartWindow::onContinueGame);
    connect(galleryBtn, &QPushButton::clicked, this, &StartWindow::onGalleryGame);
    connect(settingBtn, &QPushButton::clicked, this, &StartWindow::onSettingGame);
    connect(exitBtn, &QPushButton::clicked, this, &StartWindow::onExitGame);

    // 从 ResourceManager 获取 logo 图片
    logoPixmap = ResourceManager::instance().getPixmap(LOGO_IMAGE_PATH);
    if (logoPixmap.isNull()) {
        qDebug() << "Failed to load logo image:" << LOGO_IMAGE_PATH;
        // 尝试直接加载
        logoPixmap.load(LOGO_IMAGE_PATH);
    }
}

StartWindow::~StartWindow()
{
    if (m_mainWindow)
        delete m_mainWindow;
}

void StartWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    // 从 ResourceManager 获取背景图片
    QPixmap background = ResourceManager::instance().getPixmap(BACKGROUND_IMAGE_PATH);
    if (background.isNull()) {
        // 如果从ResourceManager获取失败，尝试直接加载
        background.load(BACKGROUND_IMAGE_PATH);
    }

    if (!background.isNull()) {
        // 缩放背景图以填充整个窗口
        painter.drawPixmap(0, 0, width(), height(), background);
    }
    else {
        // 如果背景图加载失败，使用默认背景色
        painter.fillRect(rect(), QColor(50, 50, 50));
        qDebug() << "Failed to load background image:" << BACKGROUND_IMAGE_PATH;
    }

    // 绘制logo在左下角
    if (!logoPixmap.isNull()) {
        // 根据缩放因子计算logo大小
        int logoWidth = logoPixmap.width() * LOGO_SCALE_FACTOR;
        int logoHeight = logoPixmap.height() * LOGO_SCALE_FACTOR;

        // 计算左下角位置（考虑边距）
        int x = BOTTOM_MARGIN - 30;
        int y = height() - logoHeight - BOTTOM_MARGIN + 35;

        // 绘制logo
        painter.drawPixmap(x, y, logoWidth, logoHeight, logoPixmap);
    }
}

void StartWindow::onStartGame()
{
    m_audioManager->stopBgm();

    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(START_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(START_SOUND_PATH, [this]() {
            if (!m_mainWindow) {
                m_mainWindow = new MainWindow();
            }
            m_mainWindow->show();
            this->close();
        });
    }
    else {
        qDebug() << "Start sound file not found:" << START_SOUND_PATH;
        // 即使没有音效也继续执行
        if (!m_mainWindow) {
            m_mainWindow = new MainWindow();
        }
        m_mainWindow->show();
        this->close();
    }
}

void StartWindow::onContinueGame()
{
    m_audioManager->stopBgm();

    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(CONTINUE_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(CONTINUE_SOUND_PATH, [this]() {
            if (!m_mainWindow)
                m_mainWindow = new MainWindow();

            m_mainWindow->startWindowContinue();
            m_mainWindow->show();
            this->close();
        });
    }
    else {
        qDebug() << "Continue sound file not found:" << CONTINUE_SOUND_PATH;
        // 即使没有音效也继续执行
        if (!m_mainWindow)
            m_mainWindow = new MainWindow();

        m_mainWindow->startWindowContinue();
        m_mainWindow->show();
        this->close();
    }
}

void StartWindow::onGalleryGame()
{
    m_audioManager->stopBgm();

    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(GALLERY_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(GALLERY_SOUND_PATH, [this]() {
            if (!m_galleryWindow)
                m_galleryWindow = new GalleryWindow();
            m_galleryWindow->show();
            this->close();
        });
    }
    else {
        qDebug() << "Gallery sound file not found:" << GALLERY_SOUND_PATH;
        // 即使没有音效也继续执行
        if (!m_galleryWindow)
            m_galleryWindow = new GalleryWindow();
        m_galleryWindow->show();
        this->close();
    }
}

void StartWindow::onSettingGame()
{
    m_audioManager->stopBgm();

    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(SETTING_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(SETTING_SOUND_PATH, [this]() {
            if (!m_settingWindow)
                m_settingWindow = new SettingWindow();
            m_settingWindow->show();
            this->close();
        });
    }
    else {
        qDebug() << "Setting sound file not found:" << SETTING_SOUND_PATH;
        // 即使没有音效也继续执行
        if (!m_settingWindow)
            m_settingWindow = new SettingWindow();
        m_settingWindow->show();
        this->close();
    }
}

void StartWindow::onExitGame()
{
    m_audioManager->stopBgm();

    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(EXIT_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(EXIT_SOUND_PATH, [this]() {
            QApplication::quit();
        });
    }
    else {
        qDebug() << "Exit sound file not found:" << EXIT_SOUND_PATH;
        // 即使没有音效也退出
        QApplication::quit();
    }
}
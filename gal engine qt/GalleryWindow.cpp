#include "GalleryWindow.h"
#include "StartWindow.h"
#include "ClickableLabel.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QFileInfoList>
#include <QPixmap>
#include <QScrollArea>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPainter>

const float g_scaler = 1.0;
const QString BACKGROUND_IMAGE_PATH = "";
const QString LOGO_IMAGE_PATH = "resources/logo.png";
const double LOGO_SCALE_FACTOR = 0.3;
const int BUTTON_WIDTH = 160;
const int BUTTON_HEIGHT = 50;

// 音频文件路径
const QString GALLERY_BGM_PATH = "resources/mako.mp3";
const QString CG_SOUND_PATH = "resources/CG.mp3";
const QString HSCENE_SOUND_PATH = "resources/hscene.mp3";
const QString STAND_SOUND_PATH = "resources/looklook.mp3";
const QString MUSIC_SOUND_PATH = "resources/music.mp3";

GalleryWindow::GalleryWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("GalEngine - Gallery");
    setFixedSize(1280, 720);

    // 创建音频管理器
    m_audioManager = new AudioManager(this);

    // 预加载图片资源
    if (!BACKGROUND_IMAGE_PATH.isEmpty()) {
        ResourceManager::instance().preloadImage(BACKGROUND_IMAGE_PATH);
    }
    ResourceManager::instance().preloadImage(LOGO_IMAGE_PATH);

    // 注册音频资源
    ResourceManager::instance().registerAudio(GALLERY_BGM_PATH);
    ResourceManager::instance().registerAudio(CG_SOUND_PATH);
    ResourceManager::instance().registerAudio(HSCENE_SOUND_PATH);
    ResourceManager::instance().registerAudio(STAND_SOUND_PATH);
    ResourceManager::instance().registerAudio(MUSIC_SOUND_PATH);

    // 播放背景音乐
    if (ResourceManager::instance().hasAudio(GALLERY_BGM_PATH)) {
        m_audioManager->playBgm(GALLERY_BGM_PATH);
    }
    else {
        qDebug() << "Gallery BGM file not found:" << GALLERY_BGM_PATH;
    }

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

    auto* mainLayout = new QVBoxLayout(this);

    auto* buttonLayout = new QHBoxLayout();
    cgBtn = new QPushButton("CG", this);
    hsceneBtn = new QPushButton("Hscene", this);
    standBtn = new QPushButton("Stands", this);
    musicBtn = new QPushButton("Music", this);
    returnBtn = new QPushButton("Return", this);

    QList<QPushButton*> buttons = { cgBtn, hsceneBtn, standBtn, musicBtn, returnBtn };
    for (auto* btn : buttons) {
        btn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
        buttonLayout->addWidget(btn);
    }
    mainLayout->addLayout(buttonLayout);

    auto* centerLayout = new QHBoxLayout();
    prevBtn = new QPushButton("<", this);
    nextBtn = new QPushButton(">", this);
    prevBtn->setFixedSize(60, 60);
    nextBtn->setFixedSize(60, 60);

    displayLabel = new QLabel(this);
    displayLabel->setFixedSize(800, 450);
    displayLabel->setStyleSheet("background-color: rgba(30,30,30,180); border:2px solid gray;");
    displayLabel->setAlignment(Qt::AlignCenter);

    centerLayout->addWidget(prevBtn);
    centerLayout->addWidget(displayLabel, 1);
    centerLayout->addWidget(nextBtn);
    mainLayout->addLayout(centerLayout);

    // ---------- 底部缩略图区域 ----------
    previewArea = new QScrollArea(this);
    previewWidget = new QWidget();
    previewLayout = new QHBoxLayout(previewWidget);
    previewWidget->setLayout(previewLayout);
    previewArea->setWidget(previewWidget);
    previewArea->setWidgetResizable(true);
    previewArea->setFixedHeight(120);
    mainLayout->addWidget(previewArea);

    setLayout(mainLayout);

    connect(cgBtn, &QPushButton::clicked, this, &GalleryWindow::onCgGame);
    connect(hsceneBtn, &QPushButton::clicked, this, &GalleryWindow::onHsceneGame);
    connect(standBtn, &QPushButton::clicked, this, &GalleryWindow::onStandGame);
    connect(musicBtn, &QPushButton::clicked, this, &GalleryWindow::onMusicGame);
    connect(returnBtn, &QPushButton::clicked, this, &GalleryWindow::onReturnGame);
    connect(prevBtn, &QPushButton::clicked, this, &GalleryWindow::showPrevImage);
    connect(nextBtn, &QPushButton::clicked, this, &GalleryWindow::showNextImage);

    // 从 ResourceManager 获取 logo 图片
    logoPixmap = ResourceManager::instance().getPixmap(LOGO_IMAGE_PATH);
    if (logoPixmap.isNull()) {
        // 如果从ResourceManager获取失败，尝试直接加载
        logoPixmap.load(LOGO_IMAGE_PATH);
    }
}

GalleryWindow::~GalleryWindow()
{
    if (m_startWindow)
        delete m_startWindow;
}

// ====== 绘制背景 & Logo ======
void GalleryWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 从 ResourceManager 获取背景图片
    QPixmap background;
    if (!BACKGROUND_IMAGE_PATH.isEmpty()) {
        background = ResourceManager::instance().getPixmap(BACKGROUND_IMAGE_PATH);
        if (background.isNull()) {
            // 如果从ResourceManager获取失败，尝试直接加载
            background.load(BACKGROUND_IMAGE_PATH);
        }
    }

    if (!background.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), background);
    }
    else {
        painter.fillRect(rect(), QColor(50, 50, 50));
    }

    if (!logoPixmap.isNull()) {
        int logoWidth = logoPixmap.width() * LOGO_SCALE_FACTOR;
        int logoHeight = logoPixmap.height() * LOGO_SCALE_FACTOR;
        int x = 10;
        int y = height() - logoHeight - 10;
        painter.drawPixmap(x, y, logoWidth, logoHeight, logoPixmap);
    }
}

// ====== 按钮逻辑 ======
void GalleryWindow::onCgGame()
{
    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(CG_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(CG_SOUND_PATH, [this]() {
            loadImages("assets/bg");
        });
    }
    else {
        qDebug() << "CG sound file not found:" << CG_SOUND_PATH;
        // 即使没有音效也继续执行
        loadImages("assets/bg");
    }
}

void GalleryWindow::onHsceneGame()
{
    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(HSCENE_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(HSCENE_SOUND_PATH, [this]() {
            clearImages();
        });
    }
    else {
        qDebug() << "Hscene sound file not found:" << HSCENE_SOUND_PATH;
        // 即使没有音效也继续执行
        clearImages();
    }
}

void GalleryWindow::onStandGame()
{
    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(STAND_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(STAND_SOUND_PATH, [this]() {
            loadImages("assets/ch");
        });
    }
    else {
        qDebug() << "Stand sound file not found:" << STAND_SOUND_PATH;
        // 即使没有音效也继续执行
        loadImages("assets/ch");
    }
}

void GalleryWindow::onReturnGame()
{
    m_audioManager->stopBgm();
    if (!m_startWindow)
        m_startWindow = new StartWindow();
    m_startWindow->show();
    this->close();
}

void GalleryWindow::loadImages(const QString& folder)
{
    clearImages();

    QDir dir(folder);
    QStringList filters = { "*.png", "*.jpg", "*.jpeg" };
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    for (auto& file : files) {
        imageList.append(file.absoluteFilePath());
    }
    currentIndex = 0;
    updateDisplay();
    updatePreview();
}

void GalleryWindow::clearImages()
{
    imageList.clear();
    currentIndex = -1;
    displayLabel->clear();

    QLayoutItem* child;
    while ((child = previewLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
}

void GalleryWindow::updateDisplay()
{
    if (currentIndex >= 0 && currentIndex < imageList.size()) {
        QPixmap pix(imageList[currentIndex]);
        if (!pix.isNull()) {
            pix = pix.scaled(displayLabel->size() * g_scaler, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            displayLabel->setPixmap(pix);
        }
    }
}

void GalleryWindow::updatePreview()
{
    for (int i = 0; i < imageList.size(); ++i) {
        QPixmap pix(imageList[i]);
        auto* thumb = new ClickableLabel(i);
        thumb->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        thumb->setStyleSheet("border:2px solid gray;");
        previewLayout->addWidget(thumb);

        connect(thumb, &ClickableLabel::clicked, this, &GalleryWindow::onThumbnailClicked);
    }
}

void GalleryWindow::onThumbnailClicked(int index)
{
    if (index >= 0 && index < imageList.size()) {
        currentIndex = index;
        updateDisplay();
    }
}

void GalleryWindow::showPrevImage()
{
    if (currentIndex > 0) {
        currentIndex--;
        updateDisplay();
    }
}

void GalleryWindow::showNextImage()
{
    if (currentIndex < imageList.size() - 1) {
        currentIndex++;
        updateDisplay();
    }
}

void GalleryWindow::onMusicGame()
{
    // 使用 ResourceManager 检查音频是否存在
    if (ResourceManager::instance().hasAudio(MUSIC_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(MUSIC_SOUND_PATH, [this]() {
            loadMusic("assets/bgm");
        });
    }
    else {
        qDebug() << "Music sound file not found:" << MUSIC_SOUND_PATH;
        // 即使没有音效也继续执行
        loadMusic("assets/bgm");
    }
}

void GalleryWindow::loadMusic(const QString& folder)
{
    clearImages();

    QDir dir(folder);
    QStringList filters = { "*.mp3", "*.wav", "*.ogg", "*.m4a" };
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    for (int i = 0; i < files.size(); ++i) {
        QString path = files[i].absoluteFilePath();
        QString name = files[i].baseName();

        QPushButton* musicItem = new QPushButton(name, this);
        musicItem->setFixedSize(150, 40);
        previewLayout->addWidget(musicItem);

        connect(musicItem, &QPushButton::clicked, this, [this, path]() {
            m_audioManager->stopBgm();
            m_audioManager->playBgm(path);
            displayLabel->setText("Now Playing:\n" + path);
        });
    }
}
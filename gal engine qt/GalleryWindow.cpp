#include "GalleryWindow.h"
#include "StartWindow.h"
#include "ClickableLabel.h"
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

GalleryWindow::GalleryWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("GalEngine - Gallery");
    setFixedSize(1280, 720);

    mmm_bgm = new QMediaPlayer(this);
    mmm_bgmOut = new QAudioOutput(this);
    mmm_bgm->setAudioOutput(mmm_bgmOut);
    mmm_bgmOut->setVolume(0.5f);

    mmm_se = new QMediaPlayer(this);
    mmm_seOut = new QAudioOutput(this);
    mmm_se->setAudioOutput(mmm_seOut);
    mmm_seOut->setVolume(1.0f);

    mmm_bgm->setSource(QUrl::fromLocalFile("resources/mako.mp3"));
    mmm_bgm->setLoops(QMediaPlayer::Infinite);
    mmm_bgm->play();

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

    logoPixmap.load(LOGO_IMAGE_PATH);
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

    QPixmap background(BACKGROUND_IMAGE_PATH);
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
    mmm_se->setSource(QUrl::fromLocalFile("resources/CG.mp3"));
    mmm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            loadImages("assets/bg");

            // 断开连接
            disconnect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mmm_se->play();
}

void GalleryWindow::onHsceneGame()
{
    mmm_se->setSource(QUrl::fromLocalFile("resources/hscene.mp3"));
    mmm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            clearImages();

            // 断开连接
            disconnect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mmm_se->play();
}

void GalleryWindow::onStandGame()
{
    mmm_se->setSource(QUrl::fromLocalFile("resources/looklook.mp3"));
    mmm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            loadImages("assets/ch");

            // 断开连接
            disconnect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mmm_se->play();
}

void GalleryWindow::onReturnGame()
{
    mmm_bgm->stop();
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
    mmm_se->setSource(QUrl::fromLocalFile("resources/music.mp3"));
    mmm_se->setLoops(1);

    connect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            loadMusic("assets/bgm");
            disconnect(mmm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mmm_se->play();
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
            mmm_bgm->stop();
            mmm_bgm->setSource(QUrl::fromLocalFile(path));
            mmm_bgm->setLoops(QMediaPlayer::Infinite);
            mmm_bgm->play();

            displayLabel->setText("Now Playing:\n" + path);
        });
    }
}
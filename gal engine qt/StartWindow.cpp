#include "StartWindow.h"
#include "SettingWindow.h"
#include "GalleryWindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QApplication>
#include <qpushbutton.h>
#include <QString>
#include <QPainter>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "MainWindow.h"

// 配置变量
const QString BACKGROUND_IMAGE_PATH = "resources/background.jpg"; // 底图文件路径
const QString LOGO_IMAGE_PATH = "resources/logo.png"; // logo文件路径
const double LOGO_SCALE_FACTOR = 0.3; // logo缩放因子
const int BUTTON_WIDTH = 240; // 按钮宽度
const int BUTTON_HEIGHT = 60; // 按钮高度
const int RIGHT_MARGIN = 20; // 右侧边距
const int BOTTOM_MARGIN = 0; // 底部边距

StartWindow::StartWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("GalEngine - Start");
    setFixedSize(1280, 720);

    mm_bgm = new QMediaPlayer(this);
    mm_bgmOut = new QAudioOutput(this);
    mm_bgm->setAudioOutput(mm_bgmOut);
    mm_bgmOut->setVolume(0.5f);

    mm_se = new QMediaPlayer(this);
    mm_seOut = new QAudioOutput(this);
    mm_se->setAudioOutput(mm_seOut);
    mm_seOut->setVolume(1.0f);

    mm_bgm->setSource(QUrl::fromLocalFile("resources/little explorer.mp3"));
    mm_bgm->setLoops(QMediaPlayer::Infinite);
    mm_bgm->play();

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

    // 加载logo图片
    logoPixmap.load(LOGO_IMAGE_PATH);
    if (logoPixmap.isNull()) {
        qDebug() << "Failed to load logo image:" << LOGO_IMAGE_PATH;
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

    // 绘制背景图
    QPixmap background(BACKGROUND_IMAGE_PATH);
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
    mm_bgm->stop();
    mm_se->setSource(QUrl::fromLocalFile("resources/start.mp3"));
    mm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            if (!m_mainWindow) {
                m_mainWindow = new MainWindow();
            }
            m_mainWindow->show();
            this->close();

            // 断开连接
            disconnect(mm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mm_se->play();
}

void StartWindow::onContinueGame()
{
    mm_bgm->stop();
    mm_se->setSource(QUrl::fromLocalFile("resources/Continue.mp3"));
    mm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            if (!m_mainWindow)
                m_mainWindow = new MainWindow();
            m_mainWindow->loadGame();
            m_mainWindow->show();
            this->close();

            // 断开连接
            disconnect(mm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mm_se->play();
}

void StartWindow::onGalleryGame()
{
    mm_bgm->stop();
    mm_se->setSource(QUrl::fromLocalFile("resources/gallery.mp3"));
    mm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            if (!m_galleryWindow)
                m_galleryWindow = new GalleryWindow();
            m_galleryWindow->show();
            this->close();

            // 断开连接
            disconnect(mm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mm_se->play();
}

void StartWindow::onSettingGame()
{
    mm_bgm->stop();
    mm_se->setSource(QUrl::fromLocalFile("resources/setting.mp3"));
    mm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            if (!m_settingWindow)
                m_settingWindow = new SettingWindow();
            m_settingWindow->show();
            this->close();

            // 断开连接
            disconnect(mm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mm_se->play();
}

void StartWindow::onExitGame()
{
    mm_bgm->stop();
    mm_se->setSource(QUrl::fromLocalFile("resources/exit.mp3"));
    mm_se->setLoops(1);

    // 连接媒体播放结束的信号
    connect(mm_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            QApplication::quit();

            // 断开连接
            disconnect(mm_se, &QMediaPlayer::mediaStatusChanged, this, nullptr);
        }
    });

    mm_se->play();
}
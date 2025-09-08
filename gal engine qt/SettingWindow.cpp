#include "SettingWindow.h"
#include "StartWindow.h"
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
#include <QUrl>
#include <QDebug>


const QString BACKGROUND_IMAGE_PATH = "resources/background.png";
const QString LOGO_IMAGE_PATH = "resources/logo.png";
const double LOGO_SCALE_FACTOR = 0.3; // logo缩放因子
const int BUTTON_WIDTH = 240; // 按钮宽度
const int BUTTON_HEIGHT = 60; // 按钮高度
const int RIGHT_MARGIN = 20; // 右侧边距
const int BOTTOM_MARGIN = 0; // 底部边距


const QString SETTING_BGM_PATH = "resources/Perple Moon.mp3";
const QString RETURN_SOUND_PATH = "";

SettingWindow::SettingWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("GalEngine - Setting");
    setFixedSize(1280, 720);

    m_audioManager = new AudioManager(this);

    ResourceManager::instance().preloadImage(BACKGROUND_IMAGE_PATH);
    ResourceManager::instance().preloadImage(LOGO_IMAGE_PATH);

    ResourceManager::instance().registerAudio(SETTING_BGM_PATH);
    ResourceManager::instance().registerAudio(RETURN_SOUND_PATH);

    if (ResourceManager::instance().hasAudio(SETTING_BGM_PATH)) {
        m_audioManager->playBgm(SETTING_BGM_PATH);
    }
    else {
        qDebug() << "Setting BGM file not found:" << SETTING_BGM_PATH;
    }

    setStyleSheet(R"(
    QPushButton {
        background-color: rgba(125, 105, 150, 200); /* 亮粉色 */
        color: white;
        border: 2px solid #FF69B4;
        border-radius: 10px;
        font-size: 18px;
        font-weight: bold;
        padding: 10px;
        box-shadow: 0 0 10px rgba(125, 105, 150, 100);
    }
    QPushButton:hover {
        background-color: rgba(125, 130, 170, 230);
        border: 2px solid #FF82C8;
        border-radius: 14px;
        box-shadow: 0 0 15px rgba(125, 105, 150, 150);
    }
    QPushButton:pressed {
        background-color: rgba(120, 80, 125, 255);
        border: 2px solid #DC509B;
        box-shadow: 0 0 5px rgba(125, 105, 150, 100);
    }
    )");

    auto* mainLayout = new QHBoxLayout(this);

    auto* leftSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainLayout->addItem(leftSpacer);

    auto* rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->setSpacing(20);

    returnBtn = new QPushButton(QString::fromLocal8Bit("return"), this);

    returnBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);

    rightLayout->addWidget(returnBtn);

    rightLayout->insertStretch(0, 1);
    rightLayout->addStretch(1);

    mainLayout->addLayout(rightLayout);
    mainLayout->setAlignment(rightLayout, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->setContentsMargins(0, 0, RIGHT_MARGIN, 0);

    connect(returnBtn, &QPushButton::clicked, this, &SettingWindow::onReturnGame);

    logoPixmap = ResourceManager::instance().getPixmap(LOGO_IMAGE_PATH);
    if (logoPixmap.isNull()) {
        qDebug() << "Failed to load logo image:" << LOGO_IMAGE_PATH;
        logoPixmap.load(LOGO_IMAGE_PATH);
    }
}

SettingWindow::~SettingWindow()
{
    if (m_startWindow)
        delete m_startWindow;
}

void SettingWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QPixmap background = ResourceManager::instance().getPixmap(BACKGROUND_IMAGE_PATH);
    if (background.isNull()) {
        background.load(BACKGROUND_IMAGE_PATH);
    }

    if (!background.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), background);
    }
    else {
        painter.fillRect(rect(), QColor(50, 50, 50));
        qDebug() << "Failed to load background image:" << BACKGROUND_IMAGE_PATH;
    }

    if (!logoPixmap.isNull()) {
        int logoWidth = logoPixmap.width() * LOGO_SCALE_FACTOR;
        int logoHeight = logoPixmap.height() * LOGO_SCALE_FACTOR;

        int x = BOTTOM_MARGIN - 30;
        int y = height() - logoHeight - BOTTOM_MARGIN + 35;

        painter.drawPixmap(x, y, logoWidth, logoHeight, logoPixmap);
    }
}

void SettingWindow::onReturnGame()
{
    m_audioManager->stopBgm();

    if (ResourceManager::instance().hasAudio(RETURN_SOUND_PATH)) {
        m_audioManager->playSeWithCallback(RETURN_SOUND_PATH, [this]() {
            if (!m_startWindow)
                m_startWindow = new StartWindow();
            m_startWindow->show();
            this->close();
        });
    }
    else {
        qDebug() << "Return sound file not found:" << RETURN_SOUND_PATH;
        if (!m_startWindow)
            m_startWindow = new StartWindow();
        m_startWindow->show();
        this->close();
    }
}
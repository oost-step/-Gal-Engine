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

// ���ñ���
const QString BACKGROUND_IMAGE_PATH = "resources/background.jpg"; // ��ͼ�ļ�·��
const QString LOGO_IMAGE_PATH = "resources/logo.png"; // logo�ļ�·��
const double LOGO_SCALE_FACTOR = 0.3; // logo��������
const int BUTTON_WIDTH = 240; // ��ť���
const int BUTTON_HEIGHT = 60; // ��ť�߶�
const int RIGHT_MARGIN = 20; // �Ҳ�߾�
const int BOTTOM_MARGIN = 0; // �ײ��߾�

// ��Ƶ�ļ�·��
const QString SETTING_BGM_PATH = "resources/Perple Moon.mp3";
const QString RETURN_SOUND_PATH = "";

SettingWindow::SettingWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("GalEngine - Setting");
    setFixedSize(1280, 720);

    // ������Ƶ������
    m_audioManager = new AudioManager(this);

    // Ԥ����ͼƬ��Դ
    ResourceManager::instance().preloadImage(BACKGROUND_IMAGE_PATH);
    ResourceManager::instance().preloadImage(LOGO_IMAGE_PATH);

    // ע����Ƶ��Դ
    ResourceManager::instance().registerAudio(SETTING_BGM_PATH);
    ResourceManager::instance().registerAudio(RETURN_SOUND_PATH);

    // ���ű�������
    if (ResourceManager::instance().hasAudio(SETTING_BGM_PATH)) {
        m_audioManager->playBgm(SETTING_BGM_PATH);
    }
    else {
        qDebug() << "Setting BGM file not found:" << SETTING_BGM_PATH;
    }

    // ������ʽ�����ڰ�ť��ͣЧ��
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

    // ʹ��ˮƽ���ֽ���ť�������Ҳ�
    auto* mainLayout = new QHBoxLayout(this);

    // ���ռλ�ռ�
    auto* leftSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainLayout->addItem(leftSpacer);

    // �Ҳఴť����
    auto* rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->setSpacing(20);

    returnBtn = new QPushButton(QString::fromLocal8Bit("return"), this);

    // ���ð�ť��С
    returnBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);

    // ��ӵ��Ҳ಼��
    rightLayout->addWidget(returnBtn);

    // �ڰ�ť�Ϸ���ӵ��Կռ䣬ʹ��ť��ֱ����
    rightLayout->insertStretch(0, 1);
    rightLayout->addStretch(1);

    // ���Ҳ಼����ӵ�������
    mainLayout->addLayout(rightLayout);
    mainLayout->setAlignment(rightLayout, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->setContentsMargins(0, 0, RIGHT_MARGIN, 0);

    connect(returnBtn, &QPushButton::clicked, this, &SettingWindow::onReturnGame);

    // �� ResourceManager ��ȡ logo ͼƬ
    logoPixmap = ResourceManager::instance().getPixmap(LOGO_IMAGE_PATH);
    if (logoPixmap.isNull()) {
        qDebug() << "Failed to load logo image:" << LOGO_IMAGE_PATH;
        // ����ֱ�Ӽ���
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

    // �� ResourceManager ��ȡ����ͼƬ
    QPixmap background = ResourceManager::instance().getPixmap(BACKGROUND_IMAGE_PATH);
    if (background.isNull()) {
        // �����ResourceManager��ȡʧ�ܣ�����ֱ�Ӽ���
        background.load(BACKGROUND_IMAGE_PATH);
    }

    if (!background.isNull()) {
        // ���ű���ͼ�������������
        painter.drawPixmap(0, 0, width(), height(), background);
    }
    else {
        // �������ͼ����ʧ�ܣ�ʹ��Ĭ�ϱ���ɫ
        painter.fillRect(rect(), QColor(50, 50, 50));
        qDebug() << "Failed to load background image:" << BACKGROUND_IMAGE_PATH;
    }

    // ����logo�����½�
    if (!logoPixmap.isNull()) {
        // �����������Ӽ���logo��С
        int logoWidth = logoPixmap.width() * LOGO_SCALE_FACTOR;
        int logoHeight = logoPixmap.height() * LOGO_SCALE_FACTOR;

        // �������½�λ�ã����Ǳ߾ࣩ
        int x = BOTTOM_MARGIN - 30;
        int y = height() - logoHeight - BOTTOM_MARGIN + 35;

        // ����logo
        painter.drawPixmap(x, y, logoWidth, logoHeight, logoPixmap);
    }
}

void SettingWindow::onReturnGame()
{
    m_audioManager->stopBgm();

    // ʹ�� ResourceManager �����Ƶ�Ƿ����
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
        // ��ʹû����ЧҲ����ִ��
        if (!m_startWindow)
            m_startWindow = new StartWindow();
        m_startWindow->show();
        this->close();
    }
}
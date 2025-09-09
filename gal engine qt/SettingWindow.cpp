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
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>

bool g_autoMode = false;
bool g_skipMode = false;

const QString BACKGROUND_IMAGE_PATH = "resources/background.png";
const QString LOGO_IMAGE_PATH = "resources/logo.png";
const double LOGO_SCALE_FACTOR = 0.3; // logo缩放因子
const int BUTTON_WIDTH = 240; // 按钮宽度
const int BUTTON_HEIGHT = 60; // 按钮高度
const int RIGHT_MARGIN = 20; // 右侧边距
const int BOTTOM_MARGIN = 0; // 底部边距


const QString SETTING_BGM_PATH = "resources/Perple Moon.mp3";
const QString RETURN_SOUND_PATH = "";

SettingWindow::SettingWindow(QWidget* parent, bool fromMain, QWidget* caller)
    : QWidget(parent), m_fromMainWindow(fromMain), m_caller(caller)
{
    setWindowTitle("GalEngine - Setting");
    setFixedSize(1280, 720);

    m_audioManager = new AudioManager(this);

    if (!m_fromMainWindow && m_caller.isNull()) {
        if (ResourceManager::instance().hasAudio(SETTING_BGM_PATH)) {
            m_audioManager->playBgm(SETTING_BGM_PATH);
        }
        else {
            qDebug() << "Setting BGM file not found:" << SETTING_BGM_PATH;
        }
    }

    ResourceManager::instance().preloadImage(BACKGROUND_IMAGE_PATH);
    ResourceManager::instance().preloadImage(LOGO_IMAGE_PATH);

    ResourceManager::instance().registerAudio(RETURN_SOUND_PATH);

    if (ResourceManager::instance().hasAudio(SETTING_BGM_PATH)) {
        m_audioManager->playBgm(SETTING_BGM_PATH);
    }
    else {
        qDebug() << "Setting BGM file not found:" << SETTING_BGM_PATH;
    }

    setStyleSheet(R"(
        QRadioButton {
            color: white;
            font-size: 18px;
            font-weight: bold;
            spacing: 10px;
        }
        QRadioButton::indicator {
            width: 20px;
            height: 20px;
        }
        QRadioButton::indicator:unchecked {
            border: 2px solid #FF69B4;
            border-radius: 10px;
            background: transparent;
        }
        QRadioButton::indicator:checked {
            border: 2px solid #FF69B4;
            border-radius: 10px;
            background-color: rgba(255,105,180,180);
        }
    )");

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    auto* rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->setSpacing(20);

    QRadioButton* normalModeRadio = new QRadioButton(QString::fromLocal8Bit("正常模式"), this);
    QRadioButton* autoModeRadio = new QRadioButton(QString::fromLocal8Bit("自动模式"), this);
    QRadioButton* skipModeRadio = new QRadioButton(QString::fromLocal8Bit("快进模式"), this);

    QButtonGroup* modeGroup = new QButtonGroup(this);
    modeGroup->addButton(normalModeRadio, 0);
    modeGroup->addButton(autoModeRadio, 1);
    modeGroup->addButton(skipModeRadio, 2);

    if (g_autoMode) autoModeRadio->setChecked(true);
    else if (g_skipMode) skipModeRadio->setChecked(true);
    else normalModeRadio->setChecked(true);

    connect(normalModeRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            g_autoMode = false; g_skipMode = false;
            emit modesChanged(g_autoMode, g_skipMode);
        }
    });
    connect(autoModeRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            g_autoMode = true; g_skipMode = false;
            emit modesChanged(g_autoMode, g_skipMode);
        }
    });
    connect(skipModeRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            g_autoMode = false; g_skipMode = true;
            emit modesChanged(g_autoMode, g_skipMode);
        }
    });

    rightLayout->addWidget(normalModeRadio);
    rightLayout->addWidget(autoModeRadio);
    rightLayout->addWidget(skipModeRadio);

    returnBtn = new QPushButton(QString::fromLocal8Bit("返回游戏"), this);
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

SettingWindow::~SettingWindow(){}

void SettingWindow::setCaller(QWidget* caller)
{
    m_caller = caller;
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

    if (m_fromMainWindow) {
        // 通知 MainWindow：它会自己 restore 并 show()
        emit closedFromMainWindow();
        this->close();
        return;
    }
    else {
        emit closedFromStartWindow();
        this->close();
        return;
    }


    // 如果 caller 有效 -> 恢复 caller（通常是 StartWindow）
    if (!m_caller.isNull()) {
        QWidget* w = m_caller.data();
        if (w) {
            w->show();
            this->close();
            return;
        }
    }

    // 兜底：尝试找到已有的 StartWindow 并显示（避免重复 new）
    bool found = false;
    for (QWidget* top : qApp->topLevelWidgets()) {
        if (auto* sw = qobject_cast<StartWindow*>(top)) {
            sw->show();
            found = true;
            break;
        }
    }
    if (!found) {
        // 如果确实没有 StartWindow，可以创建（但一般不要走到这里——推荐始终传 caller）
        StartWindow* sw = new StartWindow();
        sw->show();
    }
    this->close();
}
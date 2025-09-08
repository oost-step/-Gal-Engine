#include "SaveLoadWindow.h"
#include "ScriptEngine.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QPixmap>

SaveLoadWindow::SaveLoadWindow(ScriptEngine* engine, Mode mode, QWidget* parent)
    : QDialog(parent), m_engine(engine), m_mode(mode), m_currentPage(0) {

    setWindowTitle(mode == SaveMode ? "Save Game" : "Load Game");
    resize(800, 600);


    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 槽位区域
    QGridLayout* grid = new QGridLayout();
    for (int i = 0; i < m_slotsPerPage; i++) {
        QPushButton* btn = new QPushButton(this);
        connect(btn, &QPushButton::clicked, this, [this, i]() { onSlotClicked(i); });
        grid->addWidget(btn, i / 3, i % 3);
        m_slotButtons.append(btn);
    }
    mainLayout->addLayout(grid);

    // 翻页控制
    QHBoxLayout* navLayout = new QHBoxLayout();
    m_prevBtn = new QPushButton("<< Prev", this);
    m_nextBtn = new QPushButton("Next >>", this);
    m_pageLabel = new QLabel(this);

    connect(m_prevBtn, &QPushButton::clicked, this, &SaveLoadWindow::onPrevPage);
    connect(m_nextBtn, &QPushButton::clicked, this, &SaveLoadWindow::onNextPage);

    navLayout->addWidget(m_prevBtn);
    navLayout->addStretch();
    navLayout->addWidget(m_pageLabel);
    navLayout->addStretch();
    navLayout->addWidget(m_nextBtn);

    mainLayout->addLayout(navLayout);

    setLayout(mainLayout);

    loadSlots();
    refreshUi();
}

void SaveLoadWindow::loadSlots() {
    m_slots.clear();
    for (int i = 0; i < m_totalSlots; i++) {
        QString filename = QString("saves/save_slot_%1.json").arg(i);
        SaveSlotInfo info;
        info.file = filename;

        if (QFile::exists(filename)) {
            QFile f(filename);
            if (f.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    info.desc = obj.value("desc").toString();
                    info.time = obj.value("time").toString();
                    info.screenshot = obj.value("screenshot").toString();
                }
            }
        }
        m_slots.append(info);
    }
}

void SaveLoadWindow::refreshUi() {
    int start = m_currentPage * m_slotsPerPage;
    for (int i = 0; i < m_slotsPerPage; i++) {
        int idx = start + i;
        QPushButton* btn = m_slotButtons[i];
        if (idx >= m_slots.size()) {
            btn->setStyleSheet("QPushButton { "
                "font-size: 7pt; "
                "}");
            btn->setText("Empty");
            btn->setIcon(QIcon());
            continue;
        }
        SaveSlotInfo info = m_slots[idx];
        if (QFile::exists(info.file)) {
            QString label = QString("Slot %1\n%2\n%3")
                .arg(idx)
                .arg(info.time.isEmpty() ? "(no time)" : info.time)
                .arg(info.desc.isEmpty() ? "(no desc)" : info.desc);
            btn->setStyleSheet("QPushButton { "
                "font-size: 7pt; "
                "}");
            btn->setText(label);

            if (!info.screenshot.isEmpty() && QFile::exists(info.screenshot)) {
                QPixmap px(info.screenshot);
                btn->setIcon(QIcon(px));
                btn->setIconSize(QSize(200, 100));
            }
        }
        else {
            btn->setStyleSheet("QPushButton { "
                "font-size: 7pt; "
                "}");
            btn->setText(QString("Slot %1\n(Empty)").arg(idx));
            btn->setIcon(QIcon());
        }
    }

    m_pageLabel->setText(QString("Page %1 / %2")
        .arg(m_currentPage + 1)
        .arg((m_totalSlots + m_slotsPerPage - 1) / m_slotsPerPage));
}

void SaveLoadWindow::onSlotClicked(int index) {
    int slotIndex = m_currentPage * m_slotsPerPage + index;
    if (slotIndex >= m_slots.size()) return;

    if (m_mode == SaveMode) {
        emit saveToSlot(slotIndex);
        loadSlots();
        refreshUi();
    }
    else {
        if (!QFile::exists(m_slots[slotIndex].file)) {
            // 创建自定义字体的消息框
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Load");
            msgBox.setText("Empty slot.");
            msgBox.setIcon(QMessageBox::Information);

            // 设置消息框字体
            QFont msgFont("Simhei");
            msgFont.setPointSize(7);
            msgBox.setFont(msgFont);

            msgBox.exec();
            return;
        }
        emit loadFromSlot(slotIndex);
        accept();
    }
}

void SaveLoadWindow::onPrevPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        refreshUi();
    }
}

void SaveLoadWindow::onNextPage() {
    if ((m_currentPage + 1) * m_slotsPerPage < m_slots.size()) {
        m_currentPage++;
        refreshUi();
    }
}

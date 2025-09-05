#pragma once

#include <QDialog>
#include <QVector>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QDateTime>

class ScriptEngine;

struct SaveSlotInfo {
    QString file;        // JSON �浵�ļ�
    QString screenshot;  // ����ͼ·��
    QString desc;        // �浵���
    QString time;        // �浵ʱ��
};

class SaveLoadWindow : public QDialog {
    Q_OBJECT
public:
    enum Mode { SaveMode, LoadMode };

    SaveLoadWindow(ScriptEngine* engine, Mode mode, QWidget* parent = nullptr);

signals:
    void saveToSlot(int slotIndex);
    void loadFromSlot(int slotIndex);

private slots:
    void onSlotClicked(int index);
    void onPrevPage();
    void onNextPage();

private:
    void loadSlots();
    void refreshUi();

    ScriptEngine* m_engine;
    Mode m_mode;

    QVector<SaveSlotInfo> m_slots;
    QVector<QPushButton*> m_slotButtons;

    int m_currentPage;
    const int m_slotsPerPage = 9;
    const int m_totalSlots = 36; // 4 ҳ��ÿҳ 9 ��

    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QLabel* m_pageLabel;
};

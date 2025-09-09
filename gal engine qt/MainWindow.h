#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include "ImageLayer.h"
#include "DialogueBox.h"
#include "ChoiceOverlay.h"
#include "ScriptEngine.h"
#include "AudioManager.h"

class StartWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void loadGame(); 
    void startWindowContinue();// allow external call (from StartWindow -> Continue)

    QTimer* m_modeTimer = nullptr;
    QTimer* m_autoDelayTimer = nullptr;

protected:
    void resizeEvent(QResizeEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onBackgroundChanged(const QString& path);
    void onSpriteChanged(const QString& slot, const QString& path);
    void onSpriteChangedTop(const QString& slot, const QString& path);
    void onSpriteCleared(const QString& slot);
    void onSpriteClearedTop(const QString& slot);
    void onTextReady(const QString& speaker, const QString& text);
    void onChoiceRequested(const QString& prompt, const QStringList& options);
    void onPreloadRequested(const QStringList& images, const QStringList& audios);
    void onAutosavePoint(const QString& name);

    void saveGame();
    void loadGameFromDialog();

    void onReturnClicked();

    void onShakeWindow(int amplitude = 5, int duration = 500, int shakeCount = 5);

    void onClose();

signals:
    void playSound(const QString& path);

private:
    QLabel* m_bg;
    ImageLayer* m_layer;
    ImageLayerTop* m_layerT;
    DialogueBox* m_dialogue;
    ChoiceOverlay* m_choices;
    ScriptEngine* m_engine;
    AudioManager* m_audio;

    StartWindow* m_startWindow = nullptr;

    QStringList m_history;
    QString m_currentText;

    QPixmap m_bgPixmap;

    QAbstractAnimation* m_shakeAnimation = nullptr;

    QToolBar* bottomToolBar = nullptr;

    QTimer* m_ctrlLongPressTimer = nullptr;

    QVariantMap m_tempSnapshot;

    int m_autoWaitCount; // 自动模式等待计数器
    bool m_skipAllMode;  // 真正的快进模式（跳过所有已读内容）

    bool m_pausedBySetting = false;     // 打开 Setting 时暂停推进的标记

    void layoutUi();
    void showHistory();
    void saveToSlot(int slotIndex);
    void loadFromSlot(int slotIndex);
    void handleAdvance();
    void handleAdvanceOrSkip();
    void mousePressEvent(QMouseEvent* ev);
    void keyReleaseEvent(QKeyEvent* ev);
    void enableSkipAllMode(bool enable);
};

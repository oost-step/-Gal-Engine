#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
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
    void loadGame(); 
    void startWindowContinue();// allow external call (from StartWindow -> Continue)

protected:
    void resizeEvent(QResizeEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;

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

    void layoutUi();
    void showHistory();
    void saveToSlot(int slotIndex);
    void loadFromSlot(int slotIndex);
};

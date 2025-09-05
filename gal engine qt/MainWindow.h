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
    void loadGame(); // allow external call (from StartWindow -> Continue)

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

private:
    QLabel* m_bg;
    ImageLayer* m_layer;
    ImageLayerTop* m_layerT;
    DialogueBox* m_dialogue;
    ChoiceOverlay* m_choices;
    ScriptEngine* m_engine;
    AudioManager* m_audio;

    QPushButton* m_returnBtn = nullptr;
    StartWindow* m_startWindow = nullptr;

    QPixmap m_bgPixmap;

    void layoutUi();
};

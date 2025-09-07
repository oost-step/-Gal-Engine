#pragma once
#include <QObject>
#include <QVariantMap>
#include <QStack>
#include <QPair>
#include <QVariant>
#include "SceneTypes.h"

class StartWindow;

class ScriptEngine : public QObject {
    Q_OBJECT
public:
    explicit ScriptEngine(QObject* parent = nullptr);
    bool loadFromJsonFile(const QString& path);
    bool loadFromJsonObject(const QJsonObject& root);
    bool parse(const QJsonObject& root);

    void start(const QString& sceneId = QString());

    QVariantMap snapshot() const;
    void restore(const QVariantMap& m);

signals:
    void backgroundChanged(const QString& path);
    void spriteChanged(const QString& slot, const QString& path);
    void spriteChangedTop(const QString& slot, const QString& path);
    void spriteCleared(const QString& slot);
    void spriteClearedTop(const QString& slot);
    void textReady(const QString& speaker, const QString& text);
    void playBgm(const QString& path);
    void stopBgm();
    void playSe(const QString& path);
    void waitRequested(int ms);
    void choiceRequested(const QString& prompt, const QStringList& options);
    void sceneEntered(const QString& sceneId);
    void scriptEnded();
    void preloadRequested(const QStringList& imagePaths, const QStringList& audioPaths);
    void autosavePoint(const QString& name);
    void shakeWindow(const int amplitude,const int duration,const int shakeCount);
    void close();

public slots:
    void advance();
    void onChoiceSelected(int index);
    void saveSnapshotToFile(const QString& filename);
    void loadSnapshotFromFile(const QString& filename);
    void saveSnapshotWithMeta(const QString& filename,
        const QString& screenshotPath,
        const QString& desc);

private slots:
    void onBackGame();

private:
    QString m_currentBgm;
    QString m_currentBackground;
    QMap<QString, QString> m_currentSprites;
    QMap<QString, QString> m_currentProfiles;

    void handleCommand(const GE_Line& ln);
    GE_Script m_script;
    QString m_currentSceneId;
    int m_lineIndex = 0;
    QVariantMap m_flags;
    QStack<QPair<QString, int>> m_history;

    StartWindow* m_startWindow = nullptr;
};

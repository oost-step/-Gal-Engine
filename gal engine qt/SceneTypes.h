#pragma once
#include <QString>
#include <QVector>
#include <QVariantMap>
#include <QMap>

struct GE_ChoiceOption {
    QString text;
    QString gotoSceneId;
};

struct GE_Line {
    QString speaker;
    QString text;
    QString spritePath;
    QString spriteSlot;
    QString profilePath;
    QString profileSlot;

    QString cmd;
    QVariantMap args;

    bool isChoice = false;
    QString choicePrompt;
    QVector<GE_ChoiceOption> options;
};

struct GE_Scene {
    QString id;
    QString backgroundPath;
    QString musicPath;
    QVector<GE_Line> lines;
};

struct GE_Script {
    QMap<QString, GE_Scene> scenes;
    QString startSceneId;
};

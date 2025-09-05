#pragma once
#include <QFrame>
#include <QWidget>
#include "OutlineLabel.h"
#include "OutlineTextBrowser.h"

class DialogueBox : public QFrame {
    Q_OBJECT
public:
    explicit DialogueBox(QWidget* parent = nullptr);
    void setSpeaker(const QString& name);
    void setText(const QString& t);

signals:
    void clicked();

/*
protected:
    void mousePressEvent(QMouseEvent* ev) override;
*/

private:
    OutlineLabel* m_name;
    OutlineTextBrowser* m_text;
};

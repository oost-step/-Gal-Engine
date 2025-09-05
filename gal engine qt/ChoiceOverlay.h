#pragma once
#include <QWidget>

class QLabel;
class QPushButton;
class QButtonGroup;

class ChoiceOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ChoiceOverlay(QWidget* parent = nullptr);
    void setChoices(const QString& prompt, const QStringList& options);

signals:
    void choiceSelected(int index);

private slots:
    void onChoice(int id);

private:
    void clearButtons();

    QLabel* m_prompt;
    QWidget* m_buttonsHost;
    QButtonGroup* m_group;
};

#include "ChoiceOverlay.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

ChoiceOverlay::ChoiceOverlay(QWidget* parent) : QWidget(parent) {
    setStyleSheet("QWidget { background: rgba(0,0,0,160); } QPushButton { padding: 8px 12px; font-size: 16px; }");
    setVisible(false);
    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);
    auto* panel = new QWidget(this);
    auto* panelLay = new QVBoxLayout(panel);
    m_prompt = new QLabel(panel);
    m_prompt->setStyleSheet("QLabel { color: white; font-size: 18px; font-weight: bold; }");
    panelLay->addWidget(m_prompt);
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
    m_buttonsHost = new QWidget(panel);
    auto* btnLay = new QVBoxLayout(m_buttonsHost);
    btnLay->setSpacing(8);
    panelLay->addWidget(m_buttonsHost);
    outer->addWidget(panel, 0, Qt::AlignCenter);
    connect(m_group, SIGNAL(idClicked(int)), this, SLOT(onChoice(int)));
}

void ChoiceOverlay::setChoices(const QString& prompt, const QStringList& options) {
    clearButtons();
    m_prompt->setText(prompt);
    int id = 0;
    auto* layout = static_cast<QVBoxLayout*>(m_buttonsHost->layout());
    for (const auto& opt : options) {
        auto* btn = new QPushButton(opt, m_buttonsHost);
        m_group->addButton(btn, id++);
        layout->addWidget(btn);
    }
    setVisible(true);
}

void ChoiceOverlay::onChoice(int id) {
    setVisible(false);
    emit choiceSelected(id);
}

void ChoiceOverlay::clearButtons() {
    if (!m_buttonsHost->layout()) {
        m_buttonsHost->setLayout(new QVBoxLayout());
    }
    QLayoutItem* it;
    while ((it = m_buttonsHost->layout()->takeAt(0))) {
        if (it->widget()) delete it->widget();
        delete it;
    }
    qDeleteAll(m_group->buttons());
}
    
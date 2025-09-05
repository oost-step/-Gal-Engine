#include "ScriptEngine.h"
#include "StartWindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QJsonObject>
#include <QVariantList>
#include <QJsonValue>
#include <QString>
#include <QMap>

ScriptEngine::ScriptEngine(QObject* parent) : QObject(parent) {}

bool ScriptEngine::loadFromJsonFile(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;
    return parse(doc.object());
}

bool ScriptEngine::parse(const QJsonObject& root) {
    m_script = GE_Script();
    m_flags.clear();
    m_history.clear();
    m_lineIndex = 0;
    m_currentSceneId.clear();

    m_script.startSceneId = root.value("start").toString();
    const auto arr = root.value("scenes").toArray();
    for (const auto& v : arr) {
        const auto o = v.toObject();
        GE_Scene s;
        s.id = o.value("id").toString();
        s.backgroundPath = o.value("background").toString();
        s.musicPath = o.value("music").toString();
        const auto lines = o.value("lines").toArray();
        for (const auto& lv : lines) {
            const auto lo = lv.toObject();
            GE_Line ln;
            if (lo.contains("choice")) {
                ln.isChoice = true;
                const auto ch = lo.value("choice").toObject();
                ln.choicePrompt = ch.value("prompt").toString();
                const auto ops = ch.value("options").toArray();
                for (const auto& ov : ops) {
                    const auto oo = ov.toObject();
                    GE_ChoiceOption opt{ oo.value("text").toString(), oo.value("goto").toString() };
                    ln.options.push_back(opt);
                }
            }
            else if (lo.contains("cmd")) {
                ln.cmd = lo.value("cmd").toString();
                const auto args = lo.value("args").toObject();
                for (auto it = args.begin(); it != args.end(); ++it) ln.args[it.key()] = it.value().toVariant();
            }
            else {
                ln.speaker = lo.value("speaker").toString();
                ln.text = lo.value("text").toString();
                ln.spritePath = lo.value("sprite").toString();
                ln.spriteSlot = lo.value("slot").toString();
                ln.profilePath = lo.value("psprite").toString();
                ln.profileSlot = lo.value("pslot").toString();
            }
            s.lines.push_back(std::move(ln));
        }
        m_script.scenes.insert(s.id, std::move(s));
    }
    return true;
}

void ScriptEngine::start(const QString& sceneId) {
    m_currentSceneId = sceneId.isEmpty() ? m_script.startSceneId : sceneId;
    m_lineIndex = 0;
    if (!m_script.scenes.contains(m_currentSceneId)) { emit scriptEnded(); return; }
    emit sceneEntered(m_currentSceneId);
    const auto& sc = m_script.scenes[m_currentSceneId];
    if (!sc.musicPath.isEmpty()) {
        m_currentBgm = sc.musicPath;
        emit playBgm(sc.musicPath);
    }
        
    if (!sc.backgroundPath.isEmpty()) {
        m_currentBackground = sc.backgroundPath;
        emit backgroundChanged(sc.backgroundPath);
    }
    advance();
}

void ScriptEngine::advance() {
    if (!m_script.scenes.contains(m_currentSceneId)) { emit scriptEnded(); return; }
    auto& sc = m_script.scenes[m_currentSceneId];
    if (m_lineIndex >= sc.lines.size()) { emit scriptEnded(); return; }
    const GE_Line& ln = sc.lines[m_lineIndex++];
    if (ln.isChoice) {
        QStringList opts; for (const auto& o : ln.options) opts << o.text;
        emit textReady("", "");
        emit choiceRequested(ln.choicePrompt, opts);
        return;
    }
    if (!ln.cmd.isEmpty()) {
        handleCommand(ln);
        return;
    }
    if (!ln.spritePath.isEmpty()) {
        QString slot = ln.spriteSlot.isEmpty() ? "left" : ln.spriteSlot;
        m_currentSprites[slot] = ln.spritePath;
        emit spriteChanged(ln.spriteSlot.isEmpty() ? "left" : ln.spriteSlot, ln.spritePath);
    }
    if (!ln.profilePath.isEmpty()) {
        QString sslot = ln.profileSlot.isEmpty() ? "pleft" : ln.profileSlot;
        m_currentProfiles[sslot] = ln.profilePath;
        emit spriteChangedTop(ln.spriteSlot.isEmpty() ? "pleft" : ln.profileSlot, ln.profilePath);
    }
    emit textReady(ln.speaker, ln.text);
}

void ScriptEngine::onChoiceSelected(int index) {
    if (!m_script.scenes.contains(m_currentSceneId)) { emit scriptEnded(); return; }
    const auto& sc = m_script.scenes[m_currentSceneId];
    const GE_Line& ln = sc.lines[m_lineIndex - 1];
    if (!ln.isChoice || index < 0 || index >= ln.options.size()) { advance(); return; }
    const auto target = ln.options[index].gotoSceneId;
    if (!target.isEmpty() && m_script.scenes.contains(target)) {
        m_history.push(qMakePair(m_currentSceneId, m_lineIndex));
        m_currentSceneId = target;
        m_lineIndex = 0;
        emit sceneEntered(m_currentSceneId);
        const auto& nsc = m_script.scenes[m_currentSceneId];
        if (!nsc.musicPath.isEmpty()) {
            m_currentBgm = nsc.musicPath;
            emit playBgm(nsc.musicPath);
        }
        if (!nsc.backgroundPath.isEmpty()) {
            m_currentBackground = nsc.backgroundPath;
            emit backgroundChanged(nsc.backgroundPath);
        }
    }
    advance();
}

void ScriptEngine::handleCommand(const GE_Line& ln) {
    const QString c = ln.cmd.toLower();
    if (c == "bg") {
        const QString p = ln.args.value("path").toString();
        if (!p.isEmpty()) {
            m_currentBackground = p;
            emit backgroundChanged(p);
        } 
        advance();
    }
    else if (c == "music") {
        const QString p = ln.args.value("path").toString();
        if (!p.isEmpty()) {
            m_currentBgm = p;
            emit playBgm(p);
        }
        advance();
    }
    else if (c == "se") {
        const QString p = ln.args.value("path").toString();
        if (!p.isEmpty()) emit playSe(p);
        advance();
    }
    else if (c == "wait") {
        int ms = ln.args.value("ms").toInt();
        if (ms <= 0) ms = 500;
        QTimer::singleShot(ms, this, &ScriptEngine::advance);
        emit waitRequested(ms);
    }
    else if (c == "clear") {
        const QString slot = ln.args.value("slot").toString();
        if (slot.isEmpty()) {
            QStringList slotList;
            slotList << "left" << "right" << "pleft" << "pcenter" << "pright";

            for (int i = 0; i < slotList.size(); ++i) {
                const QString s = slotList.at(i);
                m_currentSprites.remove(s);
                emit spriteCleared(s);
            }

            for (int i = 0; i < slotList.size(); ++i) {
                const QString s = slotList.at(i);
                m_currentProfiles.remove(s);
                emit spriteClearedTop(s);
            }
        }
        else {
            m_currentSprites.remove(slot);
            m_currentProfiles.remove(slot);
            emit spriteCleared(slot);
            emit spriteClearedTop(slot);
        }
        advance();
    }
    else if (c == "goto" || c == "jump") {
        const QString target = ln.args.value("scene").toString();
        const QString saveName = ln.args.value("save").toString();
        if (!target.isEmpty() && m_script.scenes.contains(target)) {
            m_history.push(qMakePair(m_currentSceneId, m_lineIndex));
            m_currentSceneId = target; m_lineIndex = 0;
            emit sceneEntered(m_currentSceneId);
            const auto& nsc = m_script.scenes[m_currentSceneId];
            if (!nsc.musicPath.isEmpty()) emit playBgm(nsc.musicPath);
            if (!nsc.backgroundPath.isEmpty()) emit backgroundChanged(nsc.backgroundPath);
            if (!saveName.isEmpty()) emit autosavePoint(saveName);
        }
        advance();
    }
    else if (c == "setflag" || c == "flag") {
        const QString key = ln.args.value("key").toString();
        const QVariant val = ln.args.value("value");
        if (!key.isEmpty()) m_flags[key] = val;
        advance();
    }
    else if (c == "ifflag") {
        const QString key = ln.args.value("key").toString();
        const QVariant want = ln.args.value("value");
        const QString ts = ln.args.value("true_scene").toString();
        const QString fs = ln.args.value("false_scene").toString();
        if (m_flags.value(key) == want) {
            if (!ts.isEmpty() && m_script.scenes.contains(ts)) {
                m_history.push(qMakePair(m_currentSceneId, m_lineIndex));
                m_currentSceneId = ts; m_lineIndex = 0;
                emit sceneEntered(m_currentSceneId);
                const auto& nsc = m_script.scenes[m_currentSceneId];
                if (!nsc.musicPath.isEmpty()) emit playBgm(nsc.musicPath);
                if (!nsc.backgroundPath.isEmpty()) emit backgroundChanged(nsc.backgroundPath);
            }
        }
        else {
            if (!fs.isEmpty() && m_script.scenes.contains(fs)) {
                m_history.push(qMakePair(m_currentSceneId, m_lineIndex));
                m_currentSceneId = fs; m_lineIndex = 0;
                emit sceneEntered(m_currentSceneId);
                const auto& nsc = m_script.scenes[m_currentSceneId];
                if (!nsc.musicPath.isEmpty()) emit playBgm(nsc.musicPath);
                if (!nsc.backgroundPath.isEmpty()) emit backgroundChanged(nsc.backgroundPath);
            }
        }
        advance();
    }
    else if (c == "preload") {
        QStringList images;
        QStringList audios;
        if (ln.args.contains("images")) {
            QVariant v = ln.args.value("images");
            if (v.canConvert<QVariantList>()) {
                for (auto vv : v.toList()) images << vv.toString();
            }
            else if (v.metaType().id() == QMetaType::QString)
            {
                images << v.toString();
            }
        }
        if (ln.args.contains("audios")) {
            QVariant v = ln.args.value("audios");
            if (v.canConvert<QVariantList>()) {
                for (auto vv : v.toList()) audios << vv.toString();
            }
            else if (v.metaType().id() == QMetaType::QString) {
                audios << v.toString();
            }
        }
        if (!images.isEmpty() || !audios.isEmpty()) emit preloadRequested(images, audios);
        advance();
    }
    else if (c == "autosave") {
        const QString name = ln.args.value("name").toString();
        if (!name.isEmpty()) emit autosavePoint(name);
        advance();
    }
    else if (c == "end") {
        onBackGame();
        }
    else {
        advance();
    }
}

QVariantMap ScriptEngine::snapshot() const {
    QVariantMap m;
    m["scene"] = m_currentSceneId;
    m["index"] = m_lineIndex;
    m["flags"] = m_flags;

    m["bgm"] = m_currentBgm;
    m["background"] = m_currentBackground;

    QVariantMap spritesVm;
    for (auto it = m_currentSprites.constBegin(); it != m_currentSprites.constEnd(); ++it) {
        spritesVm.insert(it.key(), it.value());
    }
    m["sprites"] = spritesVm;

    QVariantMap profilesVm;
    for (auto it = m_currentProfiles.constBegin(); it != m_currentProfiles.constEnd(); ++it) {
        profilesVm.insert(it.key(), it.value());
    }
    m["profiles"] = profilesVm;

    return m;
}

void ScriptEngine::restore(const QVariantMap& m) {
    m_currentSceneId = m.value("scene").toString();
    m_lineIndex = m.value("index").toInt() - 1;
    m_flags = m.value("flags").toMap();

    const QString bgm = m.value("bgm").toString();
    const QString bg = m.value("background").toString();

    QVariantMap spritesVm = m.value("sprites").toMap();
    QVariantMap profilesVm = m.value("profiles").toMap();

    if (!bgm.isEmpty() && bgm != m_currentBgm) {
        m_currentBgm = bgm;
        emit playBgm(bgm);
    }

    if (!bg.isEmpty() && bg != m_currentBackground) {
        m_currentBackground = bg;
        emit backgroundChanged(bg);
    }

    for (auto it = spritesVm.constBegin(); it != spritesVm.constEnd(); ++it) {
        const QString slot = it.key();
        const QString path = it.value().toString();
        if (m_currentSprites.value(slot) != path) {
            m_currentSprites[slot] = path;
            emit spriteChanged(slot, path);
        }
    }

    {
        QStringList toRemove;
        for (auto it = m_currentSprites.constBegin(); it != m_currentSprites.constEnd(); ++it) {
            if (!spritesVm.contains(it.key())) {
                toRemove.append(it.key());
            }
        }
        for (int i = 0; i < toRemove.size(); ++i) {
            const QString k = toRemove.at(i);
            m_currentSprites.remove(k);
            emit spriteCleared(k);
        }
    }

    for (auto it = profilesVm.constBegin(); it != profilesVm.constEnd(); ++it) {
        const QString slot = it.key();
        const QString path = it.value().toString();
        if (m_currentProfiles.value(slot) != path) {
            m_currentProfiles[slot] = path;
            emit spriteChangedTop(slot, path);
        }
    }
    {
        QStringList toRemove;
        for (auto it = m_currentProfiles.constBegin(); it != m_currentProfiles.constEnd(); ++it) {
            if (!profilesVm.contains(it.key())) {
                toRemove.append(it.key());
            }
        }
        for (int i = 0; i < toRemove.size(); ++i) {
            const QString k = toRemove.at(i);
            m_currentProfiles.remove(k);
            emit spriteClearedTop(k);
        }
    }

    emit sceneEntered(m_currentSceneId);
    advance();
}

void ScriptEngine::saveSnapshotToFile(const QString& filename) {
    QVariantMap m = snapshot();
    QJsonDocument doc = QJsonDocument::fromVariant(m);
    QFile f(filename);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(doc.toJson(QJsonDocument::Compact));
        f.close();
    }
}

void ScriptEngine::loadSnapshotFromFile(const QString& filename) {
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) return;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return;
    QVariantMap m = doc.toVariant().toMap();
    restore(m);
}

void ScriptEngine::onBackGame() {
    emit stopBgm();
    if (!m_startWindow) {
        m_startWindow = new StartWindow();
    }
    m_startWindow->show();
    this->deleteLater();
}
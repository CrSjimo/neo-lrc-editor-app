#include "DocumentObject.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QJSValue>
#include <QJSEngine>
#include <QTreeView>

#include <NeoLrcEditorApp/ItemObject.h>
#include <NeoLrcEditorApp/LyricDocument.h>
#include <NeoLrcEditorApp/MainWindow.h>
#include <NeoLrcEditorApp/PlaybackController.h>

DocumentObject::DocumentObject(QObject *parent) : QObject(parent) {
}

DocumentObject::~DocumentObject() = default;

QObject *DocumentObject::insert(int time, const QString &lyric) {
    int row = LyricDocument::instance()->model()->rowCount();
    LyricDocument::instance()->pushInsertRowCommand(row, time, lyric);
    auto index = LyricDocument::instance()->proxyModel()->mapFromSource(LyricDocument::instance()->model()->index(row, 0));
    return new ItemObject(index);
}

QJSValue DocumentObject::item(int row) const {
    auto engine = qjsEngine(this);
    auto index = LyricDocument::instance()->proxyModel()->index(row, 0);
    if (index.isValid()) {
        return engine->newQObject(new ItemObject(index));
    } else {
        return QJSValue::UndefinedValue;
    }
}

QJSValue DocumentObject::items() const {
    auto engine = qjsEngine(this);
    QList<QJSValue> ret;
    ret.reserve(LyricDocument::instance()->model()->rowCount());
    for (int row = 0; row < LyricDocument::instance()->model()->rowCount(); row++) {
        auto index = LyricDocument::instance()->proxyModel()->index(row, 0);
        ret.append(engine->newQObject(new ItemObject(index)));
    }
    return engine->toScriptValue(ret);
}

QJSValue DocumentObject::selectedItems() const {
    auto engine = qjsEngine(this);
    QList<QJSValue> ret;
    for (const auto &index : MainWindow::instance()->treeView()->selectionModel()->selectedRows()) {
        ret.append(engine->newQObject(new ItemObject(index)));
    }
    return engine->toScriptValue(ret);
}

void DocumentObject::clearSelection() {
    MainWindow::instance()->treeView()->selectionModel()->clearSelection();
}

QObject * DocumentObject::findItemByTime(int time) const {
    int row = LyricDocument::instance()->findRowByTime(time);
    auto index = LyricDocument::instance()->proxyModel()->index(row, 0);
    return new ItemObject(index);
}

int DocumentObject::itemCount() const {
    return LyricDocument::instance()->model()->rowCount();
}

void DocumentObject::setPlaybackPosition(int time) {
    PlaybackController::instance()->setPositionTime(time);
}

int DocumentObject::playbackPosition() const {
    return PlaybackController::instance()->positionTime();
}

void DocumentObject::setPlaying(bool isPlaying) {
    PlaybackController::instance()->setPlaying(isPlaying);
}

bool DocumentObject::isPlaying() const {
    return PlaybackController::instance()->isPlaying();
}

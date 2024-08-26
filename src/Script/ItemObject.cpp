#include "ItemObject.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QJSValue>
#include <QJSEngine>
#include <QTreeView>

#include <NeoLrcEditorApp/LyricDocument.h>
#include <NeoLrcEditorApp/MainWindow.h>

ItemObject::ItemObject(const QPersistentModelIndex &index, QObject *parent) : QObject(parent), m_index(index) {

}

ItemObject::~ItemObject() = default;

void ItemObject::remove() {
    LyricDocument::instance()->pushDeleteRowCommand(LyricDocument::instance()->proxyModel()->mapToSource(m_index).row());
}

void ItemObject::setLyric(const QString &lyric) {
    LyricDocument::instance()->pushEditCommand(QModelIndex(m_index).siblingAtColumn(1), lyric);
}

QString ItemObject::lyric() const {
    return LyricDocument::instance()->proxyModel()->data(QModelIndex(m_index).siblingAtColumn(1)).toString();
}

void ItemObject::setTime(int time) {
    LyricDocument::instance()->pushEditCommand(QModelIndex(m_index).siblingAtColumn(0), time);
}

int ItemObject::time() const {
    return LyricDocument::instance()->proxyModel()->data(QModelIndex(m_index).siblingAtColumn(0)).toInt();
}

void ItemObject::setSelected(bool selected) {
    MainWindow::instance()->treeView()->selectionModel()->select(m_index, selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

bool ItemObject::isSelected() const {
    return MainWindow::instance()->treeView()->selectionModel()->isSelected(m_index);
}

int ItemObject::index() const {
    return m_index.row();
}

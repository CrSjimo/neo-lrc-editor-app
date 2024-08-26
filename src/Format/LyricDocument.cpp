#include "LyricDocument.h"

#include <algorithm>

#include <QStandardItemModel>
#include <QUndoStack>
#include <QFile>
#include <QSortFilterProxyModel>

#include <NeoLrcEditorApp/LyricFormatIO.h>
#include <NeoLrcEditorApp/LyricLine.h>

static LyricDocument *m_instance = nullptr;

class LyricSortFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit LyricSortFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {
    }

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        auto model = LyricDocument::instance()->model();
        auto time1 = model->data(source_left).toInt();
        auto time2 = model->data(source_right).toInt();
        if (time1 == time2)
            return model->data(source_left.siblingAtColumn(1)).toString() < model->data(source_right.siblingAtColumn(1)).toString();
        else
            return time1 < time2;
    }
};

class EditCommand : public QUndoCommand {
public:
    explicit EditCommand(const QModelIndex &index, const QVariant &newValue, QUndoCommand *parent = nullptr)
    : QUndoCommand(parent), m_index(index), m_newValue(newValue), m_oldValue(m_instance->model()->data(index)) {
    }

    void undo() override {
        m_instance->model()->setData(m_index, m_oldValue);
        m_instance->setDirty(true);
    }

    void redo() override {
        m_instance->model()->setData(m_index, m_newValue);
        m_instance->setDirty(true);
    }

private:
    QModelIndex m_index;
    QVariant m_newValue;
    QVariant m_oldValue;
};
class MoveRowCommand : public QUndoCommand {
public:
    explicit MoveRowCommand(int sourceRow, int destinationRow, QUndoCommand *parent = nullptr)
    : QUndoCommand(parent), sourceRow(sourceRow), destinationRow(destinationRow) {
    }

    void undo() override {
        auto model = m_instance->model();
        auto destinationTime = model->data(model->index(destinationRow, 0));
        auto destinationLyric = model->data(model->index(destinationRow, 1));
        model->removeRow(destinationRow);
        model->insertRow(sourceRow);
        model->setData(model->index(sourceRow, 0),destinationTime);
        model->setData(model->index(sourceRow, 1), destinationLyric);
        m_instance->setDirty(true);
    }

    void redo() override {
        auto model = m_instance->model();
        auto sourceTime = model->data(model->index(sourceRow, 0));
        auto sourceLyric = model->data(model->index(sourceRow, 1));
        model->removeRow(sourceRow);
        model->insertRow(destinationRow);
        model->setData(model->index(destinationRow, 0),sourceTime);
        model->setData(model->index(destinationRow, 1), sourceLyric);
        m_instance->setDirty(true);
    }

private:
    int sourceRow;
    int destinationRow;
};
class InsertRowCommand : public QUndoCommand {
public:
    explicit InsertRowCommand(int row, const QString &timeString, const QString &lyric, QUndoCommand *parent = nullptr)
    : QUndoCommand(parent), row(row), timeString(timeString), lyric(lyric) {
    }

    void undo() override {
        m_instance->model()->removeRow(row);
        m_instance->setDirty(true);
    }

    void redo() override {
        auto timeItem = new QStandardItem(timeString);
        auto lyricItem = new QStandardItem(lyric);
        m_instance->model()->insertRow(row, {timeItem, lyricItem});
        m_instance->setDirty(true);
    }

private:
    int row;
    QString timeString;
    QString lyric;
};
class DeleteRowCommand : public QUndoCommand {
public:
    explicit DeleteRowCommand(int row, QUndoCommand *parent = nullptr)
    : QUndoCommand(parent), row(row), time(m_instance->model()->data(m_instance->model()->index(row, 0))), lyric(m_instance->model()->data(m_instance->model()->index(row, 1))) {
    }

    void undo() override {
        m_instance->model()->insertRow(row);
        m_instance->model()->setData(m_instance->model()->index(row, 0), time);
        m_instance->model()->setData(m_instance->model()->index(row, 1), lyric);
        m_instance->setDirty(true);
    }

    void redo() override {
        m_instance->model()->removeRow(row);
        m_instance->setDirty(true);

    }

private:
    int row;
    QVariant time;
    QVariant lyric;
};

LyricDocument::LyricDocument(QObject *parent) : QObject(parent) {
    m_instance = this;
    m_lyricModel = new QStandardItemModel(0, 2, this);
    m_lyricModel->setHeaderData(0, Qt::Horizontal, tr("Time"));
    m_lyricModel->setHeaderData(1, Qt::Horizontal, tr("Lyric"));
    m_proxyModel = new LyricSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_lyricModel);
    m_proxyModel->sort(0);
    m_undoStack = new QUndoStack(this);
}

LyricDocument::~LyricDocument() {
    m_instance = nullptr;
}

LyricDocument *LyricDocument::instance() {
    return m_instance;
}

void LyricDocument::newFile() {
    m_undoStack->clear();
    m_lyricModel->clear();
    m_lyricModel->setColumnCount(2);
    m_lyricModel->setHeaderData(0, Qt::Horizontal, tr("Time"));
    m_lyricModel->setHeaderData(1, Qt::Horizontal, tr("Lyric"));
    setFileName({});
    setDirty(false);
}

bool LyricDocument::openFile(const QString &fileName) {
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    bool ok;
    auto lyricLines = LyricFormatIO::read(&f, &ok);
    if (!ok)
        return false;
    newFile();
    buildModelFromLyricLines(lyricLines);
    setFileName(fileName);
    return true;
}

bool LyricDocument::saveFile() {
    QFile f(m_fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    LyricFormatIO::write(&f, getLyricLinesFromModel());
    setDirty(false);
    return true;
}

bool LyricDocument::saveFileAs(const QString &fileName) {
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    LyricFormatIO::write(&f, getLyricLinesFromModel());
    setDirty(false);
    setFileName(fileName);
    return true;
}

QString LyricDocument::fileName() const {
    return m_fileName;
}

bool LyricDocument::isDirty() const {
    return m_isDirty;
}

QStandardItemModel *LyricDocument::model() const {
    return m_lyricModel;
}

QSortFilterProxyModel *LyricDocument::proxyModel() const {
    return m_proxyModel;
}

QUndoStack *LyricDocument::undoStack() const {
    return m_undoStack;
}

void LyricDocument::beginTransaction(const QString &name) {
    m_undoStack->beginMacro(name);
}

void LyricDocument::pushEditCommand(const QModelIndex &index, const QVariant &value) {
    m_undoStack->push(new EditCommand(index.model() == m_proxyModel ? m_proxyModel->mapToSource(index) : index, value));
}

void LyricDocument::pushMoveRowCommand(int sourceRow, int destinationRow) {
    m_undoStack->push(new MoveRowCommand(sourceRow, destinationRow));
}

void LyricDocument::pushInsertRowCommand(int row, int time, const QString &lyric) {
    m_undoStack->push(new InsertRowCommand(row, QString::number(time), lyric));
}

void LyricDocument::pushDeleteRowCommand(int row) {
    m_undoStack->push(new DeleteRowCommand(row));
}

void LyricDocument::commitTransaction() {
    m_undoStack->endMacro();
}

void LyricDocument::abortTransaction() {
    m_undoStack->endMacro();
    auto command = const_cast<QUndoCommand *>(m_undoStack->command(m_undoStack->count() - 1));
    command->setObsolete(true);
    command->undo();
    m_undoStack->undo();
}

int LyricDocument::findRowByTime(int time) const {
    int count = m_proxyModel->rowCount();
    int first = 0;
    while (count > 0) {
        int it = first;
        int step = count / 2;
        it += step;
        if (m_proxyModel->data(m_proxyModel->index(it, 0)).toInt() < time) {
            first = ++it;
            count -= step + 1;
        } else
            count = step;
    }

    return first - 1;
}

void LyricDocument::buildModelFromLyricLines(const QList<LyricLine> &lyricLines) {
    for (const auto &lyricLine : lyricLines) {
        auto timeItem = new QStandardItem(QString::number(lyricLine.centisecond()));
        auto lyricItem = new QStandardItem(lyricLine.lyric());
        m_lyricModel->appendRow({timeItem, lyricItem});
    }
}

QList<LyricLine> LyricDocument::getLyricLinesFromModel() const {
    QList<LyricLine> ret;
    ret.reserve(m_lyricModel->rowCount());
    for (int i = 0; i < m_lyricModel->rowCount(); i++) {
        ret.append({m_lyricModel->data(m_lyricModel->index(i, 0)).toInt(), m_lyricModel->data(m_lyricModel->index(i, 1)).toString()});
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}

void LyricDocument::setFileName(const QString &fileName) {
    if (m_fileName != fileName) {
        m_fileName = fileName;
        emit fileNameChanged(fileName);
    }
}

void LyricDocument::setDirty(bool isDirty) {
    if (m_isDirty != isDirty) {
        m_isDirty = isDirty;
        emit dirtyChanged(isDirty);
    }
}

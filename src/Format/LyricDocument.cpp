#include "LyricDocument.h"

#include <QStandardItemModel>
#include <QUndoStack>
#include <QFile>

#include <NeoLrcEditorApp/LyricFormatIO.h>
#include <NeoLrcEditorApp/LyricLine.h>

static LyricDocument *m_instance = nullptr;

class EditCommand : public QUndoCommand {
public:
    EditCommand(const QModelIndex &index, const QVariant &newValue, QUndoCommand *parent = nullptr)
            : QUndoCommand(parent), m_index(index), m_newValue(newValue), m_oldValue(m_instance->model()->data(index)) {
    }

    void undo() override {
        m_instance->model()->setData(m_index, m_oldValue, Qt::EditRole);
    }

    void redo() override {
        m_instance->model()->setData(m_index, m_newValue, Qt::EditRole);
    }

private:
    QModelIndex m_index;
    QVariant m_newValue;
    QVariant m_oldValue;
};

LyricDocument::LyricDocument(QObject *parent) : QObject(parent) {
    m_instance = this;
    m_lyricModel = new QStandardItemModel(0, 2, this);
    m_lyricModel->setHeaderData(0, Qt::Horizontal, tr("Time"));
    m_lyricModel->setHeaderData(1, Qt::Horizontal, tr("Lyric"));
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

bool LyricDocument::isDirty() const {
    return m_isDirty;
}

QStandardItemModel *LyricDocument::model() const {
    return m_lyricModel;
}

QUndoStack *LyricDocument::undoStack() const {
    return m_undoStack;
}

void LyricDocument::beginTransaction(const QString &name) {
    m_undoStack->beginMacro(name);
}

void LyricDocument::pushEditCommand(const QModelIndex &index, const QVariant &value) {
    m_undoStack->push(new EditCommand(index, value));
}

void LyricDocument::commitTransaction() {
    m_undoStack->endMacro();
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

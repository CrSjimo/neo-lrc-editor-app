#include "MainWindow.h"

#include <QStandardItemModel>
#include <QSplitter>
#include <QTreeView>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QFileDialog>

#include <NeoLrcEditorApp/LyricEditorView.h>
#include <NeoLrcEditorApp/TimeValidator.h>
#include <NeoLrcEditorApp/LyricDocument.h>

class TreeViewEditTimeDelegate : public QStyledItemDelegate {
public:
    explicit TreeViewEditTimeDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        static TimeValidator validator;
        auto lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(&validator);
        return lineEdit;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QVariant value = index.model()->data(index, Qt::EditRole);
        auto lineEdit = static_cast<QLineEdit *>(editor);
        lineEdit->setText(TimeValidator::timeToString(value.toInt()));
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        auto lineEdit = static_cast<QLineEdit *>(editor);
        model->setData(index, TimeValidator::stringToTime(lineEdit->text()));
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override {
        return TimeValidator::timeToString(value.toInt());
    }
};

class TreeViewEditLyricDelegate : public QStyledItemDelegate {
public:
    explicit TreeViewEditLyricDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        static TimeValidator validator;
        auto lineEdit = new QLineEdit(parent);
        return lineEdit;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QVariant value = index.model()->data(index, Qt::EditRole);
        auto lineEdit = static_cast<QLineEdit *>(editor);
        lineEdit->setText(value.toString());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        auto lineEdit = static_cast<QLineEdit *>(editor);
        model->setData(index, lineEdit->text());
    }
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    auto splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);

    m_treeView = new QTreeView;
    m_treeView->setItemDelegateForColumn(0, new TreeViewEditTimeDelegate(m_treeView));
    m_treeView->setItemDelegateForColumn(1, new TreeViewEditLyricDelegate(m_treeView));
    splitter->addWidget(m_treeView);

    m_lyricEditorView = new LyricEditorView;
    splitter->addWidget(m_lyricEditorView);

    setCentralWidget(splitter);

    auto menuBar = new QMenuBar;
    auto fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New"), QKeySequence::New, this, &MainWindow::newFileAction);
    fileMenu->addAction(tr("&Open..."), QKeySequence::Open, this, &MainWindow::openFileAction);
    fileMenu->addAction(tr("&Save"), QKeySequence::Save, this, &MainWindow::saveFileAction);
    fileMenu->addAction(tr("Save &As..."), QKeySequence::SaveAs, this, &MainWindow::saveFileAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Import..."), Qt::CTRL | Qt::Key_I, this, &MainWindow::importAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Exit"), this, &MainWindow::exitAction);

    setMenuBar(menuBar);

    resize(800, 600);

    m_document = new LyricDocument(this);
    m_treeView->setModel(m_document->model());

}

MainWindow::~MainWindow() = default;

bool MainWindow::querySaveFile() {
    if (!m_document->isDirty())
        return true;
    auto ret = QMessageBox::warning(this, {}, tr("Current file is not saved. Do you want to save?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {
        return saveFileAction();
    } else if (ret == QMessageBox::No) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::newFileAction() {
    if (!querySaveFile())
        return;
    m_document->newFile();
}

bool MainWindow::openFileAction() {
    if (!querySaveFile())
        return false;
    auto fileName = QFileDialog::getOpenFileName(this, {}, {}, tr("LRC Files (*.lrc)"));
    if (fileName.isEmpty())
        return false;
    if (!m_document->openFile(fileName)) {
        QMessageBox::critical(this, {}, tr("Cannot open file %1").arg(fileName));
        return false;
    }
    return true;
}

bool MainWindow::saveFileAction() {
    return true;
}

bool MainWindow::saveFileAsAction() {
    return true;
}

bool MainWindow::importAction() {
    return true;
}

void MainWindow::exitAction() {
    close();
}

#include "MainWindow.h"

#include <algorithm>
#include <limits>

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
#include <QApplication>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QToolBar>
#include <QUndoStack>
#include <QTimer>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QCheckBox>
#include <QLabel>

#include <NeoLrcEditorApp/LyricEditorView.h>
#include <NeoLrcEditorApp/TimeValidator.h>
#include <NeoLrcEditorApp/LyricDocument.h>

class TimeSpinBox : public QSpinBox {
public:
    explicit TimeSpinBox(QWidget *parent = nullptr) : QSpinBox(parent) {
        m_timeValidator = new TimeValidator(this);
        setMaximum(599999);
    }

protected:
    QValidator::State validate(QString &input, int &pos) const override {
        return m_timeValidator->validate(input, pos);
    }

    int valueFromText(const QString &text) const override {
        return TimeValidator::stringToTime(text);
    }

    QString textFromValue(int val) const override {
        return TimeValidator::timeToString(val);
    }

    void fixup(QString &str) const override {
        m_timeValidator->fixup(str);
    }

private:
    TimeValidator *m_timeValidator;
};

class TreeViewEditTimeDelegate : public QStyledItemDelegate {
public:
    explicit TreeViewEditTimeDelegate(QTreeView *treeView, QObject *parent = nullptr) : QStyledItemDelegate(parent), m_treeView(treeView) {
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return new TimeSpinBox(parent);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QVariant value = index.model()->data(index, Qt::EditRole);
        auto timeSpinBox = static_cast<TimeSpinBox *>(editor);
        timeSpinBox->setValue(value.toInt());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        auto timeSpinBox = static_cast<TimeSpinBox *>(editor);
        auto data = timeSpinBox->value();
        // Determine whether inserted or modified, see MainWindow::insertAction()
        if (model->data(index, Qt::UserRole).isNull())
            LyricDocument::instance()->beginTransaction(tr("Edit Time"));
        LyricDocument::instance()->pushEditCommand(static_cast<QAbstractProxyModel *>(model)->mapToSource(index), data);
//        int destination = findSortedDestination(index);
//        if (destination != index.row()) {
//            LyricDocument::instance()->pushMoveRowCommand(index.row(), destination);
//            QTimer::singleShot(0, [=] {
//                m_treeView->setCurrentIndex(model->index(destination, 0));
//            });
//        }
        LyricDocument::instance()->commitTransaction();
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override {
        return TimeValidator::timeToString(value.toInt());
    }

    QTreeView *m_treeView;
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
        auto data = lineEdit->text();
        LyricDocument::instance()->beginTransaction(tr("Edit Lyric"));
        LyricDocument::instance()->pushEditCommand(static_cast<QAbstractProxyModel *>(model)->mapToSource(index), data);
        LyricDocument::instance()->commitTransaction();
    }
};

class LyricSortFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit LyricSortFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {
    }

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        auto model = LyricDocument::instance()->model();
        return model->data(source_left).toInt() < model->data(source_right).toInt();
    }
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;

    auto splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);

    m_treeView = new QTreeView;
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    auto timeDelegate = new TreeViewEditTimeDelegate(m_treeView, m_treeView);
    m_treeView->setItemDelegateForColumn(0, timeDelegate);
    m_treeView->setItemDelegateForColumn(1, new TreeViewEditLyricDelegate(m_treeView));
    splitter->addWidget(m_treeView);

    m_lyricEditorView = new LyricEditorView;
    splitter->addWidget(m_lyricEditorView);

    mainLayout->addWidget(splitter);

    auto playbackToolBar = new QToolBar;
    auto playPauseAction = playbackToolBar->addAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), Qt::Key_Space);
    playPauseAction->setCheckable(true);
    connect(playPauseAction, &QAction::toggled, this, [=](bool checked) {
        if (checked) {
            playPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            playPauseAction->setText(tr("Pause"));
        } else {
            playPauseAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            playPauseAction->setText(tr("Play"));
        }
    });
    playbackToolBar->addAction(playPauseAction);

    mainLayout->addWidget(playbackToolBar);

    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

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
    auto editMenu = menuBar->addMenu(tr("&Edit"));
    auto undoAction = editMenu->addAction(tr("&Undo"), QKeySequence::Undo, this, &MainWindow::undoAction);
    undoAction->setEnabled(false);
    auto redoAction = editMenu->addAction(tr("&Redo"), QKeySequence::Redo, this, &MainWindow::redoAction);
    redoAction->setEnabled(false);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Insert"), Qt::Key_Insert, this, &MainWindow::insertAction);
    editMenu->addAction(tr("Insert at Current Position"), Qt::ALT | Qt::Key_Insert, this, &MainWindow::insertAtCurrentPositionAction);
    auto deleteAction = editMenu->addAction(tr("&Delete"), Qt::Key_Delete, this, &MainWindow::deleteAction);
    deleteAction->setEnabled(false);
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, this, &MainWindow::selectAllAction);
    editMenu->addAction(tr("Deselect All"), QKeySequence::Deselect, this, &MainWindow::selectNoneAction);
    editMenu->addSeparator();
    auto quantizeAction = editMenu->addAction(tr("&Quantize..."), Qt::CTRL | Qt::Key_Q, this, &MainWindow::quantizeAction);
    quantizeAction->setEnabled(false);
    auto adjustTimeAction = editMenu->addAction(tr("Ad&just Time..."), Qt::CTRL | Qt::Key_J, this, &MainWindow::adjustTimeAction);
    adjustTimeAction->setEnabled(false);

    setMenuBar(menuBar);

    resize(800, 600);

    m_document = new LyricDocument(this);
    m_proxyModel = new LyricSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_document->model());
    m_treeView->setModel(m_proxyModel);
    m_proxyModel->sort(0);
    m_selectionModel = new QItemSelectionModel(m_proxyModel, this);
    m_treeView->setSelectionModel(m_selectionModel);

    connect(m_document, &LyricDocument::dirtyChanged, this, &MainWindow::updateTitle);
    connect(m_document, &LyricDocument::fileNameChanged, this, &MainWindow::updateTitle);
    connect(m_document->undoStack(), &QUndoStack::canUndoChanged, undoAction, &QAction::setEnabled);
    connect(m_document->undoStack(), &QUndoStack::undoTextChanged, undoAction, [=](const QString &name) {
        undoAction->setText(tr("&Undo %1").arg(name));
    });
    connect(m_document->undoStack(), &QUndoStack::canRedoChanged, redoAction, &QAction::setEnabled);
    connect(m_document->undoStack(), &QUndoStack::redoTextChanged, redoAction, [=](const QString &name) {
        redoAction->setText(tr("&Redo %1").arg(name));
    });

    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, [=] {
        if (m_selectionModel->hasSelection()) {
            deleteAction->setEnabled(true);
            quantizeAction->setEnabled(true);
            adjustTimeAction->setEnabled(true);
        } else {
            deleteAction->setEnabled(false);
            quantizeAction->setEnabled(false);
            adjustTimeAction->setEnabled(false);
        }
    });

    if (QApplication::arguments().isEmpty()) {
        m_document->newFile();
    } else {
        m_document->openFile(QApplication::arguments()[0]);
    }

    updateTitle();

}

MainWindow::~MainWindow() = default;

void MainWindow::updateTitle() {
    setWindowTitle(QString("%1 - %2%3").arg(
            QApplication::applicationName(),
            m_document->fileName().isEmpty() ? tr("Untitled") : m_document->fileName(),
            m_document->isDirty() ? QStringLiteral("*") : QString()));
}

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

void MainWindow::closeEvent(QCloseEvent *event) {
    if (querySaveFile())
        event->accept();
    else
        event->ignore();
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
    if (m_document->fileName().isEmpty())
        return saveFileAsAction();
    if (!m_document->saveFile()) {
        QMessageBox::critical(this, {}, tr("Cannot save file %1").arg(m_document->fileName()));
        return false;
    }
    return true;
}

bool MainWindow::saveFileAsAction() {
    auto fileName = QFileDialog::getSaveFileName(this, {}, m_document->fileName(), tr("LRC Files (*.lrc)"));
    if (fileName.isEmpty())
        return false;
    if (!m_document->saveFileAs(fileName)) {
        QMessageBox::critical(this, {}, tr("Cannot save file as %1").arg(m_document->fileName()));
        return false;
    }
    return true;
}

bool MainWindow::importAction() {
    return true;
}

void MainWindow::exitAction() {
    close();
}

void MainWindow::undoAction() {
    m_document->undoStack()->undo();
}

void MainWindow::redoAction() {
    m_document->undoStack()->redo();
}

void MainWindow::insertAction() {
    m_document->beginTransaction(tr("Insert"));
    m_document->pushInsertRowCommand(m_document->model()->rowCount(), {}, {});
    auto index = m_document->model()->index(m_document->model()->rowCount() - 1, 0);
    m_document->model()->setData(index, 1, Qt::UserRole);
    m_treeView->setCurrentIndex(m_proxyModel->mapFromSource(index));
    m_treeView->edit(m_proxyModel->mapFromSource(index));
}

void MainWindow::insertAtCurrentPositionAction() {

}

void MainWindow::deleteAction() {
    m_document->beginTransaction(tr("Delete"));
    auto selectedRows = m_selectionModel->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end());
    std::for_each(selectedRows.crbegin(), selectedRows.crend(), [=](const auto &index) {
        m_document->pushDeleteRowCommand(m_proxyModel->mapToSource(index).row());
    });
    m_document->commitTransaction();
}

void MainWindow::selectAllAction() {
    m_treeView->selectAll();
}

void MainWindow::selectNoneAction() {
    m_treeView->clearSelection();
}

void MainWindow::quantizeAction() {
    QDialog dlg;
    auto mainLayout = new QVBoxLayout;
    auto formLayout = new QFormLayout;
    auto tempoSpinBox = new QDoubleSpinBox;
    tempoSpinBox->setRange(1.0, std::numeric_limits<double>::max());
    tempoSpinBox->setValue(120.0);
    formLayout->addRow(tr("&Tempo"), tempoSpinBox);
    auto alignComboBox = new QComboBox;
    alignComboBox->addItem(tr("Whole note"), 0.25);
    alignComboBox->addItem(tr("Half note"), 0.5);
    alignComboBox->addItem(tr("Half note triplet"), 0.75);
    alignComboBox->addItem(tr("Quarter note"), 1);
    alignComboBox->addItem(tr("Quarter note triplet"), 1.5);
    alignComboBox->addItem(tr("Eighth note"), 2);
    alignComboBox->addItem(tr("Eighth note triplet"), 3);
    alignComboBox->addItem(tr("1/16 note"), 4);
    alignComboBox->addItem(tr("1/16 note triplet"), 6);
    alignComboBox->addItem(tr("1/32 note"), 8);
    alignComboBox->addItem(tr("1/32 note triplet"), 12);
    formLayout->addRow(tr("&Alignment"), alignComboBox);
    mainLayout->addLayout(formLayout);
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    auto okButton = new QPushButton(tr("OK"));
    buttonLayout->addWidget(okButton);
    auto cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    dlg.setLayout(mainLayout);
    dlg.setWindowTitle(tr("Quantize"));

    connect(okButton, &QAbstractButton::clicked, &dlg, &QDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Rejected)
        return;

    auto tempo = tempoSpinBox->value();
    auto ratio = alignComboBox->currentData().toDouble();
    auto div = 6000.0 / tempo / ratio;

    m_document->beginTransaction(tr("Quantize"));
    for (const auto &index : m_selectionModel->selectedRows()) {
        auto oldTime = index.model()->data(index).toInt();
        auto newTime = static_cast<int>(std::round(std::round(oldTime / div) * div));
        m_document->pushEditCommand(m_proxyModel->mapToSource(index), newTime);
    }
    m_document->commitTransaction();

}

void MainWindow::adjustTimeAction() {
    QDialog dlg;
    auto mainLayout = new QVBoxLayout;

    auto formLayout = new QFormLayout;
    auto multiplyRatioSpinBox = new QDoubleSpinBox;
    multiplyRatioSpinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    multiplyRatioSpinBox->setValue(1.0);
    formLayout->addRow(tr("&Multiply"), multiplyRatioSpinBox);
    auto timeLayout = new QHBoxLayout;
    auto offsetSpinBox = new TimeSpinBox;
    timeLayout->addWidget(offsetSpinBox);
    auto backwardCheckBox = new QCheckBox(tr("&Backward"));
    timeLayout->addWidget(backwardCheckBox);
    auto offsetLabel = new QLabel(tr("&Offset (post multiply)"));
    offsetLabel->setBuddy(offsetSpinBox);
    formLayout->addRow(offsetLabel, timeLayout);
    mainLayout->addLayout(formLayout);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    auto okButton = new QPushButton(tr("OK"));
    buttonLayout->addWidget(okButton);
    auto cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    dlg.setLayout(mainLayout);
    dlg.setWindowTitle(tr("Adjust Time"));

    connect(okButton, &QAbstractButton::clicked, &dlg, &QDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, &dlg, &QDialog::reject);

    retry:
    if (dlg.exec() == QDialog::Rejected)
        return;

    QHash<QModelIndex, int> newTimeDict;
    auto ratio = multiplyRatioSpinBox->value();
    auto offset = (backwardCheckBox->isChecked() ? -1 : 1) * offsetSpinBox->value();
    for (const auto &index : m_selectionModel->selectedRows()) {
        auto oldTime = index.model()->data(index).toInt();
        auto newTime = static_cast<int>(std::round(oldTime * ratio + offset));
        if (newTime < 0) {
            QMessageBox::warning(this, {}, "Time becomes negative after adjustment. Please Retry.");
            goto retry;
        }
        newTimeDict.insert(index, newTime);
    }
    m_document->beginTransaction(tr("Adjust Time"));
    std::for_each(newTimeDict.constKeyValueBegin(), newTimeDict.constKeyValueEnd(), [=](const auto &pair) {
        m_document->pushEditCommand(m_proxyModel->mapToSource(pair.first), pair.second);
    });
    m_document->commitTransaction();
}

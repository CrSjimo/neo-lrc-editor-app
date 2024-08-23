#include "MainWindow.h"

#include <algorithm>
#include <limits>
#include <set>

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
#include <QSortFilterProxyModel>
#include <QLabel>

#include <TalcsFormat/AudioFormatIO.h>

#include <NeoLrcEditorApp/LyricEditorView.h>
#include <NeoLrcEditorApp/TimeValidator.h>
#include <NeoLrcEditorApp/LyricDocument.h>
#include <NeoLrcEditorApp/PlaybackController.h>
#include <NeoLrcEditorApp/QuantizeDialog.h>
#include <NeoLrcEditorApp/TimeSpinBox.h>
#include <NeoLrcEditorApp/AdjustTimeDialog.h>
#include <NeoLrcEditorApp/ImportDialog.h>

static MainWindow *m_instance = nullptr;

class TreeViewEditTimeDelegate : public QStyledItemDelegate {
public:
    explicit TreeViewEditTimeDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {
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
        LyricDocument::instance()->pushEditCommand(index, data);
        LyricDocument::instance()->commitTransaction();
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
        auto data = lineEdit->text();
        LyricDocument::instance()->beginTransaction(tr("Edit Lyric"));
        LyricDocument::instance()->pushEditCommand(index, data);
        LyricDocument::instance()->commitTransaction();
    }
};

class CurrentLyricLabel : public QLabel {
public:
    explicit CurrentLyricLabel(QWidget *parent = nullptr) : QLabel(parent) {
    }

    void setRow(int row) {
        m_row = row;
        if (row == -1)
            setText({});
        else
            setText(LyricDocument::instance()->proxyModel()->data(LyricDocument::instance()->proxyModel()->index(m_row, 1)).toString());
    }

    int m_row = -1;

protected:
    void mousePressEvent(QMouseEvent *ev) override {
        if (m_row != -1)
            m_instance->treeView()->setCurrentIndex(LyricDocument::instance()->proxyModel()->index(m_row, 0));
    }
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_instance = this;

    auto playbackController = new PlaybackController(this);
    if (!playbackController->initialize())
        QMessageBox::critical(this, {}, tr("Cannot initialize audio"));

    m_document = new LyricDocument(this);

    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;

    auto splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);

    m_treeView = new QTreeView;
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    auto timeDelegate = new TreeViewEditTimeDelegate(m_treeView);
    m_treeView->setItemDelegateForColumn(0, timeDelegate);
    m_treeView->setItemDelegateForColumn(1, new TreeViewEditLyricDelegate(m_treeView));
    m_treeView->setModel(m_document->proxyModel());
    m_selectionModel = m_treeView->selectionModel();
    splitter->addWidget(m_treeView);

    m_lyricEditorView = new LyricEditorView;
    splitter->addWidget(m_lyricEditorView);

    mainLayout->addWidget(splitter);

    auto playbackTransportLayout = new QHBoxLayout;
    auto currentTimeLabel = new QLabel(TimeValidator::timeToString(0));
    playbackTransportLayout->addWidget(currentTimeLabel);
    auto timeSlider = new QSlider;
    timeSlider->setOrientation(Qt::Horizontal);
    timeSlider->setRange(0, 0);
    playbackTransportLayout->addWidget(timeSlider);
    auto totalTimeLabel = new QLabel(TimeValidator::timeToString(0));
    playbackTransportLayout->addWidget(totalTimeLabel);
    mainLayout->addLayout(playbackTransportLayout);

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
        playbackController->setPlaying(checked);
    });
    playbackToolBar->addAction(playPauseAction);
    playbackToolBar->addSeparator();
    auto audioFileNameLabel = new QLabel;
    playbackToolBar->addWidget(audioFileNameLabel);
    mainLayout->addWidget(playbackToolBar);

    QPalette alternativePalette;
    auto alternativeColor = alternativePalette.color(QPalette::WindowText);
    alternativeColor.setAlpha(0x7f);
    alternativePalette.setColor(QPalette::WindowText, alternativeColor);
    auto previousLyricLabel = new CurrentLyricLabel;
    previousLyricLabel->setPalette(alternativePalette);
    mainLayout->addWidget(previousLyricLabel);
    auto currentLyricLabel = new CurrentLyricLabel;
    mainLayout->addWidget(currentLyricLabel);
    auto nextLyricLabel = new CurrentLyricLabel;
    nextLyricLabel->setPalette(alternativePalette);
    mainLayout->addWidget(nextLyricLabel);

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
    auto deleteAction = editMenu->addAction(tr("&Delete"), Qt::Key_Delete, this, &MainWindow::deleteAction);
    deleteAction->setEnabled(false);
    editMenu->addSeparator();
    auto setTimeAction = editMenu->addAction(tr("Set Time"), Qt::CTRL | Qt::Key_F9, this, &MainWindow::setTimeAction);
    auto setTimeAndNextAction = editMenu->addAction(tr("Set Time and Next"), Qt::Key_F9, this, &MainWindow::setTimeAndNextAction);
    editMenu->addAction(tr("Insert Empty Line"), Qt::CTRL | Qt::Key_F10, this, &MainWindow::insertEmptyLineAction);
    editMenu->addAction(tr("Insert Empty Line and Next"), Qt::Key_F10, this, &MainWindow::insertEmptyLineAndNextAction);
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, this, &MainWindow::selectAllAction);
    editMenu->addAction(tr("Deselect All"), QKeySequence::Deselect, this, &MainWindow::selectNoneAction);
    editMenu->addSeparator();
    auto quantizeAction = editMenu->addAction(tr("&Quantize..."), Qt::CTRL | Qt::Key_Q, this, &MainWindow::quantizeAction);
    quantizeAction->setEnabled(false);
    auto adjustTimeAction = editMenu->addAction(tr("Ad&just Time..."), Qt::CTRL | Qt::Key_J, this, &MainWindow::adjustTimeAction);
    adjustTimeAction->setEnabled(false);

    auto playbackMenu = menuBar->addMenu(tr("&Playback"));
    playbackMenu->addAction(tr("&Open Audio File..."), Qt::CTRL | Qt::ALT | Qt::Key_O, this, &MainWindow::openAudioFileAction);
    playbackMenu->addAction(tr("&Close Audio File"), Qt::CTRL | Qt::ALT | Qt::Key_W , this, &MainWindow::closeAudioFileAction);

    setMenuBar(menuBar);

    resize(800, 600);

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
        auto flag = m_selectionModel->hasSelection();
        deleteAction->setEnabled(flag);
        quantizeAction->setEnabled(flag);
        adjustTimeAction->setEnabled(flag);
        setTimeAction->setEnabled(flag);
        setTimeAndNextAction->setEnabled(flag);
    });

    connect(playbackController, &PlaybackController::positionTimeChanged, this, [=](int time) {
        QSignalBlocker blocker(timeSlider);
        timeSlider->setValue(time);
        currentTimeLabel->setText(TimeValidator::timeToString(time));
        auto currentRow = m_document->findRowByTime(time);
        previousLyricLabel->setRow(qMax(-1, currentRow - 1));
        currentLyricLabel->setRow(currentRow);
        nextLyricLabel->setRow(currentRow + 1);
    });
    connect(timeSlider, &QSlider::valueChanged, playbackController, &PlaybackController::setPositionTime);
    connect(playbackController, &PlaybackController::audioFileNameChanged, this, [=](const QString &fileName) {
        audioFileNameLabel->setText(fileName);
        totalTimeLabel->setText(TimeValidator::timeToString(playbackController->audioLengthTime()));
        timeSlider->setMaximum(playbackController->audioLengthTime());

    });

    if (QApplication::arguments().isEmpty()) {
        m_document->newFile();
    } else {
        m_document->openFile(QApplication::arguments()[0]);
    }

    updateTitle();

}

MainWindow::~MainWindow() {
    m_instance = nullptr;
}

MainWindow *MainWindow::instance() {
    return m_instance;
}

QTreeView *MainWindow::treeView() const {
    return m_treeView;
}

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
    if (!querySaveFile())
        return false;
    ImportDialog dlg;
    if (dlg.exec() == QDialog::Rejected)
        return false;
    m_document->newFile();
    auto lyrics = dlg.text().split('\n');
    auto baseTime = dlg.initialTime();
    for (int i = 0; i < lyrics.size(); i++) {
        m_document->model()->appendRow({new QStandardItem(QString::number(baseTime + i)), new QStandardItem(lyrics[i])});
    }
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
    m_document->pushInsertRowCommand(m_document->model()->rowCount(), PlaybackController::instance()->positionTime(), {});
    auto index = m_document->model()->index(m_document->model()->rowCount() - 1, 0);
    m_document->model()->setData(index, 1, Qt::UserRole);
    m_treeView->setCurrentIndex(m_document->proxyModel()->mapFromSource(index));
    m_treeView->edit(m_document->proxyModel()->mapFromSource(index));
}

void MainWindow::deleteAction() {
    m_document->beginTransaction(tr("Delete"));
    auto selectedRows = m_selectionModel->selectedRows();
    for (auto &index : selectedRows) {
        index = m_document->proxyModel()->mapToSource(index);
    }
    std::sort(selectedRows.begin(), selectedRows.end());
    std::for_each(selectedRows.crbegin(), selectedRows.crend(), [=](const auto &index) {
        m_document->pushDeleteRowCommand(index.row());
    });
    m_document->commitTransaction();
}

void MainWindow::setTimeAction() {
    auto currentIndex = m_treeView->currentIndex().siblingAtColumn(0);
    m_document->beginTransaction(tr("Set Time"));
    m_document->pushEditCommand(currentIndex, PlaybackController::instance()->positionTime());
    m_document->commitTransaction();
}

void MainWindow::setTimeAndNextAction() {
    setTimeAction();
    auto currentIndex = m_treeView->currentIndex().siblingAtColumn(0);
    m_treeView->setCurrentIndex(currentIndex.siblingAtRow(currentIndex.row() + 1));
}

void MainWindow::insertEmptyLineAction() {
    m_document->beginTransaction(tr("Insert Empty Line"));
    m_document->pushInsertRowCommand(m_document->model()->rowCount(), PlaybackController::instance()->positionTime(), {});
    m_document->commitTransaction();
    auto index = m_document->model()->index(m_document->model()->rowCount() - 1, 0);
    m_treeView->setCurrentIndex(m_document->proxyModel()->mapFromSource(index));
}

void MainWindow::insertEmptyLineAndNextAction() {
    insertEmptyLineAction();
    auto currentIndex = m_treeView->currentIndex().siblingAtColumn(0);
    m_treeView->setCurrentIndex(currentIndex.siblingAtRow(currentIndex.row() + 1));
}

void MainWindow::selectAllAction() {
    m_treeView->selectAll();
}

void MainWindow::selectNoneAction() {
    m_treeView->clearSelection();
}

void MainWindow::quantizeAction() {
    QuantizeDialog dlg;
    if (dlg.exec() == QDialog::Rejected)
        return;
    auto div = dlg.div();

    m_document->beginTransaction(tr("Quantize"));
    for (const auto &index : m_selectionModel->selectedRows()) {
        auto oldTime = index.model()->data(index).toInt();
        auto newTime = static_cast<int>(std::round(std::round(oldTime / div) * div));
        m_document->pushEditCommand(index, newTime);
    }
    m_document->commitTransaction();

}

void MainWindow::adjustTimeAction() {
    AdjustTimeDialog dlg;
    retry:
    if (dlg.exec() == QDialog::Rejected)
        return;

    QHash<QModelIndex, int> newTimeDict;
    auto ratio = dlg.ratio();
    auto offset = dlg.offset();
    for (const auto &index : m_selectionModel->selectedRows()) {
        auto oldTime = index.model()->data(index).toInt();
        auto newTime = static_cast<int>(std::round(oldTime * ratio + offset));
        if (newTime < 0) {
            QMessageBox::warning(this, {}, "Time becomes negative after adjustment. Please retry.");
            goto retry;
        }
        newTimeDict.insert(m_document->proxyModel()->mapToSource(index), newTime);
    }
    m_document->beginTransaction(tr("Adjust Time"));
    std::for_each(newTimeDict.constKeyValueBegin(), newTimeDict.constKeyValueEnd(), [=](const auto &pair) {
        m_document->pushEditCommand(pair.first, pair.second);
    });
    m_document->commitTransaction();
}

void MainWindow::openAudioFileAction() {
    static auto filters = ([] {
        QStringList ret;
        std::set<QString> extensions;
        for (const auto &fmtInfo : talcs::AudioFormatIO::availableFormats()) {
            QStringList fmtExtensions;
            fmtExtensions.append(fmtInfo.extension);
            if (fmtInfo.extension == "raw") {
                continue;
            }
            for (const auto &subtypeInfo : fmtInfo.subtypes) {
                fmtExtensions += subtypeInfo.extensions;
            }
            extensions.insert(fmtExtensions.cbegin(), fmtExtensions.cend());
            std::transform(fmtExtensions.cbegin(), fmtExtensions.cend(), fmtExtensions.begin(), [](const QString &extension) {
                return "*." + extension;
            });
            ret.append(QString("%1 (%2)").arg(fmtInfo.name, fmtExtensions.join(" ")));
        }
        QStringList allSupportedFileExtensions;
        allSupportedFileExtensions.reserve(extensions.size());
        std::transform(extensions.cbegin(), extensions.cend(), std::back_inserter(allSupportedFileExtensions), [](const QString &extension) {
            return "*." + extension;
        });
        ret.prepend(tr("All supported files (%1)").arg(allSupportedFileExtensions.join(" ")));
        return ret.join(";;");
    })();
    auto fileName = QFileDialog::getOpenFileName(this, {}, {}, filters);
    if (fileName.isEmpty())
        return;
    if (!PlaybackController::instance()->openAudioFile(fileName)) {
        QMessageBox::critical(this, {}, tr("Cannot open audio file %1").arg(fileName));
        return;
    }
    auto lyricFileName = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).baseName() + QStringLiteral(".lrc"));
    if (LyricDocument::instance()->fileName() != lyricFileName) {
        if (!QFileInfo(lyricFileName).isFile())
            return;
        if (QMessageBox::question(this, {}, tr("Also open %1?").arg(lyricFileName)) == QMessageBox::No)
            return;
        if (!m_document->openFile(lyricFileName)) {
            QMessageBox::critical(this, {}, tr("Cannot open file %1").arg(fileName));
            return;
        }
    }
}

void MainWindow::closeAudioFileAction() {
    Q_UNUSED(this);
    PlaybackController::instance()->closeAudioFile();
}

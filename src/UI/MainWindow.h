#ifndef NEOLRCEDITORAPP_MAINWINDOW_H
#define NEOLRCEDITORAPP_MAINWINDOW_H

#include <QMainWindow>

class QTreeView;
class QToolBar;
class QItemSelectionModel;

class LyricDocument;
class LyricEditorView;

class LyricSortFilterProxyModel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:

    void updateTitle();

    bool querySaveFile();

    void newFileAction();
    bool openFileAction();
    bool saveFileAction();
    bool saveFileAsAction();
    bool importAction();
    void exitAction();

    void undoAction();
    void redoAction();
    void insertAction();
    void insertAtCurrentPositionAction();
    void deleteAction();
    void selectAllAction();
    void selectNoneAction();
    void quantizeAction();
    void adjustTimeAction();
    LyricDocument *m_document;

    QTreeView *m_treeView;
    LyricEditorView *m_lyricEditorView;
    QItemSelectionModel *m_selectionModel;
    LyricSortFilterProxyModel *m_proxyModel;
};


#endif //NEOLRCEDITORAPP_MAINWINDOW_H

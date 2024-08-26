#ifndef NEOLRCEDITORAPP_MAINWINDOW_H
#define NEOLRCEDITORAPP_MAINWINDOW_H

#include <QMainWindow>

class QTreeView;
class QToolBar;
class QItemSelectionModel;
class QJSEngine;

class LyricDocument;
class LyricEditorView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    static MainWindow *instance();

    QTreeView *treeView() const;

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
    void deleteAction();
    void setTimeAction();
    void setTimeAndNextAction();
    void insertEmptyLineAction();
    void insertEmptyLineAndNextAction();
    void selectAllAction();
    void selectNoneAction();
    void quantizeAction();
    void adjustTimeAction();

    void openAudioFileAction();
    void closeAudioFileAction();

    void reloadScriptsAction();
    void openScriptDirAction();
    void executeScript(const QString &fileName);

    LyricDocument *m_document;

    QTreeView *m_treeView;
    LyricEditorView *m_lyricEditorView;
    QItemSelectionModel *m_selectionModel;

    QMenu *m_batchProcessMenu;
    QJSEngine *m_engine;
};


#endif //NEOLRCEDITORAPP_MAINWINDOW_H

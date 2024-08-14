#ifndef NEOLRCEDITORAPP_MAINWINDOW_H
#define NEOLRCEDITORAPP_MAINWINDOW_H

#include <QMainWindow>
class QTreeView;

class LyricDocument;
class LyricEditorView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    bool querySaveFile();

    void newFileAction();
    bool openFileAction();
    bool saveFileAction();
    bool saveFileAsAction();
    bool importAction();
    void exitAction();

    LyricDocument *m_document;

    QTreeView *m_treeView;
    LyricEditorView *m_lyricEditorView;
};


#endif //NEOLRCEDITORAPP_MAINWINDOW_H

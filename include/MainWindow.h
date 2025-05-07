#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QListWidget>
#include "CanvasWidget.h"
#include "ToolBar.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateStatusBar();
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void exportAsImage();
    void importBackground();
    void about();
    void updateShapeList();

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void createControls();
    void setupConnections();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);

    CanvasWidget *m_canvas;
    ToolBar *m_toolBar;
    QString m_currentFile;

    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_helpMenu;

    QAction *m_newAct;
    QAction *m_openAct;
    QAction *m_saveAct;
    QAction *m_saveAsAct;
    QAction *m_exportImageAct;
    QAction *m_importBackgroundAct;
    QAction *m_exitAct;
    QAction *m_undoAct;
    QAction *m_redoAct;
    QAction *m_clearAct;
    QAction *m_aboutAct;

    QListWidget* m_shapeListWidget;
    QPushButton* m_deleteButton;
    QPushButton* m_moveUpButton;
    QPushButton* m_moveDownButton;

    QPushButton* m_stopAnimationButton;
};

#endif // MAINWINDOW_H
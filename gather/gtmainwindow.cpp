/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtmainwindow.h"
#include "gtdocloader.h"
#include "gtdocmodel.h"
#include "gtdocoutline.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include "gtdocview.h"
#include <QtCore/QtDebug>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

GT_BEGIN_NAMESPACE

GtMainWindow::GtMainWindow()
{
    setupUi(this);

    // document thread
    docLoader = QSharedPointer<GtDocLoader>(new GtDocLoader());
    docModel = QSharedPointer<GtDocModel>(new GtDocModel());
    docModel->setMinScale(0.1);
    docModel->setMaxScale(4.0);
    docModel->setMouseMode(GtDocModel::SelectText);
    docThread = new QThread(this);

    QDir dir(QCoreApplication::applicationDirPath());

    if (dir.cd("loader")) {
        int count = docLoader->registerLoaders(dir.absolutePath());
        qDebug() << "registered loaders:" << count;
    }

    docThread->start();

    // GUI thread
    docView->setModel(docModel.data());
    docView->setRenderThread(docThread);
    docView->setRenderCacheSize(1024 * 1024 * 20);

    // Recent files
    recentFileActions[0] = actionRecentFile0;
    recentFileActions[1] = actionRecentFile1;
    recentFileActions[2] = actionRecentFile2;
    recentFileActions[3] = actionRecentFile3;
    recentFileActions[4] = actionRecentFile4;

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    recentFileSeparator = menuFile->insertSeparator(actionExit);

    // Settings
    readSettings();
    setWindowIcon(QIcon(":/images/logo.bmp"));
    setCurrentFile("");
}

GtMainWindow::~GtMainWindow()
{
    docView->setModel(0);
    document.clear();
    docModel.clear();
    docLoader.clear();

    docThread->quit();
    docThread->wait();
}

void GtMainWindow::on_actionOpen_triggered()
{
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(
            this, tr("Open Document"), lastOpenPath,
            tr("Document Files (*.pdf *.txt);;All Files (*.*)"));

        if (fileName.isEmpty())
            return;

        lastOpenPath = QFileInfo(fileName).path();
        loadFile(fileName);
    }
}

void GtMainWindow::on_actionExit_triggered()
{
    close();
}

void GtMainWindow::on_actionZoomIn_triggered()
{
    docView->zoomIn();
}

void GtMainWindow::on_actionZoomOut_triggered()
{
    docView->zoomOut();
}

void GtMainWindow::on_actionRotateLeft_triggered()
{
    docModel->setRotation(docModel->rotation() - 90);
}

void GtMainWindow::on_actionRotateRight_triggered()
{
    docModel->setRotation(docModel->rotation() + 90);
}

void GtMainWindow::on_actionAboutGather_triggered()
{
}

void GtMainWindow::docLoaded(GtDocument *doc)
{
    if (doc != document.data()) {
        qWarning() << "invalid document loaded:" << document.data() << doc;
        return;
    }

    if (document->isLoaded()) {
        docModel->setDocument(document.data());

        GtDocOutline *outline = document->outline();
        GtDocOutline::Iterator it(*outline);
        while (it.hasNext()) {
            qDebug() << it.next();
        }
    }
    else {
        Q_ASSERT(0);
    }
}

void GtMainWindow::readSettings()
{
    QSettings settings("Software Inc.", "Spreadsheet");
    restoreGeometry(settings.value("geometry").toByteArray());
    recentFiles = settings.value("recentFiles").toStringList();
    lastOpenPath = settings.value("lastOpenPath").toString();
    updateRecentFileActions();
}

void GtMainWindow::writeSettings()
{
    QSettings settings("Software Inc.", "Spreadsheet");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("recentFiles", recentFiles);
    settings.setValue("lastOpenPath", lastOpenPath);
}

bool GtMainWindow::okToContinue()
{
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("hello"),
                                     tr("The document has been modified.\n"
                                        "Do you want to save your changes?"),
                                     QMessageBox::Yes | QMessageBox::No
                                     | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            return true;
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}

bool GtMainWindow::loadFile(const QString &fileName)
{
    docModel->setDocument(0);

    GtDocument *doc = docLoader->loadDocument(fileName, docThread);
    if (NULL == doc)
        return false;

    document = QSharedPointer<GtDocument>(doc, &QObject::deleteLater);
    Q_ASSERT(document->thread() == docThread);

    if (document->isLoaded()) {
        docLoaded(doc);
    }
    else {
        connect(doc, SIGNAL(loaded(GtDocument*)), this, SLOT(docLoaded(GtDocument*)));
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    return true;
}

void GtMainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = tr("gather");
    if (!curFile.isEmpty()) {
        shownName = strippedName(curFile);
        recentFiles.removeAll(curFile);
        recentFiles.prepend(curFile);
        updateRecentFileActions();
    }

    setWindowTitle(shownName);
}

void GtMainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }
    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);
        } else {
            recentFileActions[j]->setVisible(false);
        }
    }

    recentFileSeparator->setVisible(!recentFiles.isEmpty());
}

QString GtMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void GtMainWindow::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            loadFile(action->data().toString());
    }
}

void GtMainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;

    default:
        break;
    }
}

void GtMainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

GT_END_NAMESPACE

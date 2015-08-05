#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QThreadPool>

#include <hadith.h>
#include <hadithCommon.h>

#include "initializationworker.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
//    ui->progressBar->setVisible(false);
    ui->statusbar->showMessage("Idle.");
    connect(this, SIGNAL(updateProgress(int)), ui->progressBar, SLOT(setValue(int)), Qt::DirectConnection);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);

    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

        default:
            break;
    }
}

void MainWindow::report(int value) {
//    qDebug() << "Reporting value " << value;
    emit updateProgress(value);
//    ui->progressBar->setValue(value);
}

void MainWindow::setCurrentAction(const QString &action) {
    ui->statusbar->showMessage(action);
}

void MainWindow::resetActionDisplay() {
    ui->progressBar->reset();
    ui->statusbar->showMessage("Idle.");
}

void MainWindow::tag(int /*start*/, int /*length*/, QColor /*color*/, bool /*textcolor*/) {
//    QTextBrowser *taggedBox = m_ui->hadith_display;
//    QTextCursor c = taggedBox->textCursor();
//    c.setPosition(start, QTextCursor::MoveAnchor);
//    c.setPosition(start + length, QTextCursor::KeepAnchor);
//    taggedBox->setTextCursor(c);
//    if (textcolor) {
//        taggedBox->setTextColor(color);
//    } else {
//        taggedBox->setTextBackgroundColor(color);
//    }
}

void MainWindow::startTaggingText(QString &/*text*/) {
//    QTextBrowser *taggedBox = m_ui->hadith_display;
//    taggedBox->clear();
//    taggedBox->setLayoutDirection(Qt::RightToLeft);
//    QTextCursor c = taggedBox->textCursor();
//    c.clearSelection();
//    c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
//    taggedBox->setTextCursor(c);
//    taggedBox->setTextBackgroundColor(Qt::white);
//    taggedBox->setTextColor(Qt::black);
//    taggedBox->setText(text);
}

void MainWindow::finishTaggingText() {
//    QTextBrowser *taggedBox = m_ui->hadith_display;
//    QTextCursor c = taggedBox->textCursor();
//    c.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
//    taggedBox->setTextCursor(c);
}

QString MainWindow::getFileName() {
//    if (browseFileDlg == NULL) {
//        QString dir = QDir::currentPath();
//        browseFileDlg = new QFileDialog(NULL, QString("Open File"), dir, QString("All Files (*)"));
//        browseFileDlg->setOptions(QFileDialog::DontUseNativeDialog);
//        browseFileDlg->setFileMode(QFileDialog::ExistingFile);
//        browseFileDlg->setViewMode(QFileDialog::Detail);
//    }
//    if (browseFileDlg->exec()) {
//        QStringList files = browseFileDlg->selectedFiles();
//        QString fileName = files[0];
//        return fileName;
//    }
    return "";
}

void MainWindow::displayGraph(AbstractGraph */*graph*/) {
//    try {
//        system("dot -Tsvg graph.dot -o graph.svg");
//        if (graph == NULL) {
//            QMainWindow *mw = new QMainWindow(NULL);
//            mw->setWindowTitle(QString("Sarf Graph (") + m_ui->input->toPlainText() + ")");
//            QScrollArea *sa = new QScrollArea(mw);
//            mw->setCentralWidget(sa);
//            QLabel *pic = new QLabel(sa);
//            pic->setPixmap(QPixmap("./graph.svg"));
//            sa->setWidget(pic);
//            mw->show();
//        } else {
//            GeneMainWindow *m = new GeneMainWindow;
//            m->show();
//            GeneTree *t = (dynamic_cast<GeneTree *>(graph))->duplicate();
//            m->display(t, "./graph.svg", true);
//        }
//    } catch (...) {
//    }
}

void MainWindow::on_initializeButton_clicked() {
    ui->progressBar->setVisible(true);
    ui->initializeButton->setEnabled(false);
    ui->statusbar->showMessage("Initializing...");
    theSarf = new Sarf();

    if (!theSarf->start(&output_str, &error_str, this)) {
        QMessageBox::critical(this, tr("ATMineDemo"), tr("Cannot set up the project. The application will now exit."));
    }

    Sarf::use(theSarf);
    QThread *thread = new QThread();
    InitializationWorker *worker = new InitializationWorker();
    connect(thread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    worker->moveToThread(thread);
    qDebug() << this->thread()->currentThreadId();
    thread->start();
    qDebug() << "WTF!?";
}

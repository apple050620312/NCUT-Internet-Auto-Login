#include "MainWindow.h"

#include <QTabWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>

#include "Logger.h"
#include "Network.h"
#include "Config.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle("NCUT Auto Login (Qt)");
  buildUi();
  setupTray();
  loadConfig();
  applyTheme();

  connect(&Logger::instance(), &Logger::newLine, this, [this](const QString& line){
    if (logEdit_) { logEdit_->appendPlainText(line.trimmed()); }
  });

  connect(&mon_, &Monitor::networkRestored, this, &MainWindow::updateStatus);
  connect(&mon_, &Monitor::loginSucceeded, this, &MainWindow::updateStatus);
  connect(&mon_, &Monitor::loginFailed, this, &MainWindow::updateStatus);
}

MainWindow::~MainWindow() {}

void MainWindow::buildUi() {
  tabs_ = new QTabWidget(this);
  QWidget* dash = new QWidget(this);
  QWidget* settings = new QWidget(this);
  QWidget* logs = new QWidget(this);

  // Dashboard
  statusLabel_ = new QLabel(tr("Status: Stopped"), dash);
  startBtn_ = new QPushButton(tr("Start"), dash);
  stopBtn_ = new QPushButton(tr("Stop"), dash);
  stopBtn_->setEnabled(false);
  auto dashLayout = new QVBoxLayout;
  auto btnRow = new QHBoxLayout; btnRow->addWidget(startBtn_); btnRow->addWidget(stopBtn_); btnRow->addStretch();
  dashLayout->addWidget(statusLabel_); dashLayout->addLayout(btnRow); dashLayout->addStretch();
  dash->setLayout(dashLayout);

  connect(startBtn_, &QPushButton::clicked, this, &MainWindow::onStart);
  connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::onStop);

  // Settings
  accEdit_ = new QLineEdit(settings); pwdEdit_ = new QLineEdit(settings); pwdEdit_->setEchoMode(QLineEdit::Password);
  langCombo_ = new QComboBox(settings); langCombo_->addItems({"English", QString::fromUtf8("繁體中文")});
  darkCheck_ = new QCheckBox(tr("Dark mode"), settings);
  closeTrayCheck_ = new QCheckBox(tr("Close X to tray"), settings);
  startMinCheck_ = new QCheckBox(tr("Start minimized to tray"), settings);
  asNone_ = new QRadioButton(tr("Autostart: None"), settings);
  asReg_ = new QRadioButton(tr("Autostart: Registry (Run)"), settings);
  asSvc_ = new QRadioButton(tr("Autostart: Windows Service"), settings);

  auto setLayout = new QVBoxLayout;
  auto row1 = new QHBoxLayout; row1->addWidget(new QLabel(tr("Account:"))); row1->addWidget(accEdit_);
  auto row2 = new QHBoxLayout; row2->addWidget(new QLabel(tr("Password:"))); row2->addWidget(pwdEdit_);
  auto row3 = new QHBoxLayout; row3->addWidget(new QLabel(tr("Language:"))); row3->addWidget(langCombo_); row3->addWidget(darkCheck_); row3->addStretch();
  auto row4 = new QHBoxLayout; row4->addWidget(closeTrayCheck_); row4->addWidget(startMinCheck_); row4->addStretch();
  auto row5 = new QHBoxLayout; row5->addWidget(asNone_); row5->addWidget(asReg_); row5->addWidget(asSvc_); row5->addStretch();
  setLayout->addLayout(row1); setLayout->addLayout(row2); setLayout->addLayout(row3); setLayout->addLayout(row4); setLayout->addLayout(row5); setLayout->addStretch();
  settings->setLayout(setLayout);

  connect(darkCheck_, &QCheckBox::toggled, this, &MainWindow::onDarkToggled);
  connect(langCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onLangChanged);
  connect(asNone_, &QRadioButton::toggled, this, &MainWindow::onAutostartChanged);
  connect(asReg_, &QRadioButton::toggled, this, &MainWindow::onAutostartChanged);
  connect(asSvc_, &QRadioButton::toggled, this, &MainWindow::onAutostartChanged);
  connect(accEdit_, &QLineEdit::textChanged, this, &MainWindow::onCredChanged);
  connect(pwdEdit_, &QLineEdit::textChanged, this, &MainWindow::onCredChanged);

  // Logs
  logEdit_ = new QPlainTextEdit(logs); logEdit_->setReadOnly(true);
  auto logLayout = new QVBoxLayout; logLayout->addWidget(logEdit_); logs->setLayout(logLayout);

  tabs_->addTab(dash, tr("Dashboard"));
  tabs_->addTab(settings, tr("Settings"));
  tabs_->addTab(logs, tr("Logs"));
  setCentralWidget(tabs_);

  statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupTray() {
  tray_ = new QSystemTrayIcon(windowIcon(), this);
  QMenu* menu = new QMenu(this);
  QAction* showAct = menu->addAction(tr("Show"));
  QAction* startAct = menu->addAction(tr("Start"));
  QAction* stopAct = menu->addAction(tr("Stop"));
  QAction* darkAct = menu->addAction(tr("Dark mode")); darkAct->setCheckable(true);
  menu->addSeparator();
  QAction* exitAct = menu->addAction(tr("Exit"));
  tray_->setContextMenu(menu);
  tray_->setToolTip(windowTitle());
  tray_->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
  tray_->show();
  connect(showAct, &QAction::triggered, this, [this]{ showNormal(); raise(); activateWindow(); });
  connect(startAct, &QAction::triggered, this, &MainWindow::onStart);
  connect(stopAct, &QAction::triggered, this, &MainWindow::onStop);
  connect(darkAct, &QAction::toggled, darkCheck_, &QCheckBox::setChecked);
  connect(exitAct, &QAction::triggered, this, [this]{ cfg_.askOnClose=false; close(); });
}

void MainWindow::applyTheme() {
  if (cfg_.darkTheme) {
    qApp->setStyle("Fusion");
    QPalette p; p.setColor(QPalette::Window, QColor(32,32,32)); p.setColor(QPalette::WindowText, QColor(220,220,220));
    p.setColor(QPalette::Base, QColor(24,24,24)); p.setColor(QPalette::AlternateBase, QColor(32,32,32)); p.setColor(QPalette::Text, QColor(220,220,220));
    p.setColor(QPalette::Button, QColor(48,48,48)); p.setColor(QPalette::ButtonText, QColor(220,220,220)); p.setColor(QPalette::Highlight, QColor(64,128,255));
    qApp->setPalette(p);
  } else {
    qApp->setPalette(QPalette());
  }
}

void MainWindow::loadConfig() {
  cfg_ = Config::load();
  accEdit_->setText(cfg_.account);
  pwdEdit_->setText(cfg_.password);
  darkCheck_->setChecked(cfg_.darkTheme);
  closeTrayCheck_->setChecked(cfg_.closeToTray);
  startMinCheck_->setChecked(cfg_.startMinimized);
  langCombo_->setCurrentIndex(cfg_.language=="zh-TW"?1:0);
  asNone_->setChecked(cfg_.autostart == AutostartMode::None);
  asReg_->setChecked(cfg_.autostart == AutostartMode::Registry);
  asSvc_->setChecked(cfg_.autostart == AutostartMode::Service);
  if (cfg_.startMinimized) hide();
}

void MainWindow::saveConfig() { Config::save(cfg_); }

void MainWindow::onStart() {
  if (accEdit_->text().isEmpty() || pwdEdit_->text().isEmpty()) {
    QMessageBox::warning(this, windowTitle(), tr("Please enter account and password")); return;
  }
  cfg_.account = accEdit_->text(); cfg_.password = pwdEdit_->text(); saveConfig();
  mon_.setCredentials(cfg_.account, cfg_.password); mon_.start();
  startBtn_->setEnabled(false); stopBtn_->setEnabled(true); statusLabel_->setText(tr("Status: Monitoring"));
}

void MainWindow::onStop() {
  mon_.stop(); startBtn_->setEnabled(true); stopBtn_->setEnabled(false); statusLabel_->setText(tr("Status: Stopped"));
}

void MainWindow::onDarkToggled(bool on) { cfg_.darkTheme = on; applyTheme(); saveConfig(); }
void MainWindow::onLangChanged(int idx) { cfg_.language = (idx==1?"zh-TW":"en"); saveConfig(); }

void MainWindow::onAutostartChanged() {
  if (asReg_->isChecked()) { cfg_.autostart = AutostartMode::Registry; Config::setRegistryRun(true); }
  else if (asSvc_->isChecked()) { cfg_.autostart = AutostartMode::Service; /* TODO: service */ }
  else { cfg_.autostart = AutostartMode::None; Config::clearAllAutostart(); }
  saveConfig();
}

void MainWindow::onCredChanged(const QString&) { cfg_.account = accEdit_->text(); cfg_.password = pwdEdit_->text(); saveConfig(); }

void MainWindow::closeEvent(QCloseEvent* e) {
  if (cfg_.askOnClose) {
    QMessageBox box(QMessageBox::Question, windowTitle(), tr("Choose Exit to quit, or Hide to tray to keep running."), QMessageBox::NoButton, this);
    QCheckBox* remember = new QCheckBox(tr("Always use this choice")); box.setCheckBox(remember);
    auto exitBtn = box.addButton(tr("Exit"), QMessageBox::AcceptRole);
    auto hideBtn = box.addButton(tr("Hide to tray"), QMessageBox::DestructiveRole);
    auto cancelBtn = box.addButton(tr("Cancel"), QMessageBox::RejectRole);
    box.exec();
    if (box.clickedButton() == cancelBtn) { e->ignore(); return; }
    if (remember->isChecked()) { cfg_.askOnClose = false; cfg_.closeToTray = (box.clickedButton() == hideBtn); saveConfig(); }
    if (box.clickedButton() == hideBtn) { hide(); e->ignore(); return; }
    // else Exit
    e->accept(); return;
  }
  if (cfg_.closeToTray) { hide(); e->ignore(); return; }
  e->accept();
}

void MainWindow::updateStatus() {
  statusLabel_->setText(mon_.running() ? tr("Status: Monitoring") : tr("Status: Stopped"));
}


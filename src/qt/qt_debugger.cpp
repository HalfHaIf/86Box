/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Program settings UI module.
 *
 *
 *
 * Authors: Cacodemon345
 *
 *          Copyright 2021-2022 Cacodemon345
 */
#include <QDebug>

#include "qt_debugger.hpp"
#include "ui_qt_debugger.h"
#include "qt_mainwindow.hpp"
#include "ui_qt_mainwindow.h"
#include "qt_machinestatus.hpp"

#include <QMap>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>

extern "C" {
#include <86box/86box.h>
#include <86box/version.h>
#include <86box/config.h>
#include <86box/plat.h>
#include <86box/mem.h>
#include <86box/rom.h>
}

extern MainWindow            *main_window;

Debugger::CustomTranslator *Debugger::translator   = nullptr;
QTranslator                    *Debugger::qtTranslator = nullptr;

Debugger::Debugger(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Debugger)
{
	setWindowTitle("Debugger");
	setModal(false);
    ui->setupUi(this);
}

void
Debugger::accept()
{
    QDialog::accept();
}

Debugger::~Debugger()
{
    delete ui;
}
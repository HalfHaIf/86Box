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

#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QMap>
#include <QPainter>
#include <QString>
#include <QTableWidget>
#include <QTimer>

extern "C" {
#include "../cpu/cpu.h"

#include <86box/86box.h>
#include <86box/version.h>
#include <86box/config.h>
#include <86box/plat.h>
#include <86box/mem.h>
#include <86box/rom.h>
}

extern MainWindow            *main_window;

const std::string cpunames[] = {
	"Intel 8088",
	"Intel 8086",
	"Kyiv Research Institute of Microdevices K1810VM86",
	"NEC V20",
	"NEC V30",
	"Intel 80188",
	"Intel 80186",
	"Intel 80286",
	"Intel i386SX",
	"IBM 386SLC",
	"IBM 486SLC",
	"Intel i386DX",
	"IBM 486BL",
	"Intel RapidCAD",
	"Intel 486SLC",
	"Intel 486DLC",
	"Intel i486SX",
	"AMD Am486SX",
	"Cyrix Cx486S",
	"Intel i486DX",
	"AMD Am486DX",
	"AMD Am486DXL",
	"Cyrix Cx486DX",
	"ST STPC-DX",
	"Intel i486SX-S",
	"Intel i486DX-S",
	"AMD Am486DX (Enhanced)",
	"Cyrix Cx5x86",
	"Intel Pentium OverDrive",
	"IDT WinChip",
	"IDT WinChip 2",
	"Intel Pentium",
	"Intel Pentium MMX",
	"Cyrix Cx6x86",
	"Cyrix Cx6x86MX",
	"Cyrix Cx6x86L",
	"Cyrix CxGX1",
	"AMD K5 (Model 0)",
	"AMD K5 (Model 1/2/3)",
	"AMD K6 (Model 6)",
	"AMD K6-2",
	"AMD K6-2C",
	"AMD K6-3",
	"AMD K6-2+",
	"AMD K6-III+",
	"Cyrix III",
	"Intel Pentium Pro",
	"Intel Pentium II",
	"Intel Pentium II OverDrive"
};

const std::string fpunames[] = {
	"None",
	"Intel 8087",
	"Intel 80187",
	"Intel 287",
	"Intel 287XL",
	"Intel 387",
	"Intel 487SX",
	"Internal FPU"
};

Debugger::Debugger(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Debugger)
{
	setWindowTitle("Debugger");
	setModal(false);
    ui->setupUi(this);
	
	qlist_buf = findChildren<QLabel*>();
	
	// Copy the contents of qlist_buf (QList) to cpu_labels (QMap) so we can easily address it later 
	for (QLabel* label : qlist_buf) {
		if (!label->objectName().isEmpty()) {
			cpu_labels[label->objectName()] = label;
		}	
	}
	
	initRegsTable();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timer->start(16); // TODO: Implement option to be able to change redraw rate with dropdown box
}

void Debugger::initRegsTable() {
	
	//Get regsTable object
	regstable = findChild<QTableWidget*>("regsTable");
	
	regstable->setRowCount(4);
	regstable->setColumnCount(2);
	
	regstable->verticalHeader()->setVisible(false);
	regstable->horizontalHeader()->setVisible(false);
	
	//Init regs_map
	regs_map = {
		{"EAX", (void*)&EAX},
		{"EBX", (void*)&EBX},
		{"ECX", (void*)&ECX},
		{"EDX", (void*)&EDX}
	};
	return;
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

// Format Hz and bytes
QString Debugger::formatFrequencyString(uint32_t rspeed) {
    const QStringList units = {"Hz", "kHz", "MHz", "GHz", "THz"};
    int unitIndex = 0;
    double value = rspeed;

    while (value >= 1000.0 && unitIndex < units.size() - 1) {
        value /= 1000.0;
        unitIndex++;
    }

    return QString("%1 %2").arg(value, 0, 'f', 2).arg(units[unitIndex]);
}

QString Debugger::formatByteString(uint32_t mem_size) {
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double value = mem_size * 1000.0;

    while (value >= 1000.0 && unitIndex < units.size() - 1) {
        value /= 1000.0;
        unitIndex++;
    }

    return QString("%1 %2").arg(value, 0, 'f', 2).arg(units[unitIndex]);
}

QString Debugger::formatHexString(QString str) {
	QString qstr_buf = str;
	qstr_buf = qstr_buf.toUpper();
	qstr_buf.prepend("0x");	
	return qstr_buf;
}

void Debugger::drawCPUInfo(QPaintEvent *event) {
	// Load buffer with initial string
	QString qstr_buf = QString::fromStdString(cpunames[cpu_s->cpu_type - 1]);
	
	// CPUType
	qstr_buf.prepend("CPU Type: ");
	qstr_buf.append(" @ ");
	qstr_buf.append(formatFrequencyString(cpu_s->rspeed));
	
	cpu_labels["CPUType"]->setText(qstr_buf);
	
	//FPUType
 	qstr_buf = QString::fromStdString(fpunames[fpu_type]);
	qstr_buf.prepend("FPU Type: ");
	cpu_labels["FPUType"]->setText(qstr_buf);
	
	//dynaRec
	qstr_buf = QString::fromStdString(cpu_use_dynarec ? "On" : "Off");
	qstr_buf.prepend("Dynamic recompiler: ");
	
	cpu_labels["dynaRec"]->setText(qstr_buf);
	
	//softFloatFPU
	qstr_buf = QString::fromStdString(fpu_softfloat ? "On" : "Off");
	qstr_buf.prepend("Softfloat FPU: ");
	
	cpu_labels["softFloatFPU"]->setText(qstr_buf);
	
	//PITMode
	qstr_buf = QString::fromStdString(pit_mode ? "Slow" : "Fast");
	qstr_buf.prepend("PIT mode: ");
	
	cpu_labels["PITMode"]->setText(qstr_buf);
	
	//waitStates
	if (!cpu_waitstates)
		qstr_buf = "Default";	
	else	
		qstr_buf = QString::number(cpu_waitstates - 1);
	qstr_buf.prepend("Wait states: ");
	
	cpu_labels["waitStates"]->setText(qstr_buf);
	
	//memoryInfo
	qstr_buf = formatByteString(mem_size);
	qstr_buf.prepend("Memory: ");
	qstr_buf.append(" of RAM (");
	qstr_buf.append(QString::number(mem_size * 1000.0));
	qstr_buf.append(" bytes )"); 
	
	cpu_labels["memoryInfo"]->setText(qstr_buf);
	return;
}

void Debugger::updateRegsTable(QString reg_name, int reg_width) {
	QString qstr_buf;
	uint8_t originalnumber;
	uint16_t number;
	switch (reg_width) {
		case 8:
		originalnumber = *(uint8_t*)regs_map[reg_name];
		number = (originalnumber & 0xFF);
		qstr_buf = QString::number(number, 16);
		break;
		case 16:
		qstr_buf = QString::number(*(uint16_t*)regs_map[reg_name], 16);
		break;		
		case 32:
		qstr_buf = QString::number(*(uint32_t*)regs_map[reg_name], 16);
		break;
	}
	regstable->setItem(currentrow, 0, new QTableWidgetItem(reg_name));
	regstable->setItem(currentrow, 1, new QTableWidgetItem(formatHexString(qstr_buf)));
	currentrow++;
}

void Debugger::drawRegs(QPaintEvent *event) {
	currentrow = 0;
	regstable->setRowCount(4);
	updateRegsTable("EAX", 32);
	updateRegsTable("EBX", 32);
	updateRegsTable("ECX", 32);
	updateRegsTable("EDX", 32);

	regstable->resizeRowsToContents();
	regstable->resizeColumnsToContents();
	return;
}

void Debugger::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	drawCPUInfo(event);
	drawRegs(event);
	
	return;
}
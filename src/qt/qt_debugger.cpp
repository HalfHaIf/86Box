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
#include <QMessageBox>
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


void Debugger::on_pauseButton_clicked() {
	plat_pause(dopause ^ 1);
	QIcon pause_icon = dopause ? QIcon(":/menuicons/qt/icons/run.ico") : QIcon(":/menuicons/qt/icons/pause.ico");
	ui->pauseButton->setIcon(pause_icon);
}

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
	
	connect(ui->pauseButton, &QPushButton::clicked, this, &on_pauseButton_clicked);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timer->start(16); // TODO: Implement option to be able to change redraw rate with dropdown box
}

void Debugger::initRegsTable() {
	
	//Get regsTable object
	regstable = findChild<QTableWidget*>("regsTable");
	
	regstable->setRowCount(0);
	regstable->setColumnCount(2);
	
	regstable->verticalHeader()->setVisible(false);
	regstable->horizontalHeader()->setVisible(false);
	
	//Init regs_map
	regs_map = {
		// Segments
		{"CS", (void*)&CS},
		{"DS", (void*)&DS},
		{"ES", (void*)&ES},
		{"SS", (void*)&SS},	
		// i386+
		{"FS", (void*)&FS},		
		{"GS", (void*)&GS},	
		
		// 16-bit registers (8086/8088-286)
		{"AX", (void*)&AX},
		{"BX", (void*)&BX},
		{"CX", (void*)&CX},
		{"DX", (void*)&DX},
		{"SP", (void*)&SP},
		{"BP", (void*)&BP},
		{"SI", (void*)&SI},
		{"DI", (void*)&DI},		
		
		// 32-bit registers (i386+)
		{"EAX", (void*)&EAX},
		{"EBX", (void*)&EBX},
		{"ECX", (void*)&ECX},
		{"EDX", (void*)&EDX},
		{"ESP", (void*)&ESP},
		{"EBP", (void*)&EBP},
		{"ESI", (void*)&ESI},
		{"EDI", (void*)&EDI},
		
		// Misc.
		{"IP", (void*)&cpu_state.pc},
		{"FLAGS", (void*)&cpu_state.flags},
		{"EFLAGS", (void*)&cpu_state.eflags},
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
	
	if (!regs_map[reg_name]) {
		QMessageBox::critical(this, "Error", "An error occured while processing the CPU registers.");
		this->accept();
	}
		
	
	int rowcount = regstable->rowCount();
	regstable->insertRow(rowcount);
	
	
	
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
	regstable->setItem(rowcount, 0, new QTableWidgetItem(reg_name));
	regstable->setItem(rowcount, 1, new QTableWidgetItem(formatHexString(qstr_buf)));
	
	
}

void Debugger::drawRegs(QPaintEvent *event) {

	// Add general-purpose registers
	regstable->setRowCount(0);
	
	if (cpu_s->cpu_type < CPU_386SX) {
		updateRegsTable("AX", 16);
		updateRegsTable("BX", 16);
		updateRegsTable("CX", 16);
		updateRegsTable("DX", 16);
		updateRegsTable("SP", 16);
		updateRegsTable("BP", 16);
		updateRegsTable("SI", 16);
		updateRegsTable("DI", 16);		
	} else {
		updateRegsTable("EAX", 32);
		updateRegsTable("EBX", 32);
		updateRegsTable("ECX", 32);
		updateRegsTable("EDX", 32);
		updateRegsTable("ESP", 32);
		updateRegsTable("EBP", 32);
		updateRegsTable("ESI", 32);
		updateRegsTable("EDI", 32);		
	}
	
	
	// Add misc registers (IP, FLAGS)
	updateRegsTable("IP", 32);
	updateRegsTable("FLAGS", 16);
	
	if (cpu_s->cpu_type >= CPU_386SX) {
		updateRegsTable("EFLAGS", 16);
	}
	
	// Add segments
	updateRegsTable("CS", 16);
	updateRegsTable("DS", 16);
	updateRegsTable("ES", 16);
	
	if (cpu_s->cpu_type >= CPU_386SX) {
		updateRegsTable("FS", 16);
		updateRegsTable("GS", 16);
	}
	
	updateRegsTable("SS", 16);
		
	//regstable->setRowCount(rowcount);
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
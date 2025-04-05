#ifndef QT_DEBUGGER_HPP
#define QT_DEBUGGER_HPP

#include <QDialog>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QTableWidget>

namespace Ui {
class Debugger;
}

class Debugger : public QDialog {
    Q_OBJECT

public:
    explicit Debugger(QWidget *parent = nullptr);
	
	QString formatFrequencyString(uint32_t rspeed);
	QString formatByteString(uint32_t mem_size);
	QString formatHexString(QString str);
	
    ~Debugger();
protected slots:
	void accept() override;
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::Debugger *ui;
	QMap<QString, QLabel*> cpu_labels;
	QMap<QString, void*> regs_map;
	QList<QLabel*> qlist_buf;
	QTableWidget* regstable;
	
	bool paused;
	
	friend class MainWindow;
	 
	
	void drawCPUInfo(QPaintEvent *event);
	void drawRegs(QPaintEvent *event);
	void initRegsTable();
	void updateRegsTable(QString reg_name, int reg_width);
	
	void on_pauseButton_clicked();
	void on_stepInto_clicked();
	void on_stepOver_clicked();
};

#endif // QT_DEBUGGER_HPP

#ifndef QT_DEBUGGER_HPP
#define QT_DEBUGGER_HPP

#include <QDialog>
#include <QLabel>
#include <QMap>
#include <QTableWidget>

namespace Ui {
class Debugger;
}

class Debugger : public QDialog {
    Q_OBJECT

public:
    explicit Debugger(QWidget *parent = nullptr);
	
	QString formatFrequency(uint32_t rspeed);
	QString formatBytes(uint32_t mem_size);
	
    ~Debugger();
protected slots:
	void accept() override;
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::Debugger *ui;
	QMap<QString, QLabel*> cpu_labels;
	QList<QLabel*> qlist_buf;
	QTableWidget* regstable;
	
	friend class MainWindow;
	 
	
	void drawCPUInfo(QPaintEvent *event);
	void drawRegs(QPaintEvent *event);
	void initRegsTable();
};

#endif // QT_DEBUGGER_HPP

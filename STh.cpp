#include "STh.h"
#include "MainStarter.h"
#include <iostream>
#include <QTextStream>
#include <QString>

// Консольные версии методов для вывода в stdout/stderr
//BA TablelistView
void STh::doEmitionChangeBARow(int index, QString loginPass, QString percentage)
{
	QTextStream out(stdout);
	out << "[BA] Row " << index << ": " << loginPass << " (" << percentage << "%)" << Qt::endl;
}

void STh::doEmitionShowRedVersion()
{
	QTextStream out(stdout);
	out << "[VERSION] Red version notification" << Qt::endl;
}

void STh::doEmitionStartScanIP()
{
	QTextStream out(stdout);
	out << "[SCAN] Starting IP scan..." << Qt::endl;
}

void STh::doEmitionStartScanDNS()
{
	QTextStream out(stdout);
	out << "[SCAN] Starting DNS scan..." << Qt::endl;
}

void STh::doEmitionStartScanImport()
{
	QTextStream out(stdout);
	out << "[SCAN] Starting import scan..." << Qt::endl;
}

void STh::doEmitionAddIncData(QString ip, QString str)
{
	QTextStream out(stdout);
	out << "[INC] " << ip << ": " << str << Qt::endl;
}

void STh::doEmitionAddOutData(QString str)
{
	QTextStream out(stdout);
	out << "[OUT] " << str << Qt::endl;
}

void STh::doEmitionFoundData(QString str)
{
	QTextStream out(stdout);
	out << str << Qt::endl;
}

void STh::doEmitionRedFoundData(QString str)
{
	QTextStream err(stderr);
	err << "[ERROR] " << str << Qt::endl;
}

void STh::doEmitionGreenFoundData(QString str)
{
	QTextStream out(stdout);
	out << "[OK] " << str << Qt::endl;
}

void STh::doEmitionFoundDataCustom(QString str, QString color)
{
	QTextStream out(stdout);
	out << "[" << color << "] " << str << Qt::endl;
}

void STh::doEmitionYellowFoundData(QString str)
{
	QTextStream out(stdout);
	out << "[WARN] " << str << Qt::endl;
}

void STh::doEmitionDebugFoundData(QString str)
{
	QTextStream out(stdout);
	out << "[DEBUG] " << str << Qt::endl;
}

void STh::doEmitionKillSttThread()
{
	QTextStream out(stdout);
	out << "[SCAN] Stopping scan thread..." << Qt::endl;
}

void STh::doEmitionDataSaved(bool status)
{
	QTextStream out(stdout);
	if (status) {
		out << "[INFO] Data saved successfully" << Qt::endl;
	}
}

void STh::doEmitionUpdateArc(unsigned long gTargets)
{
	QTextStream out(stdout);
	out << "\r[PROGRESS] Targets remaining: " << gTargets;
	out.flush();
}

void STh::doEmitionBlockButton(bool value)
{
	// В консольной версии не нужно блокировать кнопки
	Q_UNUSED(value);
}

int STh::baModelSize()
{
	return 0; // В консольной версии не поддерживается
}

void STh::setMode(short mode) {
	gMode = mode;
}
void STh::setTarget(QString target) {
	this->target = target;
}
void STh::setPorts(QString ports) {
	this->ports = ports;
}
void STh::run() 
{
	MainStarter ms;
	ms.start(this->target.toLocal8Bit().data(),
		this->ports.toLocal8Bit().data());
}

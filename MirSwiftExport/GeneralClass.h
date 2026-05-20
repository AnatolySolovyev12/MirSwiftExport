#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QDebug>;
#include <QFile>
#include <qsqlerror>
#include <QSqlQuery>
#include <iostream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <qtimer>
#include <QString>
#include <QPair>

class GeneralClass  : public QObject
{
	Q_OBJECT

public:
	GeneralClass(QObject *parent = nullptr);
	~GeneralClass();

	bool connectToDb();
	void getAllDevice();
	QList<QPair<QString, QString>> getMiddleId(QList<QPair<QString, QString>>tempArr);

	void getAllValueForEnergy();

	QList<QPair<QString, QPair<QString, QString>>> getIdFromIdValues(QList<QPair<QString, QString>>tempArr);


private:

	QSqlDatabase mainConnection;
	QList<QPair<QString, QString>> idMiddleSerialFinal;
	QList<QPair<QString, QPair<QString, QString>>> finalArrIdAndValueFinal;
};


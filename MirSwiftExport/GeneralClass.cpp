#include "GeneralClass.h"

GeneralClass::GeneralClass(QObject* parent)
	: QObject(parent)
{
	if (connectToDb())
	{
		getAllDevice();
		getAllValueForEnergy();

		for (int counter = 0, counterValue = 0; counter < idMiddleSerialFinal.length(); ++counter, ++counterValue)
		{
			QString temp;
			QString dayValue = finalArrIdAndValueFinal[counterValue].second.second;

			if (dayValue.length() > 3)
				dayValue.insert(dayValue.length() - 3, ",");
			else
				dayValue.push_front("0,");

			temp += idMiddleSerialFinal[counter].second + " " + dayValue;
			++counterValue;

			QString nightValue = finalArrIdAndValueFinal[counterValue].second.second;

			if (nightValue.length() > 3)
				nightValue.insert(nightValue.length() - 3, ",");
			else
				nightValue.push_front("0,");

			temp += " " + nightValue;

			if (idMiddleSerialFinal[counter].second == "") continue;

			qDebug() << temp;
		}

		mainConnection.close();
	}
}



GeneralClass::~GeneralClass()
{
	if (mainConnection.isOpen())
		mainConnection.close();
}



bool GeneralClass::connectToDb()
{
	QFile tempForCheckDb;

	std::string dbString;

	do {
		std::cout << "Enter name of your dataBase in app directory: ";
		std::cin >> dbString;

		if (std::cin.fail() || dbString.length() < 0 || dbString.length() > 100) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Очищаем буфер
			std::cout << "Incorrect name. Try again.\n";
		}
	} while (std::cin.fail() || dbString.length() < 0 || dbString.length() > 100);

	tempForCheckDb.setFileName(QString::fromStdString(dbString).trimmed()); // рихтануть метод в плане указания базы

	if (!tempForCheckDb.exists())
	{
		qDebug() << "NOT found your dataBase";
		return false;
	}

	mainConnection = QSqlDatabase::addDatabase("QSQLITE", "mirSwiftDb");
	mainConnection.setDatabaseName(QString::fromStdString(dbString).trimmed());

	if (!mainConnection.open())
	{
		if (mainConnection.lastError().isValid())
		{
			qDebug() << "Error in connectToDb::connectToDb() when try to create/open nameDb. Error:\n" << mainConnection.lastError().text();
		}
		else
			qDebug() << "NOT OPEN nameDb";

		return false;
	}
	else
	{
		//qDebug() << "Db is open";

		return true;
	}
}



void GeneralClass::getAllDevice()
{
	QSqlQuery queryMain(mainConnection);
	QSqlQuery queryCheck(mainConnection);

	QString queryString = "select object_owner_id, property_value, time_stamp from properties where property_type_id = 987 and property_value != '0'";

	QList<QPair<QString, QString>>idSerialList;

	if (!queryMain.exec(queryString) || !queryMain.next())
	{
		if (queryMain.lastError().isValid())
		{
			qDebug() << "Error in getAllData() when try to first read. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
		}
		else
			qDebug() << "NOT doing first read";

		return;
	}
	else
	{
		// Делаем первичную проверку на предмет того что это та запись с серийником которая нам требуется

		//std::cout << "First serial in func: " << queryMain.value(0).toString().toStdString() << "   " << queryMain.value(1).toString().toStdString() << std::endl;

		QString queryCheckString = QString("select * from objects where object_id = %1").arg(queryMain.value(0).toString());

		if (!queryCheck.exec(queryCheckString) || !queryCheck.next())
		{

			if (queryCheck.lastError().isValid())
			{
				qDebug() << "Error in getAllData() when try to check first read. Query:\n" << queryCheck.lastQuery() << "\nError text:\n" << queryCheck.lastError().text();
			}
			else
				qDebug() << "NOT check first read in start";

			return;
		}
		else
		{
			//std::cout << "First check in func: " << queryCheck.value(0).toString().toStdString() << "   " << queryCheck.value(1).toString().toStdString() << "   " << queryCheck.value(2).toString().toStdString() << std::endl;

			if (queryCheck.value(2).toString() == "Параметры устройства")
			{
				idSerialList.push_back(qMakePair(queryMain.value(0).toString(), queryMain.value(1).toString()));
			}
		}

		// Делаем последующие проверки на предмет того что это те записи с серийником которые нам требуется

		while (queryMain.next())
		{
			//std::cout << "Next serial in func: " << queryMain.value(0).toString().toStdString() << "   " << queryMain.value(1).toString().toStdString() << std::endl;

			QString queryCheckString = QString("select * from objects where object_id = %1").arg(queryMain.value(0).toString());

			if (!queryCheck.exec(queryCheckString) || !queryCheck.next())
			{
				if (queryCheck.lastError().isValid())
				{
					qDebug() << "Error in getAllData() when try to check first read. Query:\n" << queryCheck.lastQuery() << "\nError text:\n" << queryCheck.lastError().text();
				}
				else
					qDebug() << "NOT check first read";

				return;
			}
			else
			{
				//std::cout << "Next check in func: " << queryCheck.value(0).toString().toStdString() << "   " << queryCheck.value(1).toString().toStdString() << "   " << queryCheck.value(2).toString().toStdString() << std::endl;

				if (queryCheck.value(2).toString() == "Параметры устройства")
				{
					idSerialList.push_back(qMakePair(queryMain.value(0).toString(), queryMain.value(1).toString()));
				}
			}
		}
	}

	// выводим чистый массив с кем работать
	/*qDebug() << "Full serial array...";

	for (auto& val : idSerialList)
	{
		qDebug() << val.first << "   " << val.second;
	}
	*/
	idMiddleSerialFinal = getMiddleId(idSerialList);
}



QList<QPair<QString, QString>> GeneralClass::getMiddleId(QList<QPair<QString, QString>>tempArr)
{
	QSqlQuery queryMain(mainConnection);
	QList<QPair<QString, QString>>idMiddleSerial;

	for (auto& val : tempArr)
	{
		QString queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(val.first);

		if (!queryMain.exec(queryString) || !queryMain.next())
		{
			if (queryMain.lastError().isValid())
			{
				qDebug() << "Error in getMiddleId() when try to get middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
			}
			else
				qDebug() << "NOT doing get middle id";

			break;
		}
		else
		{
			queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(queryMain.value(1).toString());

			if (!queryMain.exec(queryString) || !queryMain.next())
			{
				if (queryMain.lastError().isValid())
				{
					qDebug() << "Error in getMiddleId() when try to get next middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
				}
				else
					qDebug() << "NOT doing get next middle id";

				break;
			}
			else
			{
				idMiddleSerial.push_back(qMakePair(queryMain.value(1).toString(), val.second));
			}

		}
	}

	// выводим чистый массив с кем работать
	/*qDebug() << "Full id serial array...";

	for (auto& val : idMiddleSerial)
	{
		qDebug() << val.first << "   " << val.second;
	}*/

	return idMiddleSerial;
}



void GeneralClass::getAllValueForEnergy()
{
	QString queryString = "select object_id, object_type_id, object_name from objects where object_type_id = 500105 OR object_type_id = 500106";

	QSqlQuery queryMain(mainConnection);
	QSqlQuery querySecond(mainConnection);

	QList<QPair<QString, QString>>idAndEnergy;

	if (!queryMain.exec(queryString) || !queryMain.next())
	{
		if (queryMain.lastError().isValid())
		{
			qDebug() << "Error in getAllValueForEnergy() when try to get all value id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
		}
		else
			qDebug() << "NOT get all value";

		return;
	}
	else
	{
		QString queryString = QString("SELECT object_owner_id, property_value, time_stamp FROM properties where object_owner_id = %1").arg(queryMain.value(0).toString());

		if (!querySecond.exec(queryString) || !querySecond.next())
		{
			if (querySecond.lastError().isValid())
			{
				qDebug() << "Error in getAllValueForEnergy() when try to get first querySecond. Query:\n" << querySecond.lastQuery() << "\nError text:\n" << querySecond.lastError().text();
			}
			else
				qDebug() << "NOT get all value";

			return;
		}
		else
		{
			idAndEnergy.push_back(qMakePair(querySecond.value(0).toString(), querySecond.value(1).toString()));
		}

		while (queryMain.next())
		{
			QString queryString = QString("SELECT object_owner_id, property_value, time_stamp FROM properties where object_owner_id = %1").arg(queryMain.value(0).toString());

			if (!querySecond.exec(queryString) || !querySecond.next())
			{
				if (querySecond.lastError().isValid())
				{
					qDebug() << "Error in getAllValueForEnergy() when try to get first querySecond. Query:\n" << querySecond.lastQuery() << "\nError text:\n" << querySecond.lastError().text();
				}
				else
					qDebug() << "NOT get all value";

				return;
			}
			else
			{
				idAndEnergy.push_back(qMakePair(querySecond.value(0).toString(), querySecond.value(1).toString()));
			}

		}
	}


	// выводим чистый массив с кем работать
	/*qDebug() << "Full value And Id  array...";

	for (auto& val : idAndEnergy)
	{
		qDebug() << val.first << "   " << val.second;
	}
	*/


	finalArrIdAndValueFinal = getIdFromIdValues(idAndEnergy);
}



QList<QPair<QString, QPair<QString, QString>>> GeneralClass::getIdFromIdValues(QList<QPair<QString, QString>>tempArr)
{
	QSqlQuery queryMain(mainConnection);

	QList<QPair<QString, QPair<QString, QString>>>finalArrIdAndValue;


	for (auto& val : tempArr)
	{
		QString queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(val.first);

		if (!queryMain.exec(queryString) || !queryMain.next())
		{
			if (queryMain.lastError().isValid())
			{
				qDebug() << "Error in getMiddleId() when try to get middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
			}
			else
				qDebug() << "NOT doing get middle id";

			break;
		}
		else
		{
			queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(queryMain.value(1).toString());

			QString tempId = queryMain.value(2).toString();

			if (!queryMain.exec(queryString) || !queryMain.next())
			{
				if (queryMain.lastError().isValid())
				{
					qDebug() << "Error in getMiddleId() when try to get next middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
				}
				else
					qDebug() << "NOT doing get next middle id";

				break;
			}
			else
			{
				finalArrIdAndValue.push_back(qMakePair(queryMain.value(1).toString(), qMakePair(val.first, val.second)));
			}

		}
	}

	// выводим чистый массив с кем работать
	/*qDebug() << "Full idId array...";

	for (auto& val : finalArrIdAndValue)
	{
		qDebug() << val.first << "   " << val.second;
	}
	*/
	return finalArrIdAndValue;
}
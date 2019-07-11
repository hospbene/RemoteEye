#include "src/GoalkeeperAnalysis.h"

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QtWidgets/QApplication>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	GoalkeeperAnalysis w;
	w.show();
	return a.exec();
}

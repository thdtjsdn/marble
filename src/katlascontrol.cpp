#include "katlascontrol.h"

#include <QtGui/QLayout>
#include <QtGui/QStringListModel>

#include "katlastoolbox.h"

KAtlasControl::KAtlasControl(QWidget *parent)
   : QWidget(parent)
{
	setWindowTitle(tr("Marble - Desktop Globe"));
//	resize(640, 640);
	resize(680, 640);

	toolbox = new KAtlasToolBox(this);
	toolbox -> setFixedWidth(185);

	katlasview=new KAtlasView(this);
	katlasview->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

	katlasview->setMinimumZoom( toolbox->minimumZoom() );

	QVBoxLayout *vlayout = new QVBoxLayout(this);

	QHBoxLayout *hlayout = new QHBoxLayout();	

	hlayout->addWidget(toolbox);
	hlayout->addWidget(katlasview);
	vlayout->addLayout(hlayout);

	toolbox -> setLocations(katlasview->getPlaceMarkModel());

	connect(toolbox, SIGNAL(goHome()), katlasview, SLOT(goHome()));
	connect(toolbox, SIGNAL(zoomChanged(int)), katlasview, SLOT(zoomView(int)));
	connect(toolbox, SIGNAL(zoomIn()), katlasview, SLOT(zoomIn()));
	connect(toolbox, SIGNAL(zoomOut()), katlasview, SLOT(zoomOut()));

	connect(toolbox, SIGNAL(moveLeft()), katlasview, SLOT(moveLeft()));
	connect(toolbox, SIGNAL(moveRight()), katlasview, SLOT(moveRight()));
	connect(toolbox, SIGNAL(moveUp()), katlasview, SLOT(moveUp()));
	connect(toolbox, SIGNAL(moveDown()), katlasview, SLOT(moveDown()));

	connect(katlasview, SIGNAL(zoomChanged(int)), toolbox, SLOT(changeZoom(int)));
	connect(toolbox, SIGNAL(centerOn(const QModelIndex&)), katlasview, SLOT(centerOn(const QModelIndex&)));
	connect(toolbox, SIGNAL(selectMapTheme(const QString&)), katlasview, SLOT(setMapTheme(const QString&)));
}

void KAtlasControl::zoomIn()
{
	katlasview->zoomIn();
}

void KAtlasControl::zoomOut()
{
	katlasview->zoomOut();
}

void KAtlasControl::moveLeft()
{
	katlasview->moveLeft();
}

void KAtlasControl::moveRight()
{
	katlasview->moveRight();
}

void KAtlasControl::moveUp()
{
	katlasview->moveUp();
}

void KAtlasControl::moveDown()
{
	katlasview->moveDown();
}

#include "katlastoolbox.moc"
#include "katlascontrol.moc"
